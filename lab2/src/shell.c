#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include "../include/command.h"
#include "../include/builtin.h"

int debug = 1;

// ======================= requirement 2.3 =======================
/**
 * @brief 
 * Redirect command's stdin and stdout to the specified file descriptor
 * If you want to implement ( < , > ), use "in_file" and "out_file" included the cmd_node structure
 * If you want to implement ( | ), use "in" and "out" included the cmd_node structure.
 *
 * @param p cmd_node structure
 * 
 */
void redirection(struct cmd_node *p){

	if (p->in_file) {
        int fd = open(p->in_file, O_RDONLY);
        if (fd == -1) {
            perror(p->in_file);
            exit(EXIT_FAILURE);
        }
        dup2(fd, STDIN_FILENO);
		p->in = fd;
		if(debug) fprintf(stderr,"dup input file\n");
        close(fd);
    } else if (p->in != STDIN_FILENO) {
        if (dup2(p->in, STDIN_FILENO) == -1) { 
			perror("dup2 p_in"); 
			exit(EXIT_FAILURE); 
		}
        close(p->in);
		if(debug) fprintf(stderr,"dup read_end\n");
    }

    if (p->out_file) {
        int fd = open(p->out_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (fd == -1) {
            perror(p->out_file);
            exit(EXIT_FAILURE);
        }
		p->out = fd;
        dup2(fd, STDOUT_FILENO);
        close(fd);
		if(debug) fprintf(stderr,"dup output file\n");
    } else if(p->out != STDOUT_FILENO) {
		if (dup2(p->out, STDOUT_FILENO) == -1) { 
			perror("dup2 p_out"); 
			exit(EXIT_FAILURE); 
		}
        close(p->out);
		if(debug) fprintf(stderr, "dup write_end\n");
	}

	if(debug) {
		fprintf(stderr, "=========== curr I/O info ==========\n");
		fprintf(stderr, "[cmd]: %s\n", p->args[0]);
		if(p->in_file) {
			fprintf(stderr, "input-file(%s)\n", p->in_file);
		}
		if(p->out_file) {
			fprintf(stderr, "output-file(%s)\n", p->out_file);
		}
		fprintf(stderr, "%d -> write-end{pipe(3), file(5)}\n", p->out);
		fprintf(stderr, "%d -> read-end{pipe(4), file(5)}\n", p->in);
		fprintf(stderr, "====================================\n");
	}
}
// ===============================================================

// ======================= requirement 2.2 =======================
/**
 * @brief 
 * Execute external command
 * The external command is mainly divided into the following two steps:
 * 1. Call "fork()" to create child process
 * 2. Call "execvp()" to execute the corresponding executable file
 * @param p cmd_node structure
 * @return int 
 * Return execution status
 */
int spawn_proc(struct cmd_node *p)
{
	pid_t pid = fork();
	if(pid < 0) {
		perror("fork");
		exit(EXIT_FAILURE);
	}
	if(pid == 0) {
		/**
		 * execvp("cmd", {"cmd", "p1", "p2"})
		 * redirect in child process
		 * prevent fd issues in parent
		 * child has its own fd table to manage files
		 */
		redirection(p);

		int status = execvp(p->args[0], p->args); 
		if(status == -1) {
			// only execution failed will come here
			perror("execvp");
			exit(EXIT_FAILURE);
		}
	} else {
		// parent wait for child or not
		waitpid(pid, NULL, 0);
	}
	return 1;
}
// ===============================================================


// ======================= requirement 2.4 =======================
/**
 * @brief 
 * Use "pipe()" to create a communication bridge between processes
 * Call "spawn_proc()" in order according to the number of cmd_node
 * @param cmd Command structure  
 * @return int
 * Return execution status 
 */
int fork_cmd_node(struct cmd *cmd)
{
	struct cmd_node *p = cmd->head;

	int in = dup(STDIN_FILENO), out = dup(STDOUT_FILENO);

	while(p) {

		// ========== init i/o interface ==========
		/**
		 * pipefd[0]: read
		 * pipefd[1]: write
		 */
		int pipefd[2];
		pipe(pipefd);

		if(p->out == 0) p->out = STDOUT_FILENO;
		
		if(p->next) {
			// 1. p writes to pipe_write_end
			p->out = pipefd[1];
			// 2. p->next reads from pipe_read_end
			p->next->in = pipefd[0];

			if(p->next->out == 0) p->next->out = STDOUT_FILENO;
		}
		// ========================================


		int status = searchBuiltInCommand(p);
		if (status != -1) {
			// build-in commands
			/**
			 * help: only stdout
			 * pwd: only stdout
			 * cd: no I/O
			 * echo: only stdout
			 *       @todo: "echo -n hello > tmp.txt"
			 * record: only stdout
			 * exit: no I/O
			 */

			// only have to consider stdout to pipe
			redirection(p);
			status = execBuiltInCommand(status, p);
		}
		else{
			// external command
			status = spawn_proc(p);
		}

		if(debug) {
			fprintf(stderr, "[%s]: p_in(%d), p_out(%d)\n", p->args[0] ,p->in, p->out);
			if(p->in_file) printf("\t(input_file) = %s\n", p->in_file);
			if(p->out_file) printf("\t(output_file) = %s\n", p->out_file);
			fprintf(stderr, "====================================\n");
		}

		// p has finished here, so we can close its read/write
		close(p->in);
		close(p->out);
		
		// recover stdin and stdout
		dup2(in, STDIN_FILENO);
		dup2(out, STDOUT_FILENO);

		p = p->next;
	}
	
	close(in);
	close(out);
	
	return 1;
}
// ===============================================================


void shell()
{
	while (1) {
		printf(">>> $ ");
		char *buffer = read_line();
		if (buffer == NULL)
			continue;

		struct cmd *cmd = split_line(buffer);
		// cmd contains a list of commands
		
		int status = -1;
		struct cmd_node *temp = cmd->head;
		
		// only a single command
		/**
		 * (1) cmd_x args > file
		 * (2) cmd_x args < file
		 */
		if(temp->next == NULL){
			status = searchBuiltInCommand(temp);
			if (status != -1){
				int in = dup(STDIN_FILENO), out = dup(STDOUT_FILENO);
				if( (in == -1) | (out == -1))
					perror("dup");

				// build-in commands
				/**
				 * help: only stdout
				 * pwd: only stdout
				 * cd: no I/O
				 * echo: only stdout
				 *       @todo: "echo -n hello > tmp.txt"
				 * record: only stdout
				 * exit: no I/O
				 */
				redirection(temp);
				status = execBuiltInCommand(status,temp);

				// recover shell stdin and stdout
				if (temp->in_file)  dup2(in, 0);
				if (temp->out_file) dup2(out, 1);
				close(in);
				close(out);
			}
			else{
				// external command
				status = spawn_proc(temp);
				
			}
		}
		// There are multiple commands ( | )
		else{
			status = fork_cmd_node(cmd);
		}
		// free space
		while (cmd->head) {
			
			struct cmd_node *temp = cmd->head;

			///////////////////////
			temp->in_file = NULL;
			temp->out_file = NULL;
			temp->in = STDIN_FILENO;
			temp->out = STDOUT_FILENO;
			///////////////////////

      		cmd->head = cmd->head->next;
			free(temp->args);
   	    	free(temp);
   		}
		free(cmd);
		free(buffer);
		
		if (status == 0)
			break;
	}
}
