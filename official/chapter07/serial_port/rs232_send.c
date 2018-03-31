#include <sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<termios.h>
#include<stdio.h>
	
#define BAUDRATE B115200
#define MODEMDEVICE  "/dev/ttyS0"
#define STOP '@'

int main( )
{
	int fd,c=0,res;
	struct termios oldtio,newtio;
	char ch;
	static char s1[20];
	printf("Start...\n ");
	/*打开arm平台的COM1通信端口*/
	fd=open(MODEMDEVICE,O_RDWR | O_NOCTTY);
	if(fd<0)
	{
		perror(MODEMDEVICE);
		exit(1);
	}
	printf(" Open...\n ");
	 /*将目前终端机的结构保存至oldtio结构*/
       cgetattr(fd,&oldtio);
/*清除newtio结构，重新设置通信协议*/
bzero(&newtio,sizeof(newtio));
/*通信协议设为8N1*/
newtio.c_cflag =BAUDRATE |CS8|CLOCAL|CREAD;
newtio.c_iflag=IGNPAR;
newtio.c_oflag=0;
/*设置为正规模式*/
newtio.c_lflag=ICANON;
/*清除所有队列在串口的输出*/
tcflush(fd,TCOFLUSH);
/*新的termios的结构作为通信端口的参数*/
tcsetattr(fd,TCSANOW,&newtio);
printf("Writing...\n ");
while(1)
{
While((ch=getchar()) !=STOP)
{
	s1[0]=ch;
	res=write(fd,s1,1);
}
s1[0]=ch;
s1[1]= '\n';
res=write(fd,s1,2);
break;
}
printf("Close...\n");
close(fd);
/*还原旧的通信端口参数*/
tcsetattr(fd,TCSANOW,&oldtio);
return 0;
}
