#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>

int main() {
	int fd, res;
	fd = open("/dev/fooshm", O_RDONLY);
	if(fd < 0) {
		perror("Failed to open the device...");
		return errno;
	}
	
	char buf[100];
	res = read(fd, buf, 5);
	buf[5] = 0;
	printf("%s\n", buf);
	return 0;
}
