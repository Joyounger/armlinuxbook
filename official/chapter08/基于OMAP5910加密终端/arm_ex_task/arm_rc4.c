#include <stdio.h>
#include <memory.h>
#include<string.h>
#include<stdlib.h>
#include<time.h>
#define buf_size 1024
typedef struct rc4_key
{ 
unsigned char state[256]; 
unsigned char x; 
unsigned char y;
} rc4_key;
#define swap_byte(x,y) t = *(x); *(x) = *(y); *(y) = t
void prepare_key(unsigned char *key_data_ptr, int key_data_len, rc4_key *key)
{
//int i;
unsigned char t;
//unsigned char swapByte;
unsigned char index1;
unsigned char index2;
unsigned char* state;
short counter;
state = &key->state[0];
for(counter = 0; counter < 256; counter++)
state[counter] = counter;
key->x = 0;
key->y = 0;
index1 = 0;
index2 = 0;
for(counter = 0; counter < 256; counter++)
{
index2 = (key_data_ptr[index1] + state[counter] + index2) % 256;
swap_byte(&state[counter], &state[index2]);
index1 = (index1 + 1) % key_data_len;
}
}
void rc4(unsigned char *buffer_ptr, int buffer_len, rc4_key *key)
{
unsigned char t;
unsigned char x;
unsigned char y;
unsigned char* state;
unsigned char xorIndex;
short counter;
x = key->x;
y = key->y;
state = &key->state[0];
for(counter = 0; counter < buffer_len; counter++)
{
x = (x + 1) % 256;
y = (state[x] + y) % 256;
swap_byte(&state[x], &state[y]);
xorIndex = (state[x] + state[y]) % 256;
buffer_ptr[counter] ^= state[xorIndex];
}
key->x = x;
key->y = y;
}
void usage(char *cmd)
{
fprintf(stderr,"%s filename\n",cmd);
}
int main(int argc, char **argv)
{
char seed[256]; //key in fact
char data[]="hahahahahahahahahaha";
char * fname;
char *tn;
char buf[buf_size];
char digit[5];
int hex, rd,i;
int n;
int sec;
clock_t start,end;
rc4_key key;
FILE *fp,*des;
fname=argv[1];



if(strcmp(fname,"")==0)
{
fprintf(stderr,"can't find the name of the file for encryption.\n");
getchar();
return 1;
}
n = strlen(data);
if(n>512)
{
fprintf(stderr,"can't tolerate key longer than 512 characters.\n");
return 1;
}
if (n&1) //nÊÇÆæÊý
{
strcat(data,"0");
n++; 
}
n/=2;
memset(digit,0,5*sizeof(char));
strcpy(digit,"AA");
for (i=0;i<n;i++)
{
digit[2] = data[i*2];
digit[3] = data[i*2+1];
sscanf(digit,"%x",&hex); 
seed[i] = hex;  
}
prepare_key(seed,n,&key);
if((fp=fopen(fname,"r"))==NULL)
exit(1);
tn=strdup(fname);
strcat(fname,".arc");
if((des=fopen(fname,"w"))==NULL)

exit(1);
start=clock();
rd = fread(buf,1,buf_size,fp);

while (rd>0)
{
rc4(buf,rd,&key);
fwrite(buf,1,rd,des);
rd = fread(buf,1,buf_size,fp);
}
fclose(fp);
fclose(des);
end=clock();
sec=(end-start)/1000;
printf("used time %d ms",sec);


}

