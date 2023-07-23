#include <sys/ioctl.h>
#include <stdio.h>
#include <fcntl.h>
#include "mydriverio.h"

int main()
{
	int fd = 0;
	fd = open("/dev/MyPCI", O_RDWR);
	if(fd < 0) {
		printf("failed to open: %d", fd);
		return fd;
	}
	int mac[6];
	if(ioctl(fd, IOC_GETMAC, mac)) {
		printf("failed to cmd");
		return -1;
	}

	int i = 0;
	while (i < 6) {
		printf("[%d] = %02x", i, (mac[i]));
		i++;
	}

	close(fd);
	return 0;
}
