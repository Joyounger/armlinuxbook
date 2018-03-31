/*my system at dsp*/
//////////////////////////////////////header file////////////////////////////////////////
#include <std.h>
#include "mailbox.h"
#include "tokliBIOS.h"
///////////////////////////////////////////////////
/*macro definition*/
//#define IMAGE_WIDTH  64 /*bitmap wide*/
//#define IMAGE_HEIGTH 64/*bitmap heigth*/
//#define FB_START 0x300800 /*Framebuffer's start address in dsp*/
//#define FB_SIZE  0x04B000 /*Framebuffer's size is 640*480(307200)*/
#define BUF_BIN_SIZE 128/*define every time deal data num is this */
#define BUF_SM_SIZE 108/*define every time deal data num is 108*/
#define BUF_MF_SIZE 120/*define medial filter data size is 120*/
#define BUF_ED_SIZE 108
//////////////////////////////////////////process function area////////////////////////
static Void busywait(Uns cnt)/*timelag function*/
{
	Uns i;
	for (i = 0; i < cnt; i++) {}
}
static Uns * Ime_binary( Uns p[],Uns num)/*deal with graph data to binary data with this function-- Binaryzation Process*/
{
	Uns *q;
	Uns k=128;/*defined Threshold 128*/
	Uns i;
	q=(Uns *)malloc(BUF_BIN_SIZE);
	q=p;
//	i=0;
	////binary function////
	for(i=0;i<num;i++)
		{
			if(p[i]>k)/*above and beyond Threshold*/
				p[i]=255;
			else
				p[i]=0;/*less than Threshold*/
		}
//		while(p[i])
//			{
//				if(p[i]> k)/*above and beyond Threshold*/
//					p[i]=255;
//				if(p[i]<=k)
//					p[i]=0;/*less than Threshold*/
//				i++;		
//			}
	///////////////////////////////
     return q;
}
Uns unt_bk;
static Uns graph1_rcv_bksnd(struct dsptask *task,Uns bid,Uns cnt)/*while receive BLOCKSEND,deal with this function*/
{	
        // Uns *p;
	//  p=(Uns *)malloc(BUF_SIZE);
	 // p=task->udata;/*read data from ipbuf_d[bid]*/
	 unt_bk=cnt;
      memcpy(task->udata,ipbuf_d[bid],cnt);/*get data from ipbuf_d[bid]*/
	  unuse_ipbuf(task,bid);/*free this ipbuf_d[bid]*/
	  return 0;
}
static Uns graph1_rcv_bkreq(struct dsptask *task,Uns cnt)/*deal with bkreq function */
{
	Int i;
	Uns mydata[BUF_BIN_SIZE];
	Uns *data_pro;
	Uns s[BUF_BIN_SIZE];//will send to arm
	Uns bid;
	Int ipbuf_trycnt=0;
	                     //unsigned char s[32]="Congraturations!DSP is working";/*this data will send to arm*/
	data_pro=(Uns *)malloc(BUF_BIN_SIZE);
	memcpy(mydata,task->udata,unt_bk);
	data_pro=Ime_binary( mydata,unt_bk);/*process data to binary*/
	for(i=0;i<BUF_BIN_SIZE;i++,data_pro++)
		s[i]=*data_pro;
	for(;;)
	{
		bid=get_free_ipbuf(task);/*get new ipbuf_[bid] for send data to arm*/
		if(bid!=MBCMD_BID_NULL)/*if get,break*/
			break;
		if(++ipbuf_trycnt>=100)/*if not get ,wait*/
			return MBCMD_EID_STVBUF;
		busywait(100);
        }
//	memcpy(task->udata,s,BUF_SIZE);//data to ipbuf_d[bid]
	memcpy(ipbuf_d[bid],s,unt_bk);
	bksnd(task,bid,unt_bk);/*send data to ARM*/
	free(data_pro);
	return 0;
}
//--------------------------------Smoothness Process -----------------
/*Void Smooth5(BYTE list1[64][64],BYTE list0[64][64],int Dx,int Dy)//5 points smooth
{
   Scale=5;
   Mask[0][0]=Mask[0][2]=Mask[2][0]=Mask[2][2]=0;
   Mask[0][1]=Mask[2][1]=Mask[1][0]=Mask[1][2]=1;
   Mask[1][1]=1;
   Template(list1,list0,Dx,Dy);
}
*/
Uns unt_smooth;/*smooth function factual data num from arm*/
Uns* Smooth(Uns data[],Uns num)//9 points smooth
{
	Uns  i,j;
	LgUns g;
	Uns temp;
	Uns *p;
	Uns data_ret[BUF_SM_SIZE/9];
	p=(Uns *)malloc(unt_smooth/9);
	p=data_ret;
	 /*  ¾í»ý¼ÆËã  */
	for(i=0,j=0;i<num;j++)
		{
			g=(data[i+1]+data[i+2]+data[i+3]+data[i+4]+data[i+5]+data[i+6]+data[i+7]+data[i+8]);
			g=g>>3;
			  if (g>0xffff) temp=255;
		              else 
				if(g<0)
				 temp=0;
				else 
				temp=(Uns)g;
		         data_ret[j]=temp;
			i=i+9;
		}
	return p;
} 
static Uns graph2_rcv_bksnd(struct dsptask *task,Uns bid,Uns cnt)/*while receive BLOCKSEND,deal with this function*/
{
	unt_smooth=cnt;
	memcpy(task->udata,ipbuf_d[bid],cnt);/*get data from ipbuf_d[bid]*/
	unuse_ipbuf(task,bid);/*free this ipbuf_d[bid]*/
	return 0;
}
static Uns graph2_rcv_bkreq(struct dsptask *task,Uns cnt)/*deal with bkreq function */
{
	Int i;
	Uns mydata[BUF_SM_SIZE];
	Uns *data_pro;
	Uns s[BUF_SM_SIZE/9];//will send to arm
	Uns bid;
	Int ipbuf_trycnt=0;
	                     //unsigned char s[32]="Congraturations!DSP is working";/*this data will send to arm*/
	data_pro=(Uns *)malloc(BUF_SM_SIZE/9);
	memcpy(mydata,task->udata,unt_smooth);/*get data*/
	data_pro=Smooth( mydata,unt_smooth);/*process data to binary*/
	for(i=0;i<(unt_smooth/9);i++,data_pro++)
		s[i]=*data_pro;
	for(;;)
	{
		bid=get_free_ipbuf(task);/*get new ipbuf_[bid] for send data to arm*/
		if(bid!=MBCMD_BID_NULL)/*if get,break*/
			break;
		if(++ipbuf_trycnt>=100)/*if not get ,wait*/
			return MBCMD_EID_STVBUF;
		busywait(100);
        }
	memcpy(ipbuf_d[bid],s,(unt_smooth/9));
	bksnd(task,bid,(unt_smooth/9));/*send data to ARM*/
	free(data_pro);
	return 0;
}
Uns Med_Sort(Uns array[],Uns n)
{
	Uns i,j;
	Uns temp,k;
	Uns f;
	for(i=1;i<n;i++)/*bubble sort */
		{
		f=0;
		for(j=n-1;j>=i;j--)
			{
				if(array[j]>array[j+1])
					{
					temp=array[j];
					array[j]=array[j+1];
					array[j+1]=temp;
					f=1;
					}
			}
		if(f==0) break;
		}
	k=array[n/2];/*return middle data*/
	return k;
}
Uns *Med_Filter(Uns da[],Uns loop)
{
	Uns i,j;
	Uns l;
	Uns lm;
	Uns ma[5];
	Uns ret[BUF_MF_SIZE/5];
	Uns temp;
	Uns *p;
	p=(Uns *)malloc(loop/5);
	p=ret;
	for(j=0;j<5;j++)
		ma[j]=0;
	lm=0;
	l=0;
	for(i=0;i<loop;i++)
		{
			ma[l]=da[i];/*get 5 points to process*/
			l++;
			if(l==5)
				{
					temp=Med_Sort(ma,5);/*sort array and get middle value*/
					ret[lm]=temp;
					for(j=0;j<5;j++)/*init*/
							ma[j]=0;
					lm++;
					l=0;
				}
		}
	return p;
}
Uns unt_mf;
static Uns graph3_rcv_bksnd(struct dsptask *task,Uns bid,Uns cnt)
{
	unt_mf=cnt;
	memcpy(task->udata,ipbuf_d[bid],cnt);/*get data from ipbuf_d[bid]*/
	unuse_ipbuf(task,bid);/*free this ipbuf_d[bid]*/
	return 0;
}
static Uns graph3_rcv_bkreq(struct dsptask *task,Uns cnt)
{
	Int i;
	Uns mydata[BUF_MF_SIZE];
	Uns *data_pro;
	Uns s[BUF_MF_SIZE/5];//will send to arm
	Uns bid;
	Int ipbuf_trycnt=0;
	                     //unsigned char s[32]="Congraturations!DSP is working";/*this data will send to arm*/
	data_pro=(Uns *)malloc(unt_mf/5);
	memcpy(mydata,task->udata,unt_mf);
	data_pro=Med_Filter( mydata,unt_mf);/*process data to binary*/
	for(i=0;i<(cnt/5);i++,data_pro++)
		s[i]=*data_pro;
	for(;;)
	{
		bid=get_free_ipbuf(task);/*get new ipbuf_[bid] for send data to arm*/
		if(bid!=MBCMD_BID_NULL)/*if get,break*/
			break;
		if(++ipbuf_trycnt>=100)/*if not get ,wait*/
			return MBCMD_EID_STVBUF;
		busywait(100);
        }
	memcpy(ipbuf_d[bid],s,unt_mf/5);
	bksnd(task,bid,unt_mf/5);/*send data to ARM*/
	free(data_pro);
	return 0;

}
Uns *Edge(Uns array[],Uns Num)
{
	Uns Scale;
	Uns temp_a,temp_b,temp_c;
	Uns i,j;
	Uns ra[BUF_ED_SIZE/9];
	Uns* q;
	q=(Uns *)malloc(Num/9);
	q=ra;
	j=0;
	Scale=4;
	for(i=0;i<Num;i+=9)
		{/*edge*/
			temp_a=abs((array[i+3]+2*array[i+5]+array[i+8])-(array[i+1]+2*array[i+4]+array[i+6]));
			temp_b=abs((array[i+6]+2*array[i+7]+array[i+8])-(array[i+1]+2*array[i+2]+array[i+3]));
//			temp_c=(temp_a>temp_b?temp_a:temp_b);
			if(temp_a>temp_b) temp_c=temp_a;
			temp_c=temp_b;
			temp_c/=Scale;
			if(temp_c>255) temp_c=255;
			ra[j]=temp_c;
			j++;
		}
	return q;
}
Uns unt_edge;
static Uns graph4_rcv_bksnd(struct dsptask *task,Uns bid,Uns cnt)
{
	unt_edge=cnt;/*unt_edge is factual data num from arm*/
	memcpy(task->udata,ipbuf_d[bid],cnt);/*get data from ipbuf_d[bid]*/
	unuse_ipbuf(task,bid);/*free this ipbuf_d[bid]*/
	return 0;
}
static Uns graph4_rcv_bkreq(struct dsptask *task,Uns cnt)
{
	Int i;
	Uns mydata[BUF_ED_SIZE];
	Uns *data_pro;
	Uns s[BUF_ED_SIZE/9];//will send to arm
	Uns bid;
	Int ipbuf_trycnt=0;
	                     //unsigned char s[32]="Congraturations!DSP is working";/*this data will send to arm*/
	data_pro=(Uns *)malloc(BUF_ED_SIZE/9);
	memcpy(mydata,task->udata,unt_edge);
	data_pro=Med_Filter( mydata,unt_edge);/*process data to binary*/
	for(i=0;i<(unt_edge/9);i++,data_pro++)
		s[i]=*data_pro;
	for(;;)
	{
		bid=get_free_ipbuf(task);/*get new ipbuf_[bid] for send data to arm*/
		if(bid!=MBCMD_BID_NULL)/*if get,break*/
			break;
		if(++ipbuf_trycnt>=100)/*if not get ,wait*/
			return MBCMD_EID_STVBUF;
		busywait(100);
        }
	memcpy(ipbuf_d[bid],s,unt_edge/9);
	bksnd(task,bid,unt_edge/9);/*send data to ARM*/
	free(data_pro);
	return 0;

}
////////////////////////////////////////////////////////////////////////
static Uns bkdata[BUF_BIN_SIZE];
#pragma DATA_SECTION(task_graph1, "dspgw_task")/*allocate EMS memory for DSP application program(Binaryzation Process)*/
struct dsptask task_graph1 = {
	TID_MAGIC,	/* tid */
	"graph1",	/* name */
	MBCMD_TTYP_GBDM | MBCMD_TTYP_GBMD |
	MBCMD_TTYP_BKDM | MBCMD_TTYP_BKMD |
	MBCMD_TTYP_PSND | MBCMD_TTYP_PRCV,
			/* ttyp: passive block snd, passive block rcv */
	graph1_rcv_bksnd,	/* rcv_snd */
	graph1_rcv_bkreq,	/* rcv_req */
	NULL,		/* rcv_tctl */
	NULL,		/* tsk_attrs */
	NULL,		/* mmap_info */
	bkdata		/* udata */
};

static Uns  smooth_bkdata[BUF_SM_SIZE];
#pragma DATA_SECTION(task_graph2, "dspgw_task")/*allocate EMS memory for DSP application program(Smoothness Process )*/
struct dsptask task_graph2= {
	TID_MAGIC,	/* tid */
	"graph2",	/* name */
	MBCMD_TTYP_GBDM | MBCMD_TTYP_GBMD |
	MBCMD_TTYP_BKDM | MBCMD_TTYP_BKMD |
	MBCMD_TTYP_PSND | MBCMD_TTYP_PRCV,
			/* ttyp: passive block snd, passive block rcv */
	graph2_rcv_bksnd,	/* rcv_snd */
	graph2_rcv_bkreq,	/* rcv_req */
	NULL,		/* rcv_tctl */
	NULL,		/* tsk_attrs */
	NULL,		/* mmap_info */
	smooth_bkdata  /* data*/
};
static Uns graph3_bkdata[BUF_MF_SIZE];
#pragma DATA_SECTION(task_graph3, "dspgw_task")/*allocate EMS memory for DSP application program(Medial Filter Process)*/
struct dsptask task_graph3= {
	TID_MAGIC,	/*tid */
	"graph3",	/* name*/
	MBCMD_TTYP_GBDM | MBCMD_TTYP_GBMD |
	MBCMD_TTYP_BKDM | MBCMD_TTYP_BKMD |
	MBCMD_TTYP_PSND | MBCMD_TTYP_PRCV,
			/* ttyp: passive block snd, passive block rcv */
	graph3_rcv_bksnd,	/* rcv_snd */
	graph3_rcv_bkreq,	/* rcv_req */
	NULL,		/* rcv_tctl */
	NULL,		/* tsk_attrs */
	NULL,		/* mmap_info */
	graph3_bkdata /* data*/
};
static Uns graph4_bkdata[BUF_ED_SIZE];
#pragma DATA_SECTION(task_graph4, "dspgw_task")/*allocate EMS memory for DSP application program(Edge detction Process)*/
struct dsptask task_graph4= {
	TID_MAGIC,	/* tid */
	"graph4",	/* name*/
	MBCMD_TTYP_GBDM | MBCMD_TTYP_GBMD |
	MBCMD_TTYP_BKDM | MBCMD_TTYP_BKMD |
	MBCMD_TTYP_PSND | MBCMD_TTYP_PRCV,
			/* ttyp: passive block snd, passive block rcv */
	graph4_rcv_bksnd,	/* rcv_snd */
	graph4_rcv_bkreq,	/* rcv_req */
	NULL,		/* rcv_tctl */
	NULL,		/* tsk_attrs */
	NULL,		/* mmap_info */
	graph4_bkdata /* data*/
};

