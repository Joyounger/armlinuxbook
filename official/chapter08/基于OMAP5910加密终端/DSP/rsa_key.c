#include <std.h>

#include "mailbox.h"

#include "tokliBIOS.h"

#include <stdio.h>
#include <time.h>
#include <string.h>

//小素数表
const static long       g_PrimeTable[]=
{
    3,
    5,
    7,
    11,
    13,
    17,
    19,
    23,
    29,
    31,
    37,
    41,
    43,
    47,
    53,
    59,
    61,
    67,
    71,
    73,
    79,
    83,
    89,
    97
};
const static long       g_PrimeCount=sizeof(g_PrimeTable) / sizeof(long);

const unsigned  long  multiplier=1274723821;

const unsigned  long  adder=1435454186;

typedef struct  RSA_PARAM_Tag
{
    unsigned  long   p;
    unsigned  long  q;   
    unsigned  long    f;      
    unsigned  long   n;
    unsigned  long   e;   //公匙，n=p*q，gcd(e,f)=1
    unsigned  long    d;      //私匙，e*d=1 (mod f)，gcd(n,d)=1
    unsigned  long    s;      //块长，满足2^s<=n的最大的s，即log2(n)
} RSA_PARAM;

RSA_PARAM RsaGetParam(void);

unsigned  long RandomPrime(char bits);

unsigned  long Random(unsigned  long n);

unsigned  long SteinGcd(unsigned  long p, unsigned  long q);


unsigned  long Euclid(unsigned  long a, unsigned  long b);

inline unsigned  long MulMod(unsigned  long a, unsigned  long b, unsigned  long n);

unsigned long RabinMiller(unsigned  long n, unsigned long loop);

long RabinMillerKnl(unsigned  long n);

unsigned  long PowMod_key(unsigned  long base, unsigned  long pow, unsigned  long n);

static Void busywait(Uns cnt)
{
	Uns i;
	for (i = 0; i < cnt; i++) {}
}

static Uns rsa_key_rcv_wdsnd(struct dsptask *task, Uns data)
{
	Uns bid;
	Int ipbuf_trycnt = 0;
	unsigned  long  *p;
	int i;
	Uns temp;
	RSA_PARAM           r;
		for (;;) {
			bid = get_free_ipbuf(task);
			if (bid != MBCMD_BID_NULL)
				break;
			if (++ipbuf_trycnt >= 100)
				return MBCMD_EID_STVBUF;
			busywait(100);
		}
/*		for(i=0;i<128;i++)ipbuf_d[bid][i]=i;
    bksnd(task, bid, 128);*/
	r=RsaGetParam();

	for (;;) {
			bid = get_free_ipbuf(task);
			if (bid != MBCMD_BID_NULL)
				break;
			if (++ipbuf_trycnt >= 100)
				return MBCMD_EID_STVBUF;
			busywait(100);
		}
	p=(unsigned long *)ipbuf_d[bid];
//	sun=n*2;

	p[0]=r.n;
	p[1]=r.e;
	p[2]=r.d;

		for( i=0;i<20;i=i+2)
		{	temp=ipbuf_d[bid][i];
			
			ipbuf_d[bid][i] = ipbuf_d[bid][i+1];
			ipbuf_d[bid][i+1]=temp;
			
			
		}
		
	bksnd(task, bid, 10);


	
	return 0;
	
}

RSA_PARAM RsaGetParam(void)
{
    RSA_PARAM           Rsa={ 0 };
    unsigned  long    t;
    Rsa.p=RandomPrime(8);          //随机生成两个素数
    Rsa.q=RandomPrime(7);
    if(Rsa.p==Rsa.q)
    Rsa.q=RandomPrime(8);
    Rsa.n=Rsa.p * Rsa.q;
    Rsa.f=(Rsa.p - 1) * (Rsa.q - 1);
    do
    {
        Rsa.e=Random(256);  //小于2^16，65536=2^16
        Rsa.e|=1;                   //保证最低位是1，即保证是奇数，因f一定是偶数，要互素，只能是奇数
    } 
	while(SteinGcd(Rsa.e,Rsa.f)!=1);
  //  while(SteinGcd(Rsa.e, Rsa.f) != 1) ;
	Rsa.d=Euclid(Rsa.e, Rsa.f);
    Rsa.s=0;
    t=Rsa.n >> 1;
    while(t)
    {
        Rsa.s++;                    //s=log2(n)
        t>>=1;
    }    return Rsa;
}
/*
随机生成一个bits位(二进制位)的素数，最多32位
*/
unsigned  long RandomPrime(char bits)
{
    unsigned  long    base;
    do
    {
        base= (unsigned long)1 << (bits - 1);   //保证最高位是1
        base+=Random(base);               //再加上一个随机数
        base|=1;    //保证最低位是1，即保证是奇数
    } while(!RabinMiller(base, 10));    //进行拉宾－米勒测试10次
    return base;    //全部通过认为是素数
}
//产生一个小于小于n的随机数
unsigned  long Random(unsigned  long n)
{		
	unsigned  long randSeed;
	randSeed= (unsigned  long)time(NULL);
  randSeed=multiplier * randSeed + adder;
  return randSeed % n;
}
/*
Stein法求最大公约数
*/
unsigned  long SteinGcd(unsigned  long p, unsigned  long q)
{
    unsigned  long    a=p > q ? p : q;
    unsigned  long    b=p < q ? p : q;
    unsigned  long    t, r=1;
    if(p == q)
    {
        return p;           //两数相等，最大公约数就是本身
    }
    else
    {
        while((!(a & 1)) && (!(b & 1)))
        {
            r<<=1;          //a、b均为偶数时，gcd(a,b)=2*gcd(a/2,b/2)
            a>>=1;
            b>>=1;
        }        
		if(!(a & 1))
        {
            t=a;            //如果a为偶数，交换a，b
            a=b;
            b=t;
        }       
		do
        {
            while(!(b & 1))
            {
                b>>=1;      //b为偶数，a为奇数时，gcd(b,a)=gcd(b/2,a)
            }            if(b < a)
            {
                t=a;        //如果b小于a，交换a，b
                a=b;
                b=t;
            }            b=(b - a) >> 1; //b、a都是奇数，gcd(b,a)=gcd((b-a)/2,a)
        }
		while(b);
        return r * a;
    }
}

unsigned  long Euclid(unsigned  long a, unsigned  long b)
{
    unsigned  long    m, e, i, j, x, y;
    long                xx, yy;
    m=b;
    e=a;
    x=0;
    y=1;
    xx=1;
    yy=1;
    while(e)
    {
        i=m / e;
        j=m % e;
        m=e;
        e=j;
        j=y;
        y*=i;
        if(xx == yy)
        {
            if(x > y)
            {
                y=x - y;
            }
            else
            {
                y-=x;
                yy=0;
            }
        }
        else
        {
            y+=x;
            xx=1 - xx;
            yy=1 - yy;
        }        x=j;
    }    if(xx == 0)
    {
        x=b - x;
    }    return x;
}
/*
Rabin-Miller素数测试，循环调用核心loop次
全部通过返回1，否则返回0
*/
unsigned long RabinMiller(unsigned  long n, unsigned long loop)
{	long i;
    //先用小素数筛选一次，提高效率
    for( i=0; i < g_PrimeCount; i++)
    {
        if(n % g_PrimeTable[i] == 0)
        {
            return 0;
        }
    }    //循环调用Rabin-Miller测试loop次，使得非素数通过测试的概率降为(1/4)^loop
    for(i=0; i < loop; i++)
    {
        if(!RabinMillerKnl(n))
        {
            return 0;
        }
    }    return 1;
}
/*
Rabin-Miller素数测试，通过测试返回1，否则返回0。
n是待测素数。
注意：通过测试并不一定就是素数，非素数通过测试的概率是1/4
*/
long RabinMillerKnl(unsigned  long n)
{
    unsigned  long    b, m, j, v, i;
    m=n - 1;
    j=0;    //0、先计算出m、j，使得n-1=m*2^j，其中m是正奇数，j是非负整数
    while(!(m & 1))
    {
        ++j;
        m>>=1;
    }    //1、随机取一个b，2<=b<n-1
    b=2 + Random(n - 3);    //2、计算v=b^m mod n
    v=PowMod_key(b, m, n);    //3、如果v==1，通过测试
    if(v == 1)
    {
        return 1;
    }    //4、令i=1
    i=1;    //5、如果v=n-1，通过测试
    while(v != n - 1)
    {
        //6、如果i==l，非素数，结束
        if(i == j)
        {
            return 0;
        }        //7、v=v^2 mod n，i=i+1
        v=PowMod_key(v, 2, n);
        ++i;        //8、循环到5
    }    return 1;
}
/*
模幂运算，返回值 x=base^pow mod n
*/
unsigned  long PowMod_key(unsigned  long base, unsigned  long pow, unsigned  long n)
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

/*
模乘运算，返回值 x=a*b mod n
*/
inline unsigned  long MulMod(unsigned  long a, unsigned  long b, unsigned  long n)
{
    return a * b % n;
}


#pragma DATA_SECTION(rsa_key, "dspgw_task")
struct dsptask rsa_key = {
        TID_MAGIC,      /* tid */
        "rsa_key",        /* name */
	MBCMD_TTYP_GBDM |
	MBCMD_TTYP_BKDM | MBCMD_TTYP_WDMD |

	MBCMD_TTYP_ASND | MBCMD_TTYP_PRCV,
			/* ttyp: active block snd, passive word rcv */
        rsa_key_rcv_wdsnd,   /* rcv_snd */
        NULL,          /* rcv_req */
        NULL,           /* rcv_tctl */
        NULL,           /* tsk_attrs */
        NULL,           /* mmap_info */
        NULL,         /* udata */
};
