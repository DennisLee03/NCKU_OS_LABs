#include <asm/current.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/printk.h>
#include <linux/proc_fs.h>
#include <linux/string.h>

#define procfs_name "Mythread_info"
#define BUFSIZE 1024
char buf[BUFSIZE];  // kernel buffer

/**
 * @brief            when trigger a write of the procfile, this function is called.
 * 
 * @param fileptr    procfile
 * @param ubuf       string from user space
 * @param buffer_len how many bytes can be sended to kernel, for instance: fwrite(data, sizeof(char), strlen(data), fd);
 * @param offset     unused here
 *
 * @return           #bytes read
 */
static ssize_t Mywrite(struct file *fileptr, const char __user *ubuf, size_t buffer_len, loff_t *offset) {
    int k_len = strlen(buf);

    if (copy_from_user(&buf[k_len], ubuf, buffer_len)) return -EFAULT;
    k_len += buffer_len;

    k_len += sprintf(&buf[k_len], "PID: %d, TID: %d, Time: %llu\n", current->tgid, current->pid, current->utime / 100 / 1000);

    return buffer_len;
}

/**
 * @brief            when trigger a read of the procfile, this function is called.
 * 
 * @param fileptr    procfile
 * @param ubuf       string to user space
 * @param buffer_len how many bytes can be sended to user space, for instance: fgets(buffer, sizeof(buffer), fptr4)
 * @param offset     r/w head, offset > 0 means we finish this read, so do nothing
 *
 * @return           #bytes read
 */
static ssize_t Myread(struct file *fileptr, char __user *ubuf, size_t ubuffer_len, loff_t *offset) {
    if(*offset > 0) return 0;

    if (copy_to_user(ubuf, buf, strlen(buf))) return -EFAULT;

    *offset = strlen(buf);

    return *offset;
}

static struct proc_ops Myops = {
    .proc_read = Myread,
    .proc_write = Mywrite,
};

static int My_Kernel_Init(void) {
    proc_create(procfs_name, 0644, NULL, &Myops);
    pr_info("My kernel says Hi");
    return 0;
}

static void My_Kernel_Exit(void) {
    pr_info("My kernel says GOODBYE");
}

module_init(My_Kernel_Init);
module_exit(My_Kernel_Exit);

MODULE_LICENSE("GPL");