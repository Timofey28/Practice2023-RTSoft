#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>

int main() {
	int fd, res;
	fd = open("/dev/fooshm_wait", O_WRONLY);
	if(fd < 0) {
		perror("Failed to open the device...");
		return errno;
	}

	char buf[256];
	while(1) {
		printf("Write something: ");
		fgets(buf, sizeof(buf), stdin);
		buf[strcspn(buf, "\n")] = 0;
		if(!strcmp(buf, "exit")) return 0;

		res = write(fd, buf, strlen(buf));
		switch(res)
		{
			case -1: printf("Buffer is full\n"); break;
			case 0: break;
			default: printf("Recorded %d out of %d bytes.\n", res, strlen(buf));
		}
	}
	return 0;
}
