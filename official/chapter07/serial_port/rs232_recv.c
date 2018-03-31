#include<sys/type.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<termios.h>
#include<stdio.h>

#define BAUDRATE B115200
#define MODEMDEVICE "/dev/ttyS0"

int main( )
{
	int fd,c=0,res;
	struct termios oldtio,newtio;
	char buf[256];
	printf("Start...\n");
	/*打开PC机的COM1通信端口*/
	fd=open(MODEMDEVICE,O_RDWR | O_NOCTTY);
	if(fd<0)
	{
		perror(MODEMDEVICE);
		exit(1);
	}
	printf("open...\n");
	/*将目前终端机的结构保存至oldtio结构*/
	tcgetattr(fd,&oldtio);
	/*清除newtio结构，重新设置通信协议*/
	bzero(&newtio,sizeof(newtio));
	/*通信协议设为8N1*/
	newtio.c_cflag =BAUDRATE |CS8|CLOCAL|CREAD;
	newtio.c_iflag=IGNPAR;
	newtio.c_oflag=0;
	/*设置为正规模式*/
	newtio.c_lflag=ICANON;
	/*清除所有队列在串口的输入*/
	tcflush(fd,TCIFLUSH);	/*新的termios的结构作为通信端口的参数*/
	tcsetattr(fd,TCSANOW,&newtio);
	printf("Reading...\n");
	while(1)
	{
		res=read(fd,buf,255);
		buf[res]=0;
		printf("res=%d  buf=%s\n",res,buf);
		if(buf[0]== '@')
			break;
	}
	printf("Close...\n");
	close(fd);
	/*恢复旧的通信端口参数*/
	tcsetattr(fd,TCSANOW,&oldtio);
	return 0;
}
