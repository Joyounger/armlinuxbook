#include <std.h>

#include "mailbox.h"

#include "tokliBIOS.h"

#include <stdio.h>
#include <time.h>
#include <string.h>
//#extern unsigned long long wacth;
#include <stdio.h>
#include<stdlib.h>
#define buf_size 1024
typedef struct rc4_key//s-box包括使用的 i，j 
{ 
unsigned char state[256]; 
unsigned char x; 
unsigned char y;
} rc4_key;
#define swap_byte(x,y) t = *(x); *(x) = *(y); *(y) = t//swap（x，y） 
void prepare_key(unsigned char *key_data_ptr, int key_data_len, rc4_key *key)//初始化s-box并用key数据搅乱 
{
int i;
unsigned char t;
unsigned char swapByte;
unsigned char index1;
unsigned char index2;
unsigned char* state;
short counter;
state = &key->state[0];
for(counter = 0; counter < 256; counter++)//s-box赋值 
state[counter] = counter;
key->x = 0;
key->y = 0;
index1 = 0;
index2 = 0;
for(counter = 0; counter < 256; counter++)
{
index2 = (key_data_ptr[index1] + state[counter] + index2) % 256;
swap_byte(&state[counter], &state[index2]);
index1 = (index1 + 1) % key_data_len;//变化k【j】 
}
}
void rc4_enc(unsigned char *buffer_ptr, int buffer_len, rc4_key *key)//rc4加密 
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
for(counter = 0; counter < buffer_len; counter++)//产生k加密明文 
{
x = (x + 1) % 256;
y = (state[x] + y) % 256;
swap_byte(&state[x], &state[y]);
xorIndex = (state[x] + state[y]) % 256;
buffer_ptr[counter] ^= state[xorIndex];//xor运算明文 
}
key->x = x;//记录最后x 
key->y = y;//记录最后y 
}

static Void busywait(Uns cnt)
{
	Uns i;
	for (i = 0; i < cnt; i++) {}
}

static Uns rc4_rcv_bksnd(struct dsptask *task, Uns bid, Uns cnt)
{
   
	memcpy(task->udata, ipbuf_d[bid], cnt);
	unuse_ipbuf(task, bid);

	return 0;
}

static Uns rc4_rcv_bkreq(struct dsptask *task, Uns cnt)
{
	Uns bid;
    Int ipbuf_trycnt = 0;
    int n;
    unsigned char * data;
    rc4_key key;
    unsigned char *seed="haadakasdaksdldlaskdl;assdjaiwe";//key
    n=strlen(seed);
    data=(unsigned char*)task->udata;
    prepare_key(seed,n,&key);
    rc4_enc(data,128,&key);
   for (;;) {
    			bid = get_free_ipbuf(task);
    			if (bid != MBCMD_BID_NULL)
    				break;
    			if (++ipbuf_trycnt >= 100)
    				return MBCMD_EID_STVBUF;
    			busywait(100);
    		}
	

	memcpy(ipbuf_d[bid], task->udata, cnt);

	bksnd(task, bid, cnt);

	return 0;
}





static Uns bkdata[128];

#pragma DATA_SECTION(rc4, "dspgw_task")
struct dsptask rc4 = {
        TID_MAGIC,      /* tid */
        "rc4",        /* name */
	MBCMD_TTYP_GBDM | MBCMD_TTYP_GBMD |
	MBCMD_TTYP_BKDM | MBCMD_TTYP_BKMD |
	MBCMD_TTYP_PSND | MBCMD_TTYP_PRCV,
			/* ttyp: passive block snd, passive block rcv */
        rc4_rcv_bksnd,   /* rcv_snd */
        rc4_rcv_bkreq,   /* rcv_req */
        NULL,           /* rcv_tctl */
        NULL,           /* tsk_attrs */
        NULL,           /* mmap_info */
       	bkdata         /* udata */
};
