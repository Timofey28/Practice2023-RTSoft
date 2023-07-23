#ifndef MYDRIVERIO_H 
#define MYDRIVERIO_H
#include <linux/ioctl.h> 
#define MAGIC_NUM 0xF1 
#define IOCTL_SET_MSG _IO(MAGIC_NUM, 0)
#define IOCTL_GET_MSG _IOR(MAGIC_NUM, 1, int*)
#define DEVICE_FILE_NAME "counter_ioctl" 
#define DEVICE_PATH "/dev/counter_ioctl" 
#endif
