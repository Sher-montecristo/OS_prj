#include <linux/init.h>
#include <linux/slab.h>
#include <linux/sched.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/vmalloc.h>
#include <asm/uaccess.h>

#define BUFFER_SIZE 128
#define PROC_NAME "pid"


static int current_pid;


static ssize_t proc_read(struct file *file, char *buf, size_t count, loff_t *pos);
static ssize_t proc_write(struct file *file, const char __user *usr_buf, size_t count, loff_t *pos);

static struct file_operations proc_ops = {
    .owner = THIS_MODULE,
    .read = proc_read,
    .write = proc_write,
};

// This function is called when the module is loaded.
static int proc_init(void) {
    proc_create(PROC_NAME, 0666, NULL, &proc_ops);
    printk(KERN_INFO "/proc/%s created\n", PROC_NAME);
    return 0;
}

// This function is called when the module is removed.
static void proc_exit(void) {
    remove_proc_entry(PROC_NAME, NULL);
    printk( KERN_INFO "/proc/%s removed\n", PROC_NAME);
}

//This function is called each time the /proc/pid is read.
static ssize_t proc_read(struct file *file, char __user *usr_buf, size_t count, loff_t *pos) {
    int rv = 0;
    char buffer[BUFFER_SIZE];
    static int completed = 0;
    
    struct task_struct *tsk = NULL;
    if (completed) {
        completed = 0;
        return 0;
    }
    tsk = pid_task(find_vpid(current_pid), PIDTYPE_PID);
    
    if(tsk) {
        rv = snprintf(buffer, BUFFER_SIZE,
                      "command = [%s], pid = [%d], state = [%ld]\n",
                      tsk->comm, current_pid, tsk->state);
    } else {
        printk(KERN_INFO "Invalid PID %d!\n", current_pid);
        rv = snprintf(buffer, BUFFER_SIZE, "Invalid PID %d!\n", current_pid);
    }
    
    completed = 1;
    if(raw_copy_to_user(usr_buf, buffer, rv)){
    	rv = -1;
    }
    
    return rv;
}

/* This function is called each time we write to the /proc file system. */
static ssize_t proc_write(struct file *file, const char __user *usr_buf, size_t count, loff_t *pos) {
    char *k_mem;
    
    // allocate kernel memory
    k_mem = kmalloc(count, GFP_KERNEL);
    
    if(raw_copy_from_user(k_mem, usr_buf, count)){
    	printk(KERN_INFO "Error copying from user!\n");
    	return -1;
    }
    k_mem[count] = '\0';
    kstrtoint(k_mem, 10, &current_pid);
    printk(KERN_INFO "Set current PID to: %d\n", current_pid);
    kfree(k_mem);
    return count;
}

/* Macros for registering module entry and exit points. */
module_init(proc_init);
module_exit(proc_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Report the task information.");
MODULE_AUTHOR("BQS");
