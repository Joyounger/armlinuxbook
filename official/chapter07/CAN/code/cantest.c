/************************************************\
*	by threewater<threewater@up-tech.com>	*
*		2003.12.18			*
*						*
\***********************************************/

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
//#include <sys/types.h>
//#include <sys/ipc.h>
#include <sys/ioctl.h>
#include <pthread.h>

//#include "hardware.h"
#include "up-can.h"

#define CAN_DEV		"/dev/can/0"
static int can_fd = -1;

#define DEBUG
#ifdef DEBUG
#define DPRINTF(x...)	printf("Debug:"##x)
#else
#define DPRINTF(x...)
#endif

static void* canRev(void* t)
{
	CanData	data;
	int i;
	
	DPRINTF("can recieve thread begin.\n");
	for(;;){
		read(can_fd, &data, sizeof(CanData));
		for(i=0;i<data.dlc;i++)
			putchar(data.data[i]);
		fflush(stdout);
		
	}
	return NULL;
}

#define MAX_CANDATALEN		8
static void CanSendString(char *pstr)
{
	CanData data;
	int len=strlen(pstr);
	memset(&data,0,sizeof(CanData));
	data.id=0x123;
	data.dlc=8;
	for(;len>MAX_CANDATALEN;len-=MAX_CANDATALEN){
		memcpy(data.data, pstr, 8);
		//write(can_fd, pstr, MAX_CANDATALEN);
		write(can_fd, &data, sizeof(data));
		pstr+=8;
	}
	data.dlc=len;
	memcpy(data.data, pstr, len);
	//write(can_fd, pstr, len);
	write(can_fd, &data, sizeof(CanData));
}

int main(int argc, char** argv)
{
	int i;
	pthread_t th_can;
	static char str[256];
	static const char quitcmd[]="\\q!";
	void * retval;
	int id=0x123;
	char usrname[100]={0,};
 
       if((can_fd=open(CAN_DEV, O_RDWR))<0){
                printf("Error opening %s can device\n", CAN_DEV);
                return 1;
        }

	ioctl(can_fd, UPCAN_IOCTRL_PRINTRIGISTER, 1);
	ioctl(can_fd, UPCAN_IOCTRL_SETID, id);
	
#ifdef DEBUG
	ioctl(can_fd, UPCAN_IOCTRL_SETLPBK, 1);
#endif
	/* Create the threads */
	pthread_create(&th_can, NULL, canRev, 0);

	printf("\nPress \"%s\" to quit!\n", quitcmd);
	printf("\nPress Enter to send!\n");

	if(argc==2){	//Send user name
		sprintf(usrname, "%s: ", argv[1]);
	}
		
	for(;;){
		int len;
		scanf("%s", str);
		
		if(strcmp(quitcmd, str)==0){
			break;
		}
		
		if(argc==2)	//Send user name
			CanSendString(usrname);
		
		len=strlen(str);
		str[len]='\n';
		str[len+1]=0;
		CanSendString(str);
	}
	/* Wait until producer and consumer finish. */
	//pthread_join(th_com, &retval);

	printf("\n");
	close(can_fd);
	return 0;
}
