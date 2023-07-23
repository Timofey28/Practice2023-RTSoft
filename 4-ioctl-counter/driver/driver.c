#include <linux/init.h>               
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/wait.h>
#include <linux/ioctl.h>
#include <linux/kthread.h>
#include <linux/delay.h>
#include <linux/device.h>       
#include "../mydriverio.h"
   
#define  DEVICE_NAME "counter_ioctl"    
#define  CLASS_NAME  "counter"
 
MODULE_LICENSE("GPL");                 
 
static int major;                  
static int numberOpens = 0;              
static struct class*  thisClass  = NULL; 
static struct device* thisDevice = NULL; 
static int counter = 0;
struct task_struct *ts;

static int dev_open(struct inode*, struct file*);
static int dev_release(struct inode*, struct file*);
static long dev_ioctl(struct file*, unsigned int cmd, unsigned long arg);

static struct file_operations fops = {
	.open = dev_open,
	.release = dev_release,
	.unlocked_ioctl = dev_ioctl, 
}; 

static int thread(void* data) {
	while(!kthread_should_stop()) {
		counter++;
		msleep(100);
	}
	return counter;
}

static int __init counter_init(void) {
	ts = kthread_run(thread, NULL, "my kthread");

	major = register_chrdev(0, DEVICE_NAME, &fops);
	if (major < 0) {
		printk(KERN_ALERT "Registering char device failed with %d\n", major);
		return major;
	}

	thisClass = class_create(THIS_MODULE, CLASS_NAME);
	if (IS_ERR(thisClass)) {                
		unregister_chrdev(major, DEVICE_NAME);
		printk(KERN_ALERT "Failed to register device class\n");
		return PTR_ERR(thisClass);          
	}

	thisDevice = device_create(thisClass, NULL, MKDEV(major, 0), NULL, DEVICE_NAME);
	if (IS_ERR(thisDevice)) {
		class_destroy(thisClass);
		unregister_chrdev(major, DEVICE_NAME);
		printk(KERN_ALERT "Failed to create the device\n");
		return PTR_ERR(thisDevice);
	}

	printk(KERN_INFO "Counter module has been loaded: %d\n", major); 
	return 0;
}

static void __exit counter_exit(void) {
	kthread_stop(ts);
	device_destroy(thisClass, MKDEV(major, 0));     
	class_unregister(thisClass);                         
	class_destroy(thisClass);                           
	unregister_chrdev(major, DEVICE_NAME);            
	printk(KERN_INFO "Counter module has been unloaded\n");
}  

static long dev_ioctl(struct file* filep, unsigned int cmd, unsigned long arg) {
	switch(cmd) {
		case IOCTL_SET_MSG: counter = 0;
							printk(KERN_INFO "Counter was set to zero\n");
							break;
		case IOCTL_GET_MSG: copy_to_user((int*) arg, &counter, sizeof(int));
							printk(KERN_INFO "Counter: %d\n", counter);
							break;
	}
	return counter;
}

static int dev_open(struct inode* inodep, struct file* filep) {
	numberOpens++;
	printk(KERN_INFO "Device has been opened %d time(s)\n", numberOpens);
	return 0;
}

static int dev_release(struct inode* inodep, struct file* filep) {
	printk(KERN_INFO "Device successfully closed\n");
	return 0;
}

module_init(counter_init);
module_exit(counter_exit);
