#include "receiver.h"

void receive(message_t* message_ptr, mailbox_t* mailbox_ptr){
    if(mailbox_ptr->flag == 1) {
        ssize_t bytes_read = mq_receive(
            mailbox_ptr->storage.mq_descriptor, 
            message_ptr->msgText,
            MAX_SIZE, 
            NULL
        );
        if(bytes_read == -1) {
            perror("mq_receive");
            exit(1);
        }
    } else {
        char *src = mailbox_ptr->storage.shm_addr;
        message_ptr->len = strnlen(src, MAX_SIZE-1);
        strncpy(message_ptr->msgText, src, message_ptr->len);
        message_ptr->msgText[message_ptr->len] = '\0';
    }
}

int main(int argc, char *argv[]) {
    // usage
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <1|2>\n", argv[0]);
        return 1;
    }

    // get the mechanism: 1 for mq, 2 for shm
    int ipc_comm = atoi(argv[1]);
    mailbox_t mailbox = {0};
    message_t message = {0};
    mailbox.flag = ipc_comm;

    // setup mailbox
    if(mailbox.flag == 1) {
        // message queue
        printf("%sMessage Passing%s\n", BLUE, RESET);
        mailbox.storage.mq_descriptor = mq_open(MQ_NAME, O_RDONLY);
        if (mailbox.storage.mq_descriptor == (mqd_t)-1) {
            perror("mq_open");
            exit(1);
        }
    } else {
        // shared memory
        printf("%sShared Memory%s\n", BLUE, RESET);
        int shm_fd = shm_open(SHM_NAME, O_RDWR, 0666);
        if(shm_fd == -1) {
            perror("shm_open");
            exit(1);
        }
        mailbox.storage.shm_addr = mmap(0, MAX_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, shm_fd, 0);
        if(mailbox.storage.shm_addr == MAP_FAILED) {
            perror("mmap");
            exit(1);
        }
        close(shm_fd);
    }

    sem_t* sender_sem = sem_open(SENDER_SEM_NAME, 0);
    sem_t* receiver_sem = sem_open(RECEIVER_SEM_NAME, 0);
    double total_time = 0;
    struct timespec start, end;

    while(1) {
        sem_wait(receiver_sem);

        clock_gettime(CLOCK_MONOTONIC, &start);
        receive(&message, &mailbox);
        clock_gettime(CLOCK_MONOTONIC, &end);

        sem_post(sender_sem);
        total_time += (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) * 1e-9;
        
        if(strcmp(message.msgText, EXIT_MSG) == 0) break;

        printf("%sReceiving message:%s %s", BLUE, RESET, message.msgText);
    }
    printf("\n");
    printf("%sSender exit!%s\n", RED, RESET);
    printf("Total time taken in receiving msg: %f s\n", total_time);

    if(mailbox.flag == 1) {
        // close message queue
        mq_close(mailbox.storage.mq_descriptor);
    } else {
        // close shared memory
        munmap(mailbox.storage.shm_addr, MAX_SIZE);
    }

    sem_close(sender_sem);
    sem_close(receiver_sem);

    return 0;
}
