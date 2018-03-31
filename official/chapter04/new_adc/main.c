/************************************************\
*	by threewater<threewater@up-tech.com>	*
*		2004.06.18			*
*						*
\***********************************************/

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/ioctl.h>
#include <pthread.h>
#include <fcntl.h>
#include "s3c2410-adc.h"

#define ADC_DEV		"/dev/adc"
static int adc_fd = -1;

static int init_ADdevice(void)
{
	if((adc_fd=open(ADC_DEV, O_RDWR))<0){
		printf("Error opening %s adc device\n", ADC_DEV);
		return -1;
	}
	return 1;
}

static int GetADresult(int channel)
{
	int PRESCALE=0XFF;
	int data=ADC_WRITE(channel, PRESCALE);
	write(adc_fd, &data, sizeof(data));
	read(adc_fd, &data, sizeof(data));
	return data;
}
static int stop=0;

static void* comMonitor(void* data)
{
	getchar();
	stop=1;
	return NULL;
}

int main(void)
{
	int i;
	float d;
	pthread_t th_com;
	void * retval;

	//set s3c44b0 AD register and start AD
	if(init_ADdevice()<0)
		return -1;
                                                                                
	/* Create the threads */
	pthread_create(&th_com, NULL, comMonitor, 0);

	printf("\nPress Enter key exit!\n");

	while( stop==0 ){
		for(i=0; i<=2; i++){//采样0~2路A/D值
			d=((float)GetADresult(i)*3.3)/1024.0;
			printf("a%d=%8.4f\t",i,d);
		}
		usleep(1);
		printf("\r");
	}
	/* Wait until producer and consumer finish. */
	pthread_join(th_com, &retval);

	printf("\n");
	return 0;
}
