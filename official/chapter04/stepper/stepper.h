#ifndef  STEPPER_DRV_H
#define STEPPER_DRV_H

#include <linux/ioctl.h>

#define MAJOR_NUM 96

#define IOCTL_SET_MSG _IOR(MAJOR_NUM, 0, int*)

#define IOCTL_GET_MSG _IOR(MAJOR_NUM, 1, int*)

#define STEPPET_START 1
#define STEPPER_STOP 2
#define STEPPER_REVERSE 3
#define STEPPER_SPEED_UP 4
#define STEEPER_SPEED_DOWN 5

struct stepper{
    int CmdID;  //indicate the type of the command
    int Arg;    //if the command is start,reverse,up or down,
	            //user can use this argument to set the speed.
}; 
/* The name of the device file */
#define DEVICE_FILE_NAME "stepper"

#endif

