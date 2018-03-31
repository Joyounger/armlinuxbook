#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>


void showbuf(char *buf);
int MAX_LEN=32;

int main()
{
	int fd;
	int i;
	char buf[255];
	
	for(i=0; i<MAX_LEN; i++){
		buf[i]=i;
	}

	fd=open("/dev/demo",O_RDWR);
	if(fd < 0){
		printf("####DEMO  device open fail####\n");
		return (-1);
	}
	printf("write %d bytes data to /dev/demo \n",MAX_LEN);
	showbuf(buf);
	write(fd,buf,MAX_LEN);

	printf("Read %d bytes data from /dev/demo \n",MAX_LEN);
	read(fd,buf,MAX_LEN);
	showbuf(buf);
	
	ioctl(fd,1,NULL);
	ioctl(fd,4,NULL);
	close(fd);
	return 0;

}


void showbuf(char *buf)
{
	int i,j=0;
	for(i=0;i<MAX_LEN;i++){
		if(i%4 ==0)
			printf("\n%4d: ",j++);
		printf("%4d ",buf[i]);
	}
	printf("\n*****************************************************\n");
}
