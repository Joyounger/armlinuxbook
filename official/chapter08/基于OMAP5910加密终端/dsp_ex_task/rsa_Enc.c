#include <std.h>

#include "mailbox.h"

#include "tokliBIOS.h"

#include <stdio.h>
#include <time.h>
#include <string.h>
//#extern unsigned long long wacth;

unsigned  long PowMod_Enc(unsigned  long base, unsigned  long pow, unsigned  long n);
inline unsigned  long MulMod(unsigned  long a, unsigned  long b, unsigned  long n);
static Void busywait(Uns cnt)
{
	Uns i;
	for (i = 0; i < cnt; i++) {}
}

static Uns rsa_Enc_rcv_bksnd(struct dsptask *task, Uns bid, Uns cnt)
{
  
	memcpy(task->udata, ipbuf_d[bid], cnt);
	unuse_ipbuf(task, bid);

	return 0;
}

static Uns rsa_Enc_rcv_bkreq(struct dsptask *task, Uns cnt)
{
	Uns bid;
    Int ipbuf_trycnt = 0;
	unsigned  long  *p;
	int i;
	Uns temp;
    unsigned  long  En_data;
    unsigned  long     n;
    unsigned  long     e;   //公匙，n=p*q，gcd(e,f)=1
    unsigned  long     d;      //私匙，e*d=1 (mod f)，gcd(n,d)=1
	Uns  t_data;
	Uns  data_8;
	Uns * data;
	int j;

   for (;;) {
    			bid = get_free_ipbuf(task);
    			if (bid != MBCMD_BID_NULL)
    				break;
    			if (++ipbuf_trycnt >= 100)
    				return MBCMD_EID_STVBUF;
    			busywait(100);
    		}
     
    n=15943;
    e=235;
    d=2203;
   	data=(Uns *)task->udata;
	p=(unsigned long *)ipbuf_d[bid];
   	j=0;
    for(i=0;i<cnt;i++)
    {
      t_data=  data[i];
      data_8=  data[i]&0xff;      
      En_data=PowMod_Enc(data_8,e,n);
	  p[j]=En_data;
	  j++;
	  data_8=  (data[i]>>8)&0xff; 
	  En_data=PowMod_Enc(data_8,e,n);
	  p[j]=En_data;
	  j++;
    }

//	uen_data=PowMod(En_data,d,n);

	
	
	/*	for( i=0;i<cnt*2+2;i=i+2)
		{	
            temp=task->udata[i];
		    task->udata[i] = task->udata[i+1];
			task->udata[i+1]=temp;
				
		}*/		
//	memcpy(ipbuf_d[bid], task->udata, cnt);

	

	bksnd(task, bid, cnt);

	return 0;
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
            b>>=1;            //a=a * a % n;    //函数看起来可以处理64位的整数，但由于这里a*a在a>=2^32时已经造成了溢出，因此实际处理范围没有64位
            a=MulMod(a, a, n);
        }        b--;        //c=a * c % n;        //这里也会溢出，若把64位整数拆为两个32位整数不知是否可以解决这个问题。
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


static Uns bkdata[128];

#pragma DATA_SECTION(rsa_Enc, "dspgw_task")
struct dsptask rsa_Enc = {
        TID_MAGIC,      /* tid */
        "rsa_Enc",        /* name */
	MBCMD_TTYP_GBDM | MBCMD_TTYP_GBMD |
	MBCMD_TTYP_BKDM | MBCMD_TTYP_BKMD |
	MBCMD_TTYP_PSND | MBCMD_TTYP_PRCV,
			/* ttyp: passive block snd, passive block rcv */
        rsa_Enc_rcv_bksnd,   /* rcv_snd */
        rsa_Enc_rcv_bkreq,   /* rcv_req */
        NULL,           /* rcv_tctl */
        NULL,           /* tsk_attrs */
        NULL,           /* mmap_info */
       	bkdata         /* udata */
};
