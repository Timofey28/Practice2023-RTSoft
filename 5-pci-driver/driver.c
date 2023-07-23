#include <linux/pci.h>

#define DEV_NAME "pci_drv"

MODULE_LICENSE("GPL");

static struct pci_device_id rtl8086_pci_tbl[] = {
	{PCI_DEVICE(0x8086, 0x100e)},
	{0,}
};

MODULE_DEVICE_TABLE(pci, rtl8086_pci_tbl);

static int major = 0;
static int port_addr = 0;

static int rtl8086_probe(struct pci_dev *dev, const struct pci_device_id *id);
static void rtl8086_remove(struct pci_dev *dev);

static struct pci_driver rtl8086_pci_driver = {
	.name = DEV_NAME,
	.id_table = rtl8086_pci_tbl,
	.probe = rtl8086_probe,
	.remove = rtl8086_remove,
};

static struct file_operations fops = {};

static int rtl8086_probe(struct pci_dev *dev, const struct pci_device_id *id) {
	port_addr = pci_resource_start(dev, 0);
	major = register_chrdev(0, "MyPCI", &fops);
	printk(KERN_INFO "Probe PCI: %d\n",major);
	//printk(KERN_INFO "Probe PCI\n");
	return 0;
}

static void rtl8086_remove(struct pci_dev *dev) {
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
