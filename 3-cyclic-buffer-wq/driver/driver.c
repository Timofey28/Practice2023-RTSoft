#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/wait.h>

#define DEVICE_NAME "fooshm_wait"
#define CLASS_NAME "foo"
#define BUFFER_SIZE 20

MODULE_LICENSE("GPL");

static int majorNumber;
static char buffer[BUFFER_SIZE] = {0};
static char* head;
static char* tail;
static bool bufferIsFull = 0;
static struct class*  thisClass  = NULL;
static struct device* thisDevice = NULL;

static char flag = 'n';
static wait_queue_head_t wq_read;
static wait_queue_head_t wq_write;

static int dev_open(struct inode*, struct file*);
static int dev_release(struct inode*, struct file*);
static ssize_t dev_read(struct file*, char*, size_t, loff_t*);
static ssize_t dev_write(struct file*, const char*, size_t, loff_t*);

static struct file_operations fops = {
	.open = dev_open,
	.read = dev_read,
	.write = dev_write,
	.release = dev_release,
};

static int __init init_module_here(void) {
   	init_waitqueue_head(&wq_read);
	init_waitqueue_head(&wq_write);

	majorNumber = register_chrdev(0, DEVICE_NAME, &fops);
	if (majorNumber < 0){
		printk(KERN_ALERT "%s: failed to register a major number\n", DEVICE_NAME);
		return majorNumber;
	}

	thisClass = class_create(THIS_MODULE, CLASS_NAME); 
	if (IS_ERR(thisClass)) {
		unregister_chrdev(majorNumber, DEVICE_NAME);
		printk(KERN_ALERT "Failed to register device class\n");
		return PTR_ERR(thisClass);
	}
 
	thisDevice = device_create(thisClass, NULL, MKDEV(majorNumber, 0), NULL, DEVICE_NAME);
	if (IS_ERR(thisDevice)) {
		class_destroy(thisClass);
		unregister_chrdev(majorNumber, DEVICE_NAME);
		printk(KERN_ALERT "Failed to create the device\n");
		return PTR_ERR(thisDevice);
	}
	printk(KERN_INFO "%s: initialized successfully!\n", DEVICE_NAME);

	head = buffer;
	tail = buffer;
	memset(buffer, 'f', sizeof(buffer));
	return 0;
}

static void __exit cleanup_module_here(void) {
	device_destroy(thisClass, MKDEV(majorNumber, 0));
	class_unregister(thisClass);
	class_destroy(thisClass);
	unregister_chrdev(majorNumber, DEVICE_NAME);
	printk(KERN_INFO "%s: Goodbye from the LKM!\n", DEVICE_NAME);
}

static int dev_open(struct inode *inodep, struct file *filep) {
	printk(KERN_INFO "Opened\n");
	return 0;
}

static int dev_release(struct inode *inodep, struct file *filep) {
	printk(KERN_INFO "Released\n");
	return 0;
}

static ssize_t dev_write(struct file* filep, const char* phrase, size_t len, loff_t* offset) { 
	if(bufferIsFull && filep->f_flags & O_NONBLOCK)
		return -EAGAIN;
	if(wait_event_interruptible(wq_write, !bufferIsFull))
		return -ERESTARTSYS;

	char temp[256];
	memset(temp, 0, 256);
	copy_from_user(temp, phrase, len);
	char* ptr = temp;

	int counter = 0;
	while(*ptr != 0) {
		*head++ = *ptr++;
		counter++;
		if(*head == 0) head = buffer;
		if(head == tail) {
			bufferIsFull = 1;
       		if(*ptr != 0) return counter; // else no need to say how many bytes were written
		}
	}

	wake_up_interruptible(&wq_read);
	
	return 0;	
}

static ssize_t dev_read(struct file* filep, char* phrase, size_t len, loff_t* offset) {
	if(head == tail && !bufferIsFull && (filep->f_flags & O_NONBLOCK))
		return -EAGAIN;
	if(wait_event_interruptible(wq_read, head != tail || bufferIsFull))
		return -ERESTARTSYS;

	bufferIsFull = 0;
	char temp[256];
	char* ptr = temp;
	len--;
	do {
		*ptr++ = *tail++;
		if(*tail == 0) tail = buffer;
	}while(len-- && head != tail);
	*ptr = 0;

	wake_up_interruptible(&wq_write);

	memset(phrase, 0, BUFFER_SIZE);
	copy_to_user(phrase, temp, strlen(temp));

	return 0;
}

module_init(init_module_here);
module_exit(cleanup_module_here);
