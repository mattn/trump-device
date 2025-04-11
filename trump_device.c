#include <linux/init.h>
#include <linux/module.h>
#include <linux/uaccess.h>
#include <linux/random.h>
#include <linux/device.h>
#include <linux/fs.h>

#define DEVICE_NAME "trump"
#define CLASS_NAME  "trump_class"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Yasuhiro Matsumoto (a.k.a. mattn)");
MODULE_DESCRIPTION("/dev/trump returns a positive random integer");
MODULE_VERSION("0.1");

static int    majorNumber;
static struct class*  trumpClass  = NULL;
static struct device* trumpDevice = NULL;

static ssize_t
trump_read(struct file *filep, char *buffer, size_t len, loff_t *offset){
  unsigned int rand;
  int length;
  char msg[32];

  get_random_bytes(&rand, sizeof(rand));
  rand = (rand & 0x7FFFFFFF) + 1;

  length = snprintf(msg, sizeof(msg), "%u\n", rand);

  if (*offset >= length)
    return 0;

  if (copy_to_user(buffer, msg, length))
    return -EFAULT;

  *offset += length;
  return length;
}

static struct
file_operations fops = {
  .owner = THIS_MODULE,
  .read = trump_read,
};

static int
__init trump_init(void){
  majorNumber = register_chrdev(0, DEVICE_NAME, &fops);
  if (majorNumber < 0){
    printk(KERN_ALERT "Trump: failed to register a major number, make america great again\n");
    return majorNumber;
  }

  trumpClass = class_create(CLASS_NAME);
  if (IS_ERR(trumpClass)){
    unregister_chrdev(majorNumber, DEVICE_NAME);
    printk(KERN_ALERT "Failed to register device class, make america great again\n");
    return PTR_ERR(trumpClass);
  }

  trumpDevice = device_create(trumpClass, NULL, MKDEV(majorNumber, 0), NULL, DEVICE_NAME);
  if (IS_ERR(trumpDevice)){
    class_destroy(trumpClass);
    unregister_chrdev(majorNumber, DEVICE_NAME);
    printk(KERN_ALERT "Failed to create the device, make america great again\n");
    return PTR_ERR(trumpDevice);
  }

  printk(KERN_INFO "Trump device created correctly, make america great again\n");
  return 0;
}

static void
__exit trump_exit(void){
  device_destroy(trumpClass, MKDEV(majorNumber, 0));
  class_unregister(trumpClass);
  class_destroy(trumpClass);
  unregister_chrdev(majorNumber, DEVICE_NAME);
  printk(KERN_INFO "Trump device removed, make america great again\n");
}

module_init(trump_init);
module_exit(trump_exit);
