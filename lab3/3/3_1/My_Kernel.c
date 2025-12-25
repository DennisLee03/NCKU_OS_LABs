#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/printk.h>
#include <linux/proc_fs.h>
#include <asm/current.h>

#define procfs_name "Mythread_info"
#define BUFSIZE  1024
char buf[BUFSIZE];

static ssize_t Mywrite(struct file *fileptr, const char __user *ubuf, size_t buffer_len, loff_t *offset){
    /* Do nothing */
	return 0;
}

/**
 * @brief when trigger a read of the procfile, this function is called.
 * 
 * @param fileptr    procfile
 * @param ubuf       string to user space
 * @param buffer_len how many bytes can be sended to user space, for instance: fgets(buffer, sizeof(buffer), fptr4)
 * @param offset     r/w head, offset > 0 means we finish this read, so do nothing
 *
 * @return           #bytes read
 */
static ssize_t Myread(struct file *fileptr, char __user *ubuf, size_t buffer_len, loff_t *offset){

    // when EOF, directly return, prevent infinite reads
    if (*offset > 0) return 0;

    // parse threads' info
    struct task_struct *thread;
    for_each_thread(current, thread) {
        if (current->pid == thread->pid) {
            continue;
        }
        *offset += sprintf(&buf[*offset], "PID: %d, TID: %d, Priority: %d, State: %d\n",
                          current->pid, thread->pid, thread->prio, thread->__state);
    }

    // send info to user space
    if (copy_to_user(ubuf, buf, buffer_len)) {
        pr_info("copy_to_user failed.\n");
        return -EFAULT;
    }

    return *offset > buffer_len ? *offset : buffer_len;
}

static struct proc_ops Myops = {
    .proc_read = Myread,
    .proc_write = Mywrite,
};

static int My_Kernel_Init(void){
    proc_create(procfs_name, 0644, NULL, &Myops);   
    pr_info("My kernel says Hi");
    return 0;
}

static void My_Kernel_Exit(void){
    pr_info("My kernel says GOODBYE");
}

module_init(My_Kernel_Init);
module_exit(My_Kernel_Exit);

MODULE_LICENSE("GPL");