/*my system at arm*/
///////////////////////////////////////////////////
/*header file*/
#include <stdio.h>   /*used to i/o*/
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <linux/fb.h>/*framebuffer*/
#include <fcntl.h>
#include "math.h"     /*used to mathematics operation*/
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <errno.h>
#include <malloc.h>
#define IMAGE_WIDTH  256/*bitmap wide*/
#define IMAGE_HEIGTH 256/*bitmap heigth*/
#define FB_START 0xE6000000 /*Framebuffer's start address in arm*/
#define FB_SIZE  0x04B000 /*Framebuffer's size is 640*480(307200)*/
#define FB_FILE "/dev/fb/0"
/*------------------------------------------------------*/
void Display(unsigned short int  da[IMAGE_HEIGTH][IMAGE_WIDTH],unsigned short int db[IMAGE_HEIGTH][IMAGE_WIDTH]);
void Smooth(unsigned short int list1[IMAGE_HEIGTH][IMAGE_WIDTH]);
unsigned short int  MedValue(unsigned short int buf[], int n, int m);
void Median(unsigned short int  list1[IMAGE_HEIGTH][IMAGE_WIDTH]);
void Sobel(unsigned short int list1[IMAGE_HEIGTH][IMAGE_WIDTH]);
void graph(int chan);
void menu();
///////////////////////////////////////////////////
///-------------------------------framebuffer info---------------------------------------------------------------------
//a framebuffer device structure;
typedef struct _fb_v4l
{
 // FrameBuffer info
   int fbfd ;            /* FrameBuffer dev handle*/
    struct fb_var_screeninfo vinfo;  /* FrameBuffer screen alterable info*/
    struct fb_fix_screeninfo finfo;  /* FrameBuffer unchangeable  info */
    char *fbp;            /* FrameBuffer EMS memory pointer*/
}fb_v41;

int main(int argc,char **argv)
{
	menu();/*transfer to menu function*/
	return 0;
}
int open_framebuffer(char *ptr,fb_v41 *vd)/*open FB*/
{
  int fbfd,screensize;
     // Open the file for reading and writing
    fbfd = open( ptr, O_RDWR);
    if (fbfd < 0)
    {
    printf("Error: cannot open framebuffer device.%x\n",fbfd);/*if open error*/
    return 1;
    }
    printf("The framebuffer device was opened successfully.\n");/*open right*/
    vd->fbfd = fbfd; /* 保存打开FrameBuffer设备的句柄*/
                                          // Get fixed screen information 获取FrameBuffer固定不变的信息
    if (ioctl(fbfd, FBIOGET_FSCREENINFO, &vd->finfo))
    {
            printf("Error reading fixed information.\n");
            return 1;
    }
    // Get variable screen information 获取FrameBuffer屏幕可变的信息
    if (ioctl(fbfd, FBIOGET_VSCREENINFO, &vd->vinfo))
    {
            printf("Error reading variable information.\n");
            return 1;
    }
 printf("%dx%d, %dbpp, xoffset=%d ,yoffset=%d \n", vd->vinfo.xres,
       vd->vinfo.yres, vd->vinfo.bits_per_pixel,vd->vinfo.xoffset,vd->vinfo.yoffset );
    // Figure out the size of the screen in bytes
    screensize = vd->vinfo.xres * vd->vinfo.yres * vd->vinfo.bits_per_pixel / 8;
         // Map the device to memory
    vd->fbp = (char *)mmap(0,screensize,PROT_READ|PROT_WRITE,MAP_SHARED,fbfd,0); /* 影射Framebuffer设备到内存*/    if ((int)vd->fbp == -1)
    {
            printf("Error: failed to map framebuffer device to memory.\n");
            return 1;
    }
    printf("The framebuffer device was mapped to memory successfully.\n");
  return  0;
}
void Display(unsigned short int  da[IMAGE_HEIGTH][IMAGE_WIDTH],unsigned short int db[IMAGE_HEIGTH][IMAGE_WIDTH])
{
	int x,y;
	fb_v41 vd;
	int ret,i,j;
	unsigned short  tempbuf[640*480];
	int location;
	unsigned short *loca_ptr;
	int xoffset;
	int yoffset;
	xoffset=0;
	yoffset=0;
	for(i=0;i<640*480;i++)
  		 tempbuf[i] = 0xffff;
	ret = open_framebuffer(FB_FILE,&vd);  /* Open FrameBuffer dev*/
//-----------------write white screen----------------------------------
	for (y = 0;y < 480;y++)    /* vertical scan*/
	  {
		    location = xoffset * 2 +
		            (y + yoffset) *(( vd.finfo).line_length); 
		    loca_ptr = (unsigned short *) (vd.fbp + location);  /*start point*/
		     for ( x = 0,i=0; x <640; x++,i++ )   /*line scan*/
			    {
			      *(loca_ptr + x) = tempbuf[i];/*write color*/
			    }
	  }
//-----------------------display bmp from file--------------------------
	xoffset=50;
	yoffset=100;
	for ( y = 0,i=IMAGE_HEIGTH; y < IMAGE_HEIGTH; y++,i-- )    /* 纵扫描*/
	  {
		    location = xoffset * 2 +
		            (y + yoffset) * ((vd.finfo).line_length); 
			printf("\nlocation=%d",location);
		    loca_ptr = (unsigned short *) (vd.fbp + location);         
		     for ( x = 0,j=0; x <IMAGE_WIDTH; x++ ,j++)   /* 行扫描  */
			    {
				printf(" %x",da[i][j]);
				*(loca_ptr+x)=((da[i][j]>>3)<<11)|((da[i][j]>>2)<<5)|(da[i][j]>>3);
	//			*(loca_ptr + x) = *da++;
			    }
	  }
	//----------------------proed bit bmp----------------------------------
	xoffset=350;
	yoffset=100;
	for ( y = 0,i=IMAGE_HEIGTH; y < IMAGE_HEIGTH; y++,i--)    /* 纵扫描*/
	  {
		    location = xoffset * 2 +
		            (y + yoffset) * ((vd.finfo).line_length); 
			printf("\nlocation=%d",location);
		    loca_ptr = (unsigned short *) (vd.fbp + location);         
		     for ( x = 0,j=0; x <IMAGE_WIDTH; x++,j++)   /* 行扫描  */
			    {
				printf(" %x",db[i][j]);
				if(db[i][j]==0)
					*(loca_ptr+x)=0;
				else
					if(db[i][j]==255)
						*(loca_ptr+x)=0xffff;
					else
						*(loca_ptr+x)=((db[i][j]>>3)<<11)|((db[i][j]>>2)<<5)|(da[i][j]>>3);
	//		      *(loca_ptr + x) = *db++;
			    }
	  }
       close(vd.fbfd);/*close fb*/
}
void Smooth(unsigned short int list1[IMAGE_HEIGTH][IMAGE_WIDTH])
{
  	unsigned int  i,j,g;
  	unsigned short int data_proed[IMAGE_HEIGTH][IMAGE_WIDTH];
	/*------------------------edge data to storage----------------------*/
	for(i=0,j=0;j<IMAGE_WIDTH;j++)
		data_proed[i][j]=list1[i][j];
	for(i=0;i<IMAGE_HEIGTH;i++)
		{
			data_proed[i][0]=list1[i][0];
			data_proed[i][IMAGE_WIDTH-1]=list1[i][IMAGE_WIDTH];
		}
	for(i=(IMAGE_HEIGTH-1),j=0;j<IMAGE_WIDTH;j++)
		data_proed[i][j]=list1[i][j];
	/*--------------------------data to process-----------------------*/
	for (i=1;i<IMAGE_HEIGTH-1;i++) {
		for (j=1;j<IMAGE_WIDTH-1;j++) {
		  g = (list1[i-1][j-1]+list1[i-1][j]+list1[i-1][j+1]+list1[i][j-1]+list1[i][j+1]+list1[i+1][j-1]+list1[i+1][j]+list1[i+1][j+1]); 
		  g=g>>3;//g=g/8
		  if (g>0xff) g=0xff;
		  else if (g<0) g=0;
		  data_proed[i][j] = (unsigned short int) g;                     //  结果送入输出行  
		}
	}
	Display(list1,data_proed);
   
}
unsigned short int  MedValue(unsigned short int buf[], int n, int m)	            /* array and return middle value*/
{
   int i,j,k,f;

   for (i=1;i<n;i++)
   {	                              //  冒泡排序
      f=0;	                                  //  biaozhi to zero
      for (j=n-1;j>=i;j--) 
      {
	 	if (buf[j]>buf[j+1]) 
	 	{	                          //  compare
			k=buf[j];
			buf[j]=buf[j+1]; //swap 
		 	buf[j+1]=k;
		 	f=1;
	    }
      }
      if (f==0) break;	                          //  if biaozhi is zero break
   }
   return(buf[m]);	                            //  return middle value
}
void Median(unsigned short int  list1[IMAGE_HEIGTH][IMAGE_WIDTH])
{
	int  i,j;
	unsigned short int buff[10];
	unsigned short int  data_proed[IMAGE_HEIGTH][IMAGE_WIDTH];
	/*------------------------edge data to storage----------------------*/
	for(i=0,j=0;j<IMAGE_WIDTH;j++)
		data_proed[i][j]=list1[i][j];
	for(i=0;i<IMAGE_HEIGTH;i++)
		{
			data_proed[i][0]=list1[i][0];
			data_proed[i][IMAGE_WIDTH-1]=list1[i][IMAGE_WIDTH];
		}
	for(i=(IMAGE_HEIGTH-1),j=0;j<IMAGE_WIDTH;j++)
		data_proed[i][j]=list1[i][j];
		printf("\n edge data to storage is OK!\n");
	/*--------------------------data to process-----------------------*/
	for (i=1;i<IMAGE_HEIGTH-1;i++) 
	{
		if(i%5==0)
			printf("\n");
		for (j=1;j<IMAGE_WIDTH-1;j++) 
		{
			 buff[0] = list1[i-1][j];               // bitmap data and 4 point's data      
			 buff[1] = list1[i][j];
			 buff[2] = list1[i+1][j];         
			 buff[3] = list1[i][j-1];
			 buff[4] = list1[i][j+1]; 
		printf("\n %x %x %x %x %x",buff[0],buff[1],buff[2],buff[3],buff[4]);              
			 data_proed[i][j] = (unsigned short int ) MedValue(buff, 5, 5/2);
							 //array and get middle value
			printf("(%d,%d)%d->%d",i,j,list1[i][j],data_proed[i][j]);
		}
	}
	Display(list1,data_proed);
}
void Sobel(unsigned short int list1[IMAGE_HEIGTH][IMAGE_WIDTH])
{
	  int  i,j,A,B,C;
	  unsigned short int scale,data_proed[IMAGE_HEIGTH][IMAGE_WIDTH];
	  /*------------------------edge data to storage----------------------*/
	for(i=0,j=0;j<IMAGE_WIDTH;j++)
		data_proed[i][j]=list1[i][j];
	for(i=0;i<IMAGE_HEIGTH;i++)
		{
			data_proed[i][0]=list1[i][0];
			data_proed[i][IMAGE_WIDTH-1]=list1[i][IMAGE_WIDTH];
		}
	for(i=(IMAGE_HEIGTH-1),j=0;j<IMAGE_WIDTH;j++)
		data_proed[i][j]=list1[i][j];
	/*--------------------------data to process-----------------------*/
	  for (i=1;i<IMAGE_HEIGTH-1;i++) {
	    for (j=1;j<IMAGE_WIDTH-1;j++) {
			/*--------------Y sobel suanzi------------------------*/
	      A = abs((list1[i-1][j+1]+2*list1[i][j+1]+list1[i+1][j+1])-
		          (list1[i-1][j-1]+2*list1[i][j-1]+list1[i+1][j-1]));
			/*--------------X sobel suanzi------------------------*/
	      B = abs((list1[i+1][j-1]+2*list1[i+1][j]+list1[i+1][j+1])-
		          (list1[i-1][j-1]+2*list1[i-1][j]+list1[i-1][j+1]));
			/*-------------------last big sobel suanzi is C-------------*/
	    	      if (A>B) C = A;
		      else     C = B;
			  /*------------------------C=C/4--------------*/
	            C=C>>2;
	      if (C>255) 
		  	C=255;
		  data_proed[i][j] = C;
		}
	  }
	  Display(list1,data_proed);
}
void graph(int chan)
{	
	FILE *fr;  /*file style*/
	int i,j,k;
	int bfOffBits;   /* bitmap real data's zero position. filehead offset's unit is byte*/
                                                      //             char *testdata="adfadfasfasfdafadfdsafadfadfaf";              //test data
	char y;//read data from bitmap
	unsigned short int data_get[IMAGE_HEIGTH][IMAGE_WIDTH];
	unsigned short int data_proed[IMAGE_HEIGTH][IMAGE_WIDTH];
//--------------------init--------------------------------------------
	for(i=0;i<IMAGE_HEIGTH;i++)
		for(j=0;j<IMAGE_WIDTH;j++)
			{
				data_get[i][j]=0;
				data_proed[i][j]=0;
			}
//	printf("\ninit ok");
	printf("%d\n",chan);
///-------------------------open bitmap file Lena64.bmp  -----------------
	if(chan==3)
	{
  	fr=fopen("median.bmp","rb");/*open file at read only and binary */	/*bfOffBits is 10 at bitmap filehead */
	printf("this is OK!");//test
	fseek(fr,10L,0);/*move to bfOffBits*/
	fread(&bfOffBits,1,4,fr);/*get bfOffBits*///	printf("bfOffBits=%d\n",bfOffBits);
//	printf("\nbfOffBits=%d",bfOffBits);
//---------------------------------get data from file-------------
	fseek(fr,bfOffBits,0);/*get all data*/
	printf("\n");
	for(i=0;i<IMAGE_HEIGTH;i++)
	{
		printf("\n");
		for(j=0;j<IMAGE_WIDTH;j++)
			{
				fread(&y,1,1,fr);/*get data*/
				data_get[i][j]=(short int)y;
				printf("%x ",data_get[i][j]);
			}
	}
	}
	else
	{
	fr=fopen("test.bmp","rb");/*open file at read only and binary */	/*bfOffBits is 10 at bitmap filehead */
	printf("this is OK!");//test
	fseek(fr,10L,0);/*move to bfOffBits*/
	fread(&bfOffBits,1,4,fr);/*get bfOffBits*///	printf("bfOffBits=%d\n",bfOffBits);
//	printf("\nbfOffBits=%d",bfOffBits);
//---------------------------------get data from file-------------
	fseek(fr,bfOffBits,0);/*get all data*/
	printf("\n");
	for(i=0;i<IMAGE_HEIGTH;i++)
	{
		printf("\n");
		for(j=0;j<IMAGE_WIDTH;j++)
			{
				fread(&y,1,1,fr);/*get data*/
				data_get[i][j]=(short int)y;
				printf("%x ",data_get[i][j]);
			}
	}
		
	}
	switch(chan)
		{
			case 1:
				{
					k = 128; 
					for (i=0; i<IMAGE_HEIGTH; i++)
					{
						for (j=0; j<IMAGE_WIDTH; j++)
						{
							//y[i][j] = 255*((y[i][j])/k);
							if(data_get[i][j] > k)//k为阀值128
							{
								data_proed[i][j] = 255;
							}
							else
							{
								data_proed[i][j] = 0;
							}
						}
					}
				}
			Display(data_get,data_proed);
			menu();
			case 2:
				Smooth(data_get);
				menu();
			case 3:
				Median(data_get);
				menu();
			case 4:
				Sobel(data_get);
				menu();
			default:
				printf("\n your input is error!\n");
				menu();
		}
}
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
	graph(a);
}
