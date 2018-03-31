/*my system at arm*/
//arm and dsp change data 28 word(2048 byte)every time
///////////////////////////////////////////////////
/*header file*/
#include <stdio.h>   /*used to i/o*/
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <linux/fb.h>/*framebuffer*/
#include <fcntl.h>
#include "math.h"     /*used to mathematics operation*/
#include <linux/fb.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <fcntl.h>
///////////////////////////////////////////////////
/*macro definition*/
#define IMAGE_WIDTH  256/*bitmap wide*/
#define IMAGE_HEIGTH 256/*bitmap heigth*/
#define FB_START 0xE6000000 /*Framebuffer's start address in arm*/
#define FB_SIZE  0x04B000 /*Framebuffer's size is 640*480(307200)*/
#define FB_FILE "/dev/fb/0"
#define BUF_BIN_SIZE 128
#define BUF_SM_SIZE 108/*define every time deal data num is 108*/
#define BUF_MF_SIZE 120
#define BUF_ED_SIZE 108
#define FB_FILE "/dev/fb/0"
#define SCREEN_WIDTH 640
#define SCREEN_LENGTH 480
///////////////////////////////////////////////////////////////////
//Framebuffer结构体
struct SCREEN_CTRL
{
	int fb;
	void * fb_mem;
	int fb_width, fb_height, fb_line_len, fb_size;
	int fb_bpp;
};

static struct SCREEN_CTRL Fb_dev;
#define SCREEN_Fb_dev	      	"/dev/fb0" //屏幕驱动程序路径
static char *default_framebuffer = SCREEN_Fb_dev;//定义了一个屏幕
static int ad_driver_fd = -1;//打开的文件指针
/////////////////////////////////////////////////////////////////////

void menu();
int Binary();
/*int Smooth();
int Med_Filter();
int Edge();
void init(short a[],int num);*/


static void busywait(int cnt)/*timelag function*/

{

	int i;

	for (i = 0; i < cnt; i++) {}

}

/***********************************
*Framebuffer设备打开,初始化
***********************************/
int framebuffer_open()
{
		int fb;
		struct fb_var_screeninfo fb_vinfo;
		struct fb_fix_screeninfo fb_finfo;
		char * fb_dev_name = NULL;
		
		if (!(fb_dev_name = getenv("FRAMEBUFFER")))
			fb_dev_name = default_framebuffer;

		fb = open (fb_dev_name, O_RDWR);
		if (fb < 0 )
		{
				printf("device %s open failed.\n", fb_dev_name);
				return -1;
		}
			
		
		if (ioctl(fb, FBIOGET_VSCREENINFO, &fb_vinfo))
		{
				printf("Can't get VSCREENINFO: %s\n", strerror(errno));
				close(fb);
				return -1;
		}
			
		if (ioctl(fb, FBIOGET_FSCREENINFO, &fb_finfo))
		{
				printf("Can't get FSCREENINFO: %s\n", strerror(errno));
				return 1;
		}	
	
		Fb_dev.fb_bpp = fb_vinfo.red.length +  fb_vinfo.green.length +  fb_vinfo.blue.length +  fb_vinfo.transp.length;
		Fb_dev.fb_width = fb_vinfo.xres;
		Fb_dev.fb_height = fb_vinfo.yres;
		Fb_dev.fb_line_len = fb_finfo.line_length;
		Fb_dev.fb_size = fb_finfo.smem_len;
		
		if (Fb_dev.fb_bpp != 16) 
		{
			printf ("frame buffer must be 16bpp mode. \n");
			exit(0);
		}

		Fb_dev.fb_mem = mmap (NULL, Fb_dev.fb_size, PROT_READ | PROT_WRITE, MAP_SHARED, fb, 0);
		if (Fb_dev.fb_mem == NULL || (int) Fb_dev.fb_mem == -1) 
		{
			Fb_dev.fb_mem = NULL;
			printf("mmap failed.\n");
			close(fb);
			return -1;
		}

		Fb_dev.fb = fb;
		memset (Fb_dev.fb_mem, 0x0, Fb_dev.fb_size);
		return 0;
}

/*************************************
*Framebuffer设备关闭
**************************************/
void framebuffer_close()
{
	if (Fb_dev.fb_mem) 
	{
		munmap (Fb_dev.fb_mem, Fb_dev.fb_size);
		Fb_dev.fb_mem = NULL;
	}

	if (Fb_dev.fb) 
	{
		close (Fb_dev.fb);
		Fb_dev.fb = 0;
	}
}
void * GetFbmem()
{
	return Fb_dev.fb_mem;
}

int Binary()/*operate to data process   -------------------Binaryzation Process*/
{
	short int cnt;   /*the data byte write to dsp and read from dsp */
	int fd;   /*will open file*/
	FILE *fr;  /*file style*/
	int k;
	int i;
	int j;
	int bfOffBits;   /* bitmap real data's zero position. filehead offset's unit is byte*/
       unsigned short *p;
	char y;//read data from bitmap
	char *devfn;/*dev name which deal with data at dsp*/
	char *data_bmp = (char*)malloc(IMAGE_WIDTH*sizeof(char));
//---------------------------------------------------------------------
      devfn = "/dev/dsptask/graph1";/*if Binaryzation Process ,open this dev*/
//-------------------------open bitmap file Lena64.bmp  -----------------
  	fr=fopen("test.bmp","rb");/*open file at read only and binary */	/*bfOffBits is 10 at bitmap filehead */
	printf("this is OK!");//test
	fseek(fr,10L,0);/*move to bfOffBits*/
	fread(&bfOffBits,1,4,fr);/*get bfOffBits*///	printf("bfOffBits=%d\n",bfOffBits);
//---------------------------------get data from file-------------
	fseek(fr,bfOffBits,0);/*get all data*/
	printf("\n");
	fd=open(devfn,O_RDWR);/*open dev*/
	if(fd<0)/*if open error*/
       {
              fprintf(stderr, "cannot open %s\n", devfn);/*print*/
              return 1;/*return*/
       }
       printf("%s\n",devfn);
	if(framebuffer_open())
	{
		printf("fail in open framebuffer dev!\n" );
	}
	p = (unsigned short *)Fb_dev.fb_mem;//定义framebuffer大的内存空间

	for(i=0;i<IMAGE_HEIGTH;i++)
	{
		for(j=0;j<2;j++)
		{
			fread((data_bmp+128*j),BUF_BIN_SIZE,1,fr);/*get data*/
			fseek(fr,BUF_BIN_SIZE,SEEK_SET);
			//----------------------------write data to dsp-------------------------------
       	 	cnt=write(fd, (data_bmp+128*j),BUF_BIN_SIZE);/*ipbuf_d[bid] data byte size is num1*2*/
			if(cnt<0)
        		{
             		  	fprintf(stderr,"write failed!!\n");
               		return 1;
       		}
			//------------------------read data from dsp-------------------------------
        		cnt=read(fd,(data_bmp+128*j),BUF_BIN_SIZE);/*read data from dsp*/  
		}
		for(k=0;k<IMAGE_WIDTH;k++)
		{  
			p[i+k]=data_bmp[k];
	 	 
		}
	}
	free(data_bmp);
	close(fd);/*close dev*/
	fclose(fr);//close file
	framebuffer_close();
	return 0;
}

/*--------------------my system menu---------------------------*/
void menu()
{
	int a;
	printf("\n*****   IMAGE PROCESSING SYSTEM         *****\n");
	printf("\n*****  1.Binaryzation Process           *****\n");
	printf("\n*****  2.Smoothness Process             *****\n");
	printf("\n*****  3.Medial Filter Process          *****\n");
	printf("\n*****  4.Edge detction Process          *****\n");
	printf("\nPlease input your change:");
	scanf("%d",&a);
	switch(a)
		{/*choose 1 or 2 or 3 or 4 is right*/
		case 1:
				Binary();
				menu();
			//	break;
		default:/*other is wrong*/
				printf("\nYour input is Error!Please input your change again!!\n");
				menu();	
			//	break;
		}
}
//---------------------------------------------------------------------/
/**/
int main(int argc,char **argv)
{
	menu();/*transfer to menu function*/
	return 0;
}
