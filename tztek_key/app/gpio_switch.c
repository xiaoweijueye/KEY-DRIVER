#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <signal.h>
#include <string.h>

int fd;
int value;

void catch_sigio(int signu)
{
	printf("catch signo\n");
	read(fd, &value, sizeof(value));
	
	if(1 == value)
		printf("gpio switch 1 pressed!\n");
	if(2 == value)
		printf("gpio switch 2 pressed!\n");
}

int main(void)
{
	int flags;
	
	SIG_ERR == signal(SIGIO, catch_sigio);
	
	fd = open("/dev/gpio_switch_1", O_RDWR);
	if(-1 == fd){
		perror("open");
		return -1;
	} 

	fcntl(fd, F_SETOWN, getpid());
	flags = fcntl(fd, F_GETFL);
	fcntl(fd, F_SETFL, flags | O_ASYNC);

	while (1){
		NULL;
	}

	return 0;
}
