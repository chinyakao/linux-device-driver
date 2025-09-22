#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>

#define DEVICE_NAME "virt_char"
#define BUF_LEN 128

static int major;
static char msg[BUF_LEN];
static int len;

static ssize_t dev_read(struct file *file, char __user *buf, size_t count, loff_t *offset) {
    return simple_read_from_buffer(buf, count, offset, msg, len);
}

static ssize_t dev_write(struct file *file, const char __user *buf, size_t count, loff_t *offset) {
    len = simple_write_to_buffer(msg, BUF_LEN, offset, buf, count);
    return len;
}

static struct file_operations fops = {
    .owner = THIS_MODULE,
    .read = dev_read,
    .write = dev_write,
};

static int __init virt_char_init(void) {
    major = register_chrdev(0, DEVICE_NAME, &fops);
    printk(KERN_INFO "virt_char loaded: major %d\n", major);
    return 0;
}

static void __exit virt_char_exit(void) {
    unregister_chrdev(major, DEVICE_NAME);
    printk(KERN_INFO "virt_char unloaded\n");
}

MODULE_LICENSE("GPL");
module_init(virt_char_init);
module_exit(virt_char_exit);
