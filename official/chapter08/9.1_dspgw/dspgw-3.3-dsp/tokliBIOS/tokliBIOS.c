/*
 * Copyright (c) 2002-2005, Nokia
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer. 
 *
 * Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution. 
 *
 * Neither the name of Nokia nor the names of its contributors may be used to
 * endorse or promote products derived from this software without specific
 * prior written permission. 
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * Toshihiro Kobayashi <toshihiro.kobayashi@nokia.com>
 * 2005/06/23:  DSP Gateway version 3.3
 */

#include <std.h>
#include <sys.h>
#include <mem.h>
#include <stdarg.h>
#include <extaddr.h>
#include "omap1510.h"
#include "mailbox.h"
#include "tokliBIOS.h"
#include "tokliBIOSlib.h"
#include "tokliBIOScfg.h"

/*
struct dspgw_version {
	Uns major;
	Uns minor;
	Uns extra1;
	Uns extra2;
};
*/
asm(" .sect \"dspgw_version\"\n"
    " .ushort 3\n"
    " .ushort 3\n"
    " .ushort 0\n"
    " .ushort 0\n");

struct task_prop *task_prop[N_TASK_MAX];
struct dsptask task_anon;

#pragma DATA_SECTION(mbseq, "SDRAM_system_data");
struct sync_seq mbseq;
static Uns mbseq_chklevel = MBSEQ_CHKLEVEL;
Uns mb_active = 0;

/*
 * syncword for each memory type
 * probably having ones both for DARAM and SARAM is redundant, but just in case.
 */
#pragma DATA_SECTION(DARAM_seq, "DARAM_seq")
#pragma DATA_SECTION(SARAM_seq, "SARAM_seq")
#pragma DATA_SECTION(SDRAM_seq, "SDRAM_seq")
static struct sync_seq DARAM_seq, SARAM_seq, SDRAM_seq;
struct mem_sync_struct mem_sync = {
	&DARAM_seq,
	&SARAM_seq,
	&SDRAM_seq,
};

struct ipbuf **ipbuf;
Uns **ipbuf_d;
static struct ipblink ipb_free;

static Uns ipbuf_bsycnt;
#pragma DATA_ALIGN(ipbuf_sys_da, 2)
#pragma DATA_ALIGN(ipbuf_sys_ad, 2)
#pragma DATA_SECTION(ipbuf_sys_da, "SDRAM_system_data")
#pragma DATA_SECTION(ipbuf_sys_ad, "SDRAM_system_data")
struct ipbuf_sys ipbuf_sys_da, ipbuf_sys_ad;

/*
 * system functions
 */
static Int init_status = 0;

Int init_done(Void)
{
	return init_status;
}

enum mem_type_e mem_type(Void *adr, Uns len, enum ptr_type_e type)
{
	/* byte address */
	LgUns ds = (LgUns)DARAM_BASE << 1;
	LgUns de = ((LgUns)DARAM_BASE + DARAM_SIZE) << 1;
	LgUns ss = (LgUns)SARAM_BASE << 1;
	LgUns se = ((LgUns)SARAM_BASE + SARAM_SIZE) << 1;
	LgUns a = (LgUns)adr;

	if (type == PTR_DATA)
		a <<= 1;

	if ((a >= ds) && (a < de)) {
		if (a + len > de)
			return MEM_TYPE_CROSSING;
		else
			return MEM_TYPE_DARAM;
	} else if ((a >= ss) && (a < se)) {
		if (a + len > se)
			return MEM_TYPE_CROSSING;
		else
			return MEM_TYPE_SARAM;
	} else {
		return MEM_TYPE_EXTERN;
	}
}

Void mbsend(Uns cmd, Uns data)
{
	Uns intm_saved;

	do {} while(inw(_DSP2ARM_Flag));

	if (!mb_active) {
		mb_active = 1;
		enable_domain(_ICR_DMA_IDLE_DOMAIN);
	}

	// need atomic operation
	intm_saved = HWI_disable();
	cmd |= (mbseq.da_dsp << 15);
	mbseq.da_dsp++;
	outw(data, _DSP2ARM);
	outw(cmd,  _DSP2ARMb);
	HWI_restore(intm_saved);
}

Void mb2send(Uns cmd, Uns data)
{
	Uns intm_saved;

	do {} while(inw(_DSP2ARM2_Flag));

	if (!mb_active) {
		mb_active = 1;
		enable_domain(_ICR_DMA_IDLE_DOMAIN);
	}

	// need atomic operation
	intm_saved = HWI_disable();
	outw(data, _DSP2ARM2);
	outw(cmd,  _DSP2ARM2b);
	HWI_restore(intm_saved);
}

Uns release_ipbuf(Uns bid)
{
	Uns intm_saved;

	if (ipbuf[bid]->ld == MBCMD_TID_FREE)	/* not locked? */
		return MBCMD_EID_NOTLOCKED;
	ipbuf[bid]->ld = MBCMD_TID_FREE;
	ipbuf[bid]->sd = MBCMD_TID_FREE;
	intm_saved = HWI_disable();
	ipblink_add_tail(&ipb_free, bid, ipbuf);
	HWI_restore(intm_saved);

	return 0;
}

/*
 * balancing ipbuf lines with ARM
 */
Void balance_ipbuf(Void)
{
	Uns bid;

	while (ipbuf_bsycnt <= _ipbuf_lines / 4) {
		bid = get_free_ipbuf(&task_anon);
		if (bid == MBCMD_BID_NULL)
			return;
		mbsend(mbcmd(MBCMD_BKYLD, 0), bid);
		ipbuf_bsycnt++;
	}
}

Uns sync_ipbuf(Uns tid, Uns bid)
{
	if (bid >= _ipbuf_lines)
		return MBCMD_EID_BADBID;
	if ((tid >= N_TASK_MAX) || (task_prop[tid] == NULL))
		return MBCMD_EID_BADTID;
	sync_with_arm(&ipbuf[bid]->sa, tid);

	return 0;
}

Void lock_ipbuf_sys_da(Void)
{
	SEM_pend(&SEM_ipbuf_sys_da, SYS_FOREVER);
	sync_with_arm(&ipbuf_sys_da.s, MBCMD_TID_FREE);
}

Void unlock_ipbuf_sys_da(Void)
{
	SEM_post(&SEM_ipbuf_sys_da);
}

Void init_icache(Void)
{
	/*
	 * configuring 2-way set-associative cache (8KB each)
	 */
	outw(_ICACHE_GCR_CUT_CLOCK |
	     _ICACHE_GCR_AUTO_GATING |
	     _ICACHE_GCR_GLOBAL_FLUSH |
	     _ICACHE_GCR_WAY_PRESENCE |
	     _ICACHE_GCR_WAY_NUMBER_2 |
	     _ICACHE_GCR_STREAMING |
	     _ICACHE_GCR_GLOBAL_ENABLE, _ICACHE_GCR);
	outw(_ICACHE_NWCR_WAY_SIZE_8K |
	     _ICACHE_NWCR_ENABLE, _ICACHE_NWCR);

	/*
	 * cache clear and wait for the process done
	 */
	asm(" BSET CACLR");
	do {} while (*_ST3_55 & _ST3_55_CACLR);

	/*
	 * cache enable
	 */
	asm(" BSET CAEN");
}

Uns icache_disable(Void)
{
	Uns ret = *_ST3_55 & _ST3_55_CAEN;

	if (ret) {
		asm(" BCLR CAEN");
		do {} while (inw(_ICACHE_ISR) & _ICACHE_ISR_ENABLE);
	}
	return ret;
}

Void icache_restore(Uns saved)
{
	if (saved) {
		asm(" BSET CAEN");
		do {} while (!(inw(_ICACHE_ISR) & _ICACHE_ISR_ENABLE));
	}
}

static Uns init_mailbox(Void)
{
	Uns dummy;

	dummy = inw(_ARM2DSP1b);	/* clear interrupt source */
	enable_irq(INT_MAILBOX1);

	/*
	 * return value will not be used at all.
	 * just to make compiler silent.
	 */
	return dummy;
}

Void reset_mailbox(Void)
{
	mbseq.da_dsp = mbseq.da_arm = 0;
	/* DSP can decide initialization value for A2D arbitrary. */
	mbseq.ad_dsp = mbseq.ad_arm = 0;

	mem_sync.DARAM->da_dsp = mem_sync.DARAM->da_arm = 0;
	mem_sync.DARAM->ad_dsp = mem_sync.DARAM->ad_arm = 0;
	mem_sync.SARAM->da_dsp = mem_sync.SARAM->da_arm = 0;
	mem_sync.SARAM->ad_dsp = mem_sync.SARAM->ad_arm = 0;
	mem_sync.SDRAM->da_dsp = mem_sync.SDRAM->da_arm = 0;
	mem_sync.SDRAM->ad_dsp = mem_sync.SDRAM->ad_arm = 0;
}

Void sync_mem_all(Void)
{
	mem_sync.DARAM->ad_dsp++;
	mem_sync.SARAM->ad_dsp++;
	mem_sync.SDRAM->ad_dsp++;
	sync_with_arm(&mem_sync.DARAM->ad_arm, mem_sync.DARAM->ad_dsp);
	sync_with_arm(&mem_sync.SARAM->ad_arm, mem_sync.SARAM->ad_dsp);
	sync_with_arm(&mem_sync.SDRAM->ad_arm, mem_sync.SDRAM->ad_dsp);
}

Void init_ipbuf(Void)
{
	Uns i;
	Void **ptrs;
	Uns ipbuf_lines_half;

	INIT_IPBLINK(&ipb_free);

	if (_ipbuf_lines == 0)
		return;

#if 0
	ipbuf   = MEM_alloc(MEM->MALLOCSEG, sizeof(Void *) * _ipbuf_lines, 1);
	ipbuf_d = MEM_alloc(MEM->MALLOCSEG, sizeof(Void *) * _ipbuf_lines, 1);
	if ((ipbuf == MEM_ILLEGAL) || (ipbuf_d == MEM_ILLEGAL)) {
		sys_cmderr(MBCMD_EID_NOMEM, MBCMD_TID_ANON);
		return;
	}
#endif

	/* reducing MEM_alloc */
	ptrs = MEM_alloc(MEM->MALLOCSEG, sizeof(Void *) * _ipbuf_lines * 2, 1);
	if (ptrs == MEM_ILLEGAL) {
		sys_cmderr(MBCMD_EID_NOMEM, MBCMD_TID_ANON);
		return;
	}
	ipbuf   = (Void *)&ptrs[0];
	ipbuf_d = (Void *)&ptrs[_ipbuf_lines];

	for (i = 0; i < _ipbuf_lines; i++) {
		ipbuf[i] = (struct ipbuf *)((LgUns)_ipbuf_body +
					    (IPBUF_HEADER_SZ +
					     (LgUns)_ipbuf_sz) * i);
		ipbuf_d[i] = ipbuf[i]->d;
	}

	/* DSP holds all ipbufs initially. */
	ipbuf_bsycnt = 0;
	ipbuf_lines_half = _ipbuf_lines / 2;
	for (i = 0; i < ipbuf_lines_half; i++) {
		/* hold for us */
		ipbuf[i]->la = MBCMD_TID_ANON;
		ipbuf[i]->sa = MBCMD_TID_ANON;
		ipbuf[i]->ld = MBCMD_TID_FREE;
		ipbuf[i]->sd = MBCMD_TID_FREE;
		ipbuf[i]->next = MBCMD_BID_NULL;
		ipblink_add_tail(&ipb_free, i, ipbuf);
	}
	for (; i < _ipbuf_lines; i++) {
		/* yield to MPU */
		ipbuf[i]->la = MBCMD_TID_ANON;
		ipbuf[i]->sa = MBCMD_TID_ANON;
		ipbuf[i]->ld = MBCMD_TID_ANON;
		ipbuf[i]->sd = MBCMD_TID_ANON;
		ipbuf[i]->next = MBCMD_BID_NULL;
		mbsend(mbcmd(MBCMD_BKYLD, 0), i);
		ipbuf_bsycnt++;
	}
}

/*
 * task I/F functions
 */
Uns get_free_ipbuf(struct dsptask *task)
{
	Uns bid;
	Uns intm_saved;

	if (ipblink_empty(&ipb_free)) {
		sys_cmderr(MBCMD_EID_IPBFULL, MBCMD_TID_ANON);
		/*
		 * FIXME: wait on queue
		 */
		do {
			busywait(10);
		} while (ipblink_empty(&ipb_free));
	}

	intm_saved = HWI_disable();
	bid = ipb_free.top;
	ipbuf[bid]->ld = task->tid;	/* lock */
	ipblink_del_top(&ipb_free, ipbuf);
	HWI_restore(intm_saved);

	return bid;
}

Void unuse_ipbuf(struct dsptask *task, Uns bid)
{
	Uns eid;

	if ((eid = release_ipbuf(bid)) != 0)
		sys_cmderr(eid, task->tid);
}

Void wdsnd(struct dsptask *task, Uns data)
{
	mbsend(mbcmd(MBCMD_WDSND, task->tid), data);
}

Void bksnd(struct dsptask *task, Uns bid, Uns cnt)
{
	Uns tid = task->tid;

	ipbuf[bid]->c = cnt;
	ipbuf[bid]->sd = tid;
	mbsend(mbcmd(MBCMD_BKSND, tid), bid);
	ipbuf_bsycnt++;
}

Void bksndp(struct dsptask *task, Void *data, Uns cnt)
{
	Uns tid = task->tid;
	LgUns adr = (LgUns)data;
	struct ipbuf_p *ipbuf_p = &task_prop[tid]->ipbuf_p_snd;

	sync_with_arm(&ipbuf_p->s, MBCMD_TID_FREE);
	ipbuf_p->c = cnt;
	ipbuf_p->s = tid;
	ipbuf_p->ah = adr >> 16;
	ipbuf_p->al = adr & 0xffff;
	mbsend(mbcmd(MBCMD_BKSNDP, tid), 0);
}

Void wdreq(struct dsptask *task)
{
	mbsend(mbcmd(MBCMD_WDREQ, task->tid), 0);
}

Void bkreq(struct dsptask *task, Uns cnt)
{
	mbsend(mbcmd(MBCMD_BKREQ, task->tid), cnt);
}

Void bkreqp(struct dsptask *task, Void *data, Uns cnt)
{
	Uns tid = task->tid;
	LgUns adr = (LgUns)data;
	struct ipbuf_p *ipbuf_p = &task_prop[tid]->ipbuf_p_req;

	ipbuf_p->c = cnt;
	ipbuf_p->s = MBCMD_TID_FREE;
	ipbuf_p->ah = adr >> 16;
	ipbuf_p->al = adr & 0xffff;
	mbsend(mbcmd(MBCMD_BKREQP, tid), 0);
}

Void cmderr(struct dsptask *task, Uns eid)
{
	sys_cmderr(eid, task->tid);
}

static Void kfunc(Uns fid, Uns data, Uns argc, Void *argv)
{
	if (argc > 0) {
		lock_ipbuf_sys_da();
		memcpy(ipbuf_sys_da.d, argv, argc);
		ipbuf_sys_da.s = MBCMD_TID_ANON;
	}
	mbsend(mbcmd(MBCMD_KFUNC, fid), data);
	if (argc > 0)
		unlock_ipbuf_sys_da();
}

#pragma DATA_SECTION(dbgbuf, "dbgbuf")
Char dbgbuf[DBG_BUF_SZ];
static Int dbg_enable = 0;

Void init_dbg(Void)
{
	dbg_enable = 1;
}

Void dbg(struct dsptask *task, Char *fmt, ...)
{
	static Uns dbg_wp = 0;
	Uns intm_saved;
	va_list list;
	int cnt;
	Char *p;

	if (!dbg_enable)
		return;

	intm_saved = HWI_disable();
	va_start(list, fmt);
	p = &dbgbuf[dbg_wp];
#ifndef DEBUG_TSK
	/*
	 * ???
	 * Basically this should be used, but when we enabled DEBUG_TSK switch,
	 * it does not work.
	 */
	vsnprintf(p, DBG_LINE_SZ, fmt, list);
#else
	SYS_vsprintf(p, fmt, list);
#endif
	cnt = strlen(p);
	if ((dbg_wp += cnt + 1) > DBG_BUF_SZ - DBG_LINE_SZ)
		dbg_wp = 0;
	va_end(list);

	mbsend(mbcmd(MBCMD_DBG, task->tid), cnt);
	HWI_restore(intm_saved);
}

/*
 * framebuffer lock / unlock
 * fb_lock_timeout(): user task calls before modifying FB.
 * fb_unlock(): user task calls after modified FB.
 * fb_enable(): called by supertask when ARM allows to modify the framebuffer
 * fb_disable(): called by supertask when ARM prohibits to modify the framebuffer
 */
Bool fb_lock_timeout(Uns timeout)
{
	return SEM_pend(&SEM_fb, timeout);
}

Void fb_unlock(Void)
{
	SEM_post(&SEM_fb);
}

static Uns fb_is_enabled = 1;

Void fb_enable(Void)
{
	if (fb_is_enabled) {
		dbg(&task_anon,
		    "[dsp] FBCTL_ENABLE received,"
		    " but fb is already enabled.\n");
		return;
	}
	fb_is_enabled = 1;
	SEM_post(&SEM_fb);
}

Void fb_disable(Void)
{
	if (!fb_is_enabled) {
		dbg(&task_anon,
		    "[dsp] FBCTL_DISABLE received,"
		    " but fb is already disabled.\n");
		return;
	}
	fb_lock();	/* may block! */
	fb_is_enabled = 0;
	kfunc(MBCMD_KFUNC_FBCTL, MBCMD_FBCTL_DISABLE, 0, NULL);
}

void uart_printf ( Char *fmt, ...)
{
  va_list args;
  char printbuffer[1024];

  va_start (args, fmt);
  vsnprintf (printbuffer, 256, fmt, args);
  va_end (args);
  Uart3_Send(printbuffer);
}

/*
 * main
 */
Void main()
{
	init_icache();
	init_wdt();
	init_timer();
	init_clock();
	init_mailbox();
	init_icr();
	mb2send(0xffff,0xffff);
	if (init_tasks())
		return;
	init_status = 1;
}

/*
 * trace
 */
Void tok_TSK_switch(TSK_Handle currTask, TSK_Handle nextTask)
{
#ifdef DEBUG_TSK
	if (currTask != nextTask) {
		dbg(&task_anon,
		    "[DSP] %08lx tok_TSK_switch: curr = %s, next = %s\n",
		    jiffies, currTask->name, nextTask->name);
	}
#endif	/* DEBUG_TSK */
}

Void tok_TSK_ready(Void)
{
}

/*
 * interrupt handler routine
 *
 * INTM is set automatically when an interrupt occurs,
 * and will be restored at iret.
 *
 * Do not use 'interrupt' modifier in conjunction with DSP/BIOS.
 */

static Uns tctl_parse(Uns tid, Uns ctlcmd)
{
	if (((ctlcmd >= 0x8100) && (ctlcmd < 0x8200)) ||
	    ((ctlcmd >= 0x9100) && (ctlcmd < 0x9200))) {
		/* This TCTL has 1 arg */
		Uns eid;
		sync_with_arm(&ipbuf_sys_ad.s, tid);
		eid = register_mbq(tid, MBCMD_TCTLDATA, ipbuf_sys_ad.d[0]);
		ipbuf_sys_ad.s = MBCMD_TID_FREE;
		if (eid)
			return eid;
	}
	return 0;
}

Uns bset_uart3 = 0;

Void mailbox_interrupt(Void)
{
	Uns cmd, data, cmd_h, cmd_l;
	Uns seq;
	Uns eid;

	Uns i;

	data = inw(_ARM2DSP1);
	cmd  = inw(_ARM2DSP1b);
	
	cmd_h = (cmd & 0x7f00) >> 8;
	cmd_l = cmd & 0x00ff;
	seq   = cmd >> 15;
	if ((seq != (mbseq.ad_dsp & 1)) && (cmd_h != MBCMD_DSPCFG)) {
		switch (mbseq_chklevel) {
		case 0:	/* no check */
			break;
		case 1:	/* notify */
			eid = MBCMD_EID_BADSEQ;
			goto error;
		case 2:	/* discard */
			return;
		}
	}

	switch (cmd_h) {
	/*
	 * commands handled immediately
	 */
	case MBCMD_RUNLEVEL:
		if (cmd_l == MBCMD_RUNLEVEL_RECOVERY)
			super_recover_int();
		register_super_mbq(cmd_h, cmd_l, data);
		break;

	/*
	 * commands handled by supertask
	 */
	case MBCMD_BKYLD:
		ipbuf_bsycnt--;
		register_super_mbq(cmd_h, cmd_l, data);
		break;
	case MBCMD_PM:
		break;
	case MBCMD_SUSPEND:
	case MBCMD_KFUNC:
	case MBCMD_TADD:
	case MBCMD_DSPCFG:
	case MBCMD_POLL:
	case MBCMD_REGRW:
	case MBCMD_GETVAR:
	case MBCMD_SETVAR:
		register_super_mbq(cmd_h, cmd_l, data);
		break;

	/*
	 * commands handled by each task
	 */
	case MBCMD_BKSND:
		if ((eid = sync_ipbuf(cmd_l, data)) != 0)
			goto error;
		ipbuf_bsycnt--;
		balance_ipbuf();
		/* this task owns this line from here */
		ipbuf[data]->ld = cmd_l;
		if ((eid = register_mbq(cmd_l, cmd_h, data)) != 0) {
			ipbuf[data]->ld = MBCMD_TID_ANON;
			unuse_ipbuf(&task_anon, data);
			goto error;
		}
		break;

	case MBCMD_TCTL:
		if ((eid = register_mbq(cmd_l, cmd_h, data)) != 0)
			goto error;
		if ((eid = tctl_parse(cmd_l, data)) != 0)
			goto error;
		break;

	case MBCMD_WDSND:
	case MBCMD_WDREQ:
	case MBCMD_BKREQ:
	case MBCMD_BKSNDP:
	case MBCMD_BKREQP:
	case MBCMD_TCFG:
		if ((eid = register_mbq(cmd_l, cmd_h, data)) != 0)
			goto error;
		break;
	/*
	 * TDEL will be fowarded to supertask,
	 * after commands in mbq were completed.
	 */
	case MBCMD_TDEL:
		if (data == MBCMD_TDEL_KILL) {
			/*
			 * TCTL:KILL will be performed in
			 * supertask context.
			 */
			register_super_mbq(cmd_h, cmd_l, data);
		} else {
			/* MBCMD_TDEL_SAFE */
			/* TCTL:TCLR will be performed in user task context. */
			if ((eid = register_mbq(cmd_l,
						MBCMD_TCTL,
						MBCMD_TCTL_TCLR)) != 0)
				goto error;
			if ((eid = register_mbq(cmd_l, cmd_h, data)) != 0)
				goto error;
		}
		break;
	case MBCMD_UARTSET:
		{
			if( data == 0x3)
		      {
				do {i = inw(_RHSW_DSP_CNF3);}
				while( i & 0x1);
				
				outw(0x2, _RHSW_DSP_CNF3);
				outw(0x2, _RHSW_DSP_CNF3);
				outw(0x2, _RHSW_DSP_CNF3);
				
				if( inw(_RHSW_DSP_CNF3) & 0x2)
				  outw(0x1111, _DSP2ARM1);// no error
				else
				  outw(0x1, _DSP2ARM1); // not set
				bset_uart3 = 1;
		      }
		    	else
				outw(0x1, _DSP2ARM1);// error
		    	outw(0x5255, _DSP2ARM1b);// reponse uart
			while( _DSP2ARM1_Flag & 0x1);
			if( bset_uart3 == 1){		
				DSP_IDLECT2 |= (EN_TIMCK_DSP | EN_GPIOCK_DSP | EN_XORPCK_DSP);	//enable dsp per clk
				DSP_RSTCT2  |= PER_EN_DSP;	//peripherals release from reset
				Uart3_Setup();
				uart_printf("UPTECH OMAP:\r\n");
			}
			else
				bset_uart3 = 0;
		    	break;    
		}	
	case MBCMD_FBTSET:
	        {
		        long i;
#define FB_START 0x300800			
			for(i = FB_START; i < FB_START + 0x4B000 ; i++)
			        *((unsigned short*) i)=0x1234;
			uart_printf("Finished\r\n");
                        outw(0xeeee, _DSP2ARM2);// reponse uart
			outw(0xeeee, _DSP2ARM2b);// reponse uart
 			
		        break;
		}

	/*
	 * illegal commands
	 */
	case MBCMD_ERR:
	default:
		eid = MBCMD_EID_BADCMD;
		goto error;
	}

	mbseq.ad_dsp++;
	return;

error:
	mbseq.ad_dsp++;
	/* error! */
	uart_printf("Mailbox1 ERROR !!!\n");
	sys_cmderr(eid, cmd);
}
