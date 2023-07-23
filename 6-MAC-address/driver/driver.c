#include <linux/pci.h>
#include "../mydriverio.h"

#define DEV_NAME "pci_drv"
#define VENDOR_ID 0x8086
#define DEVICE_ID 0x100e
#define MAC_SIZE 6

MODULE_LICENSE("GPL");

static struct pci_device_id pci_table[] = {
	{VENDOR_ID, DEVICE_ID, PCI_ANY_ID, PCI_ANY_ID, 0, 0, NULL},
	{0,}
};

MODULE_DEVICE_TABLE(pci, pci_table);

static int major;
static int mac_offset = 0;
static int mac[MAC_SIZE];
static u8 __iomem *devmem = NULL;

static int rtl8086_probe(struct pci_dev *dev, const struct pci_device_id *id);
static void rtl8086_remove(struct pci_dev *dev);

static struct pci_driver rtl8086_pci_driver = {
	.name = DEV_NAME,
	.id_table = pci_table,
	.probe = rtl8086_probe,
	.remove = rtl8086_remove,
};

static int dev_open(struct inode *inodep, struct file *filep) {
	printk(KERN_INFO "Opened\n");
	try_module_get(THIS_MODULE);
	return 0;
}

static int dev_release(struct inode *inodep, struct file *filep) {
	printk(KERN_INFO "Released\n");
	module_put(THIS_MODULE);
	return 0;
}

static long dev_ioctl(struct file* f, unsigned int cmd, unsigned long arg)
{
    int i = 0;
    switch(cmd) {
		case IOC_GETMAC:
            while(i < 6) { 
                mac[i] = (unsigned int)ioread8(&devmem[mac_offset+i]);
                pr_info("[%d] = %02x", i, (mac[mac_offset+i]));
                i++;
            }
            copy_to_user(arg, &mac, sizeof(mac));
            break;

		default:
			return -ENOTTY;
	}

	return 0;
}

static struct file_operations fops = {
	.open = dev_open,
	.release = dev_release,
	.unlocked_ioctl = dev_ioctl,
};

static int rtl8086_probe(struct pci_dev* dev, const struct pci_device_id* id) {
	major = register_chrdev(0, "MyPCI", &fops);
	unsigned long memio_start, memio_len;
    memio_start = pci_resource_start(dev, 0);
    memio_len = pci_resource_len(dev, 0);
    devmem = ioremap(memio_start, memio_len);

    if(!devmem) return -EIO;

	printk(KERN_INFO "Probe PCI: %d\n", major);
	return 0;
}

static void rtl8086_remove(struct pci_dev* dev) {
	if(devmem) iounmap(devmem);
	unregister_chrdev(major, "MyPCI");
	printk(KERN_INFO "Remove PCI");
}

static int rtl8086_init_module(void) {
	printk(KERN_INFO "initialization module completed!");
	return pci_register_driver(&rtl8086_pci_driver);
}

static void rtl8086_cleanup_module(void) {
	pci_unregister_driver(&rtl8086_pci_driver);
	printk(KERN_INFO "cleanup module completed");
}

module_init(rtl8086_init_module);
module_exit(rtl8086_cleanup_module);
