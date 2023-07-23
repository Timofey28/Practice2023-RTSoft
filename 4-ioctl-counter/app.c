#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include "mydriverio.h"
#include <stdbool.h>
 
int main()
{
	int fd, arg;
	fd = open("/dev/counter_ioctl", O_RDWR);
	if(fd < 0) {
		printf("Cannot open device file...\n");
		return 0;
	}

	char choice;
	bool anotherChar = 0; // чтобы фраза не выводилась 2 раза
	while(1) {
		if(!anotherChar) printf("Enter: 's' - to set counter to zero, 'g' - to get current counter value, 'e' - to exit\n");
		scanf("%c", &choice);
		switch(choice) {
			case 's': ioctl(fd, IOCTL_SET_MSG);
					  printf("Counter was set to zero\n");
					  anotherChar = 0;
					  break;
			case 'g': ioctl(fd, IOCTL_GET_MSG, &arg);
					  printf("Current value: %d\n", arg);
					  anotherChar = 0;
					  break;
			case 'e': close(fd);
					  return 0;
			default: anotherChar = 1;
		}
	}

	/*ioctl(fd, IOCTL_GET_MSG, &arg);
	printf("Reading Value from Driver: %d\n", arg);
	printf("Reasetting Value\n");
	ioctl(fd, IOCTL_SET_MSG);
	printf("Closing Driver\n");
	close(fd);*/

	return 0;
}
