#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/fs.h>

#define DEV_NAME "echo"
#define CL_NAME "echo"
#define BUFFER_SIZE 1024

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Harmoning");

static int device_number = 0;
static char buffer[BUFFER_SIZE] = { 0 };
static short buffer_start = 0;
static struct class* echoClass = NULL;
static struct device* echoDevice = NULL;

static int open_handler(struct inode *, struct file *);
static int release_handler(struct inode *, struct file *);
static ssize_t write_handler(struct file *, const char __user *, size_t, loff_t *);
static ssize_t read_handler(struct file *, char __user *, size_t, loff_t *);

static struct file_operations echo_ops = {
  .read = read_handler,
  .open = open_handler,
  .release = release_handler,
  .write = write_handler,
};

static int __init echo_init(void) {
  printk("echo init started\n");
  device_number = register_chrdev(0, DEV_NAME, &echo_ops);
  if (device_number < 0) {
    printk("unable to register_chrdev\n");
    return device_number;
  }
  echoClass = class_create(THIS_MODULE, CL_NAME);
  if (IS_ERR(echoClass)) {
    unregister_chrdev(device_number, DEV_NAME);
RU
    printk("unable to create echo class\n");
    return PTR_ERR(echoClass);
  }
  echoDevice = device_create(echoClass, NULL, MKDEV(device_number, 0), NULL, DEV_NAME);
  if (IS_ERR(echoDevice)) {
    class_destroy(echoClass);
    unregister_chrdev(device_number, DEV_NAME);
    printk("unable to create echo device\n");
    return PTR_ERR(echoDevice);
  }
  return 0;
}

static void __exit echo_exit(void) {
  device_destroy(echoClass, MKDEV(device_number, 0));
  class_unregister(echoClass);
  class_destroy(echoClass);
  unregister_chrdev(device_number, DEV_NAME);
  printk("echo exit\n");
}

static int open_handler(struct inode *inodep, struct file *filep) {
  printk("device opened\n");
  return 0;  
}

static ssize_t read_handler(struct file *filep, char __user *buf, size_t len, loff_t *offset) {
  printk("read echo\n");
  if (*offset >= buffer_start) {
    ssize_t space = min(len, *offset - buffer_start);
    if (space <= 0)
      return 0;
    if (copy_to_user(buf, buffer + buffer_start, space))
      return -EFAULT;
    buffer_start += space;
    return space;
  } else {
    ssize_t space_to_end = BUFFER_SIZE - buffer_start;
    ssize_t llen = min(len, space_to_end);
    if (llen <= 0)
      return 0;
    if (copy_to_user(buf, buffer + buffer_start, llen))
      return -EFAULT;
    buffer_start += llen;
    if (buffer_start == BUFFER_SIZE)
      buffer_start = 0;
    len -= llen;
    if (len <= 0)
      return llen;
    ssize_t space_to_start = *offset;
    if (copy_to_user(buf + llen, buffer, min(len, space_to_start)))
      return -EFAULT;
    llen += min(len, space_to_start);
    buffer_start += min(len, space_to_start);
    return llen;
  }
} 

static ssize_t write_handler(struct file *filep, const char __user *buf, size_t len, loff_t* offset) {
  printk("write echo\n");
  if (*offset >= buffer_start) {
    ssize_t to_end_space = BUFFER_SIZE - *offset;
    ssize_t llen = min(to_end_space, len);
    if (llen <= 0)
      return 0;
    if (copy_from_user(buffer + *offset, buf, llen))
      return -EFAULT;
    *offset += llen;
    if (*offset == BUFFER_SIZE) {
      *offset = 0;
    }
    len -= llen;
    if (len <= 0)
     return llen;
   size_t to_start_space = buffer_start;
   if (copy_from_user(buffer, buf + llen, min(to_start_space, len)))
     return -EFAULT;
   llen += min(to_start_space, len);
   *offset += min(to_start_space, llen);
   return llen;
  } else {
    ssize_t space = buffer_start - *offset;
    ssize_t llen = min(space, len);
    if (llen <= 0)
      return 0;
    if (copy_from_user(buffer + *offset, buf, llen))
      return -EFAULT;
    *offset += llen;
    return llen;
  }
}

static int release_handler(struct inode *inodep, struct file *filep) {
  printk("dev release\n");
  return 0;
}

module_init(echo_init);
module_exit(echo_exit);

