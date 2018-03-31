#include <stdio.h>
#include <memory.h>
#include<string.h>
#include<stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#define buf_size 128
unsigned  long PowMod_Enc(unsigned  long base, unsigned  long pow, unsigned  long n);
inline unsigned  long MulMod(unsigned  long a, unsigned  long b, unsigned  long n);
void usage(char *cmd)
{
fprintf(stderr,"%s filename\n",cmd);
}

int main(int argc, char **argv)
{   
    FILE *filep,*des;
    //char data[512];
    int rd;
    int i;
    int sec;
	clock_t start,end;
    char buf[buf_size];
	//unsigned  long  *p;
    unsigned  long  En_data[buf_size];
    unsigned  long     n;
    unsigned  long     e;   //公匙，n=p*q，gcd(e,f)=1
	//int j;
    char *fname;
    n=15943;
    e=235;
    fname=argv[1];

	if((filep=fopen(fname,"r"))==NULL)
       { 
         fprintf(stderr, "cannot open %s\n", fname);
         exit(1);
        }
	strcat(fname,".en");
	   if((des=fopen(fname,"w"))==NULL)
        { 
         fprintf(stderr, "cannot open %s\n", fname);
         exit(1);
        }
start=clock();
    rd = fread(buf,1,buf_size,filep) ;
    while(rd>0)
    {             
        
        for(i=0;i<rd;i++)
        { 
        En_data[i]=PowMod_Enc(buf[i],e,n);
        }
        
        fwrite(En_data,4,rd,des);
        rd = fread(buf,1,buf_size,filep);
    }
	fclose(filep);
    fclose(des);
end=clock();
sec=(end-start)/1000;
printf("used time %d ms",sec);

}
/***************************
模幂运算，返回值 x=base^pow mod n
***************************/
unsigned  long PowMod_Enc(unsigned  long base, unsigned  long pow, unsigned  long n)
{
    unsigned  long    a=base, b=pow, c=1;
    while(b)
    {
        while(!(b & 1))
        {
            b>>=1;            //a=a * a % n;    
            a=MulMod(a, a, n);
        }        b--;        //c=a * c % n;      
	 c=MulMod(a, c, n);
    }    return c;
}

/**********************
模乘运算，返回值 x=a*b mod n
***********************/
inline unsigned  long MulMod(unsigned  long a, unsigned  long b, unsigned  long n)
{
    return a * b % n;
}
