#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>

#include "stepper.h"

static int fd = -1;

static struct stepper s;


int main(int argc, char **argv)
{
	if (argc != 2) {
        fprintf(stderr,"usage: stepper command\nYou can use 5 kinds of commands\nstart\nstop\nreverse\nup\ndown\n");
        exit(1);
    }
	
	fd=open("/dev/stepper",O_RDWR);
	if(fd<0) 
	{
		printf("Can't open\n");
		return -1;
	}
	else
	{
		printf("open OK %x\n", fd);
	}

	s.Arg=0;            //now this argument is not used
	switch(argv[1][0])
	{
		case 's':
			switch(argv[1][2])
			{
				case 'a':s.CmdID=0;printf("start\n");break;
				case 'o':s.CmdID=1;printf("stop\n");break;
				default:printf("command name error\n");exit(1);
			}
			break;
		case 'r':
			s.CmdID=2;printf("reverse\n");break;
		case 'u':
			s.CmdID=3;printf("up\n");break;
		case 'd':
			s.CmdID=4;printf("down\n");break;
		default:
			printf("command name error\n");exit(1);
	}

   
	ioctl(fd, IOCTL_SET_MSG, &s);

   	close(fd);
   	
  	return 0;
}


