#include <linux/module.h>
#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/wait.h>
#define DEVICE_NAME "fooshm"
#define CLASS_NAME "foo"
#define BUFFER_SIZE 20

MODULE_LICENSE("GPL");

static int majorNumber;
static char buffer[BUFFER_SIZE];
static char* head;
static char* tail;
static bool bufferIsFull = 0;
static struct class*  thisClass  = NULL;
static struct device* thisDevice = NULL;

static ssize_t dev_write(struct file*, const char*, size_t, loff_t*);
static ssize_t dev_read(struct file*, char*, size_t, loff_t*);

static struct file_operations fops = {
	.write = dev_write,
	.read = dev_read,
};

static int __init init_module_here(void) {
	majorNumber = register_chrdev(0, DEVICE_NAME, &fops);
	if (majorNumber < 0){
		printk(KERN_ALERT "%s: failed to register a major number\n", DEVICE_NAME);
		return majorNumber;
	}
	printk(KERN_INFO "%s: registered correctly with major number %d\n", DEVICE_NAME, majorNumber);

	// Register the device class
	thisClass = class_create(THIS_MODULE, CLASS_NAME); 
	if (IS_ERR(thisClass)) {                // Check for error and clean up if there is
		unregister_chrdev(majorNumber, DEVICE_NAME);
		printk(KERN_ALERT "Failed to register device class\n");
		return PTR_ERR(thisClass);          // Correct way to return an error on a pointer
	}
	printk(KERN_INFO "%s: device class registered correctly\n", DEVICE_NAME);
 
	// Register the device driver
	thisDevice = device_create(thisClass, NULL, MKDEV(majorNumber, 0), NULL, DEVICE_NAME);
	if (IS_ERR(thisDevice)){               // Clean up if there is an error
		class_destroy(thisClass);           // Repeated code but the alternative is goto statements
		unregister_chrdev(majorNumber, DEVICE_NAME);
		printk(KERN_ALERT "Failed to create the device\n");
		return PTR_ERR(thisDevice);
	}
	printk(KERN_INFO "%s: device class created correctly\n", DEVICE_NAME);

	head = buffer;
	tail = buffer;
	memset(buffer, 'f', sizeof(buffer));
	return 0;
}

static void __exit cleanup_module_here(void) {
	device_destroy(thisClass, MKDEV(majorNumber, 0));     // remove the device
	class_unregister(thisClass);                          // unregister the device class
	class_destroy(thisClass);                             // remove the device class
	unregister_chrdev(majorNumber, DEVICE_NAME);             // unregister the major number
	printk(KERN_INFO "%s: Goodbye from the LKM!\n", DEVICE_NAME);	
}

static ssize_t dev_write(struct file* filep, const char* phrase, size_t len, loff_t* offset) { 
	if(bufferIsFull) return -1;

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
	
	return 0;	
}

static ssize_t dev_read(struct file* filep, char* phrase, size_t len, loff_t* offset) {
	if(head == tail && !bufferIsFull) {
		memset(phrase, 0, BUFFER_SIZE);
		copy_to_user(phrase, "...", 3);
		return 0;
	}
	bufferIsFull = 0;
	char temp[256];
	char* ptr = temp;
	len--;
	do {
		*ptr++ = *tail++;
		if(*tail == 0) tail = buffer;
	}while(len-- && head != tail);
	*ptr = 0;

	memset(phrase, 0, BUFFER_SIZE);
	copy_to_user(phrase, temp, strlen(temp));

	return 0;
}

module_init(init_module_here);
module_exit(cleanup_module_here);
