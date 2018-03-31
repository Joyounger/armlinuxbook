#include <std.h>

#include "mailbox.h"

#include "tokliBIOS.h"

#include <stdio.h>
#include <time.h>
#include <string.h>
//#extern unsigned long long wacth;

unsigned  long PowMod_Dec(unsigned  long base, unsigned  long pow, unsigned  long n);
inline unsigned  long MulMod(unsigned  long a, unsigned  long b, unsigned  long n);
static Void busywait(Uns cnt)
{
	Uns i;
	for (i = 0; i < cnt; i++) {}
}

static Uns rsa_Dec_rcv_bksnd(struct dsptask *task, Uns bid, Uns cnt)
{
    
	memcpy(task->udata, ipbuf_d[bid], cnt);
	unuse_ipbuf(task, bid);

	return 0;
}

static Uns rsa_Dec_rcv_bkreq(struct dsptask *task, Uns cnt)
{
	Uns bid;
   Int ipbuf_trycnt = 0;
	unsigned  long  *p;
	unsigned  long  *q;
	int i;
	Uns temp;
	Uns *data;
	unsigned  long   uen_data;
    unsigned  long     n;
  //  unsigned  long     e;   //公匙，n=p*q，gcd(e,f)=1
    unsigned  long     d;      //私匙，e*d=1 (mod f)，gcd(n,d)=1
	unsigned long  sun;
//	data=task->udata;
    n=15943;
  //  e=235;
    d=2203;
    for (;;) {
    			bid = get_free_ipbuf(task);
    			if (bid != MBCMD_BID_NULL)
    				break;
    			if (++ipbuf_trycnt >= 100)
    				return MBCMD_EID_STVBUF;
    			busywait(100);
    		}
   	q=(unsigned long *)task->udata;
   	p=(unsigned long *)ipbuf_d[bid];
    for(i=0;i<cnt;i++)
    {
      uen_data=PowMod_Dec(q[i],d,n);
	  p[i]=uen_data;
    }

//	uen_data=PowMod(En_data,d,n);

	
	
		for( i=0;i<cnt*2+2;i=i+2)
		{	
	            temp=ipbuf_d[bid][i];
		    ipbuf_d[bid][i] = ipbuf_d[bid][i+1];
		    ipbuf_d[bid][i+1]=temp;
				
		}


/*    for (;;) {
    			bid = get_free_ipbuf(task);
    			if (bid != MBCMD_BID_NULL)
    				break;
    			if (++ipbuf_trycnt >= 100)
    				return MBCMD_EID_STVBUF;
    			busywait(100);
    		}*/
   //	q=(unsigned long *)task->udata;
    
//	memcpy(ipbuf_d[bid], task->udata, cnt);

	

	bksnd(task, bid, cnt);

	return 0;
}


/***************************
模幂运算，返回值 x=base^pow mod n
***************************/
unsigned  long PowMod_Dec(unsigned  long base, unsigned  long pow, unsigned  long n)
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

#pragma DATA_SECTION(rsa_Dec, "dspgw_task")
struct dsptask rsa_Dec = {
        TID_MAGIC,      /* tid */
        "rsa_Dec",        /* name */
	MBCMD_TTYP_GBDM | MBCMD_TTYP_GBMD |
	MBCMD_TTYP_BKDM | MBCMD_TTYP_BKMD |
	MBCMD_TTYP_PSND | MBCMD_TTYP_PRCV,
			/* ttyp: passive block snd, passive block rcv */
        rsa_Dec_rcv_bksnd,   /* rcv_snd */
        rsa_Dec_rcv_bkreq,   /* rcv_req */
        NULL,           /* rcv_tctl */
        NULL,           /* tsk_attrs */
        NULL,           /* mmap_info */
       	bkdata         /* udata */
};
