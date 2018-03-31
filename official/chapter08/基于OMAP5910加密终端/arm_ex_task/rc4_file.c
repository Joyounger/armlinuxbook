#include <stdio.h>
#include <memory.h>
#include<string.h>
#include<stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#define buf_size 128
void usage(char *cmd)
{
fprintf(stderr,"%s filename\n",cmd);
}
int main(int argc, char **argv)
{   
    int fd;
    FILE *filep,*des;
    //char data[512];
    char *En_data;
    int rd;
    //int i;
	int sec;
	clock_t start,end;
    int cnt;
    char buf[buf_size];
   	//unsigned  long *En_buf;
    char *fname;
    char *devfn = "/dev/dsptask/rc4";
    	if (argc != 2)  
        {
		usage(argv[0]);
		return 1;
	}
	fname=argv[1];
	if((filep=fopen(fname,"r"))==NULL)
       { 
         fprintf(stderr, "cannot open %s\n", fname);
         exit(1);
        }
	strcat(fname,".rc4en");
        if((des=fopen(fname,"w"))==NULL)
        { 
         fprintf(stderr, "cannot open %s\n", fname);
         exit(1);
        }
    	fd = open(devfn, O_RDWR);
	if (fd < 0) {
		fprintf(stderr, "cannot open %s\n", devfn);
		return 1;
	}
	start=clock();
    rd = fread(buf,1,buf_size,filep) ;
    En_data=malloc(128);
    while (rd>0)
    {
	if(rd&1)rd++;
       cnt=write(fd, buf, rd);
        if (cnt < 0) 
        {
    			perror("read failed");
    	}
   			cnt = read(fd,En_data , buf_size);
		if (cnt < 0) 
        {
			perror("read failed");
        }
       fwrite(En_data,1,rd,des);
       rd = fread(buf,1,buf_size,filep);
    }


	
	close(fd);  
	fclose(filep);
    fclose(des);
	end=clock();
	sec=(end-start)/1000;
	printf("user time %d ms",sec);
}
