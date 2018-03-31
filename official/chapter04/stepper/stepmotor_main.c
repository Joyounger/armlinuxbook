/*
 * stepmotor_main.c
 *
 * S3C2410-S STEPMOTOR
 * derived from s3c2410_exio.c
 *
 * Author: wang bin <wbinbuaa@163.com>	
 * Date  : $Date: 2005/07/25 $ 
 *
 * $Revision: 1.0.0.1 $
 *						
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file COPYING in the main directory of this archive
 * for more details.
 */

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/ioctl.h>
#include "stepper.h"
static int step_fd = -1;


/****************************************************************/
int main(int argc, char **argv)
{
	unsigned int i = 0;
	
	if((step_fd=open(STEP_DEV, O_WRONLY))<0){
		printf("Error opening /dev/exio/0raw device\n");
		return 1;
	}
while(1)
{
	printf{"please select your operations :\n"};
	printf("1、open the stepper\n");
	printf("2、close the stepper\n");
	printf("3、reverse the stepper\n");
	printf("4、speed up stepper\n");
	printf("5、speed down stepper\n");
	printf("6、exit\n");
		
	scanf("%d",&i);
	if(i==6)
		ioctl(step_fd,STEPPER_STOP, 0);
		break;
	
	ioctl(step_fd,i, 0);
	sleep(5);
}
	
	close(step_fd);	
	printf("Step motor start running!\n");
	return 0;
}