#ifndef MYDRIVERIO_H 
#define MYDRIVERIO_H

#include <linux/ioctl.h> 

#define MAGIC_NUM 0xF1 
#define IOC_GETMAC _IOR(MAGIC_NUM, 0, int*)

#endif
