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

#ifndef __TOKLIBIOSLIB_H
#define __TOKLIBIOSLIB_H

#define MBSEQ_CHKLEVEL 1

#include <std.h>
#include <sem.h>
#include <tsk.h>
#include "omap1510.h"
#include "mailbox.h"
#include "tokliBIOS.h"
#include "queue.h"

/*
 * stuff defined by DSP/BIOS tool
 */
extern Int DARAM, SARAM;


enum mem_type_e {
	MEM_TYPE_CROSSING = -1,
	MEM_TYPE_NONE = 0,
	MEM_TYPE_DARAM,
	MEM_TYPE_SARAM,
	MEM_TYPE_EXTERN
};

enum ptr_type_e {
	PTR_CODE,
	PTR_DATA
};

#define MKLONG(uw,lw)	(((LgUns)(uw))<<16 | (lw))

/*
 * D2A_MB value definition
 *   1: use mailbox 1 for DSP->ARM mailbox
 *   2: use mailbox 2 for DSP->ARM mailbox
 */
#define D2A_MB	1

#if (D2A_MB == 1)
#	define _DSP2ARM _DSP2ARM1
#	define _DSP2ARMb _DSP2ARM1b
#	define _DSP2ARM_Flag _DSP2ARM1_Flag
#elif (D2A_MB == 2)
#	define _DSP2ARM _DSP2ARM2
#	define _DSP2ARMb _DSP2ARM2b
#	define _DSP2ARM_Flag _DSP2ARM2_Flag
#endif

struct ipbuf {
	Uns c;		/* count */
	Uns next;	/* link */
	Uns la;		/* lock owner (ARM side) */
	Uns sa;		/* sync word (ARM->DSP) */
	Uns ld;		/* lock owner (DSP side) */
	Uns sd;		/* sync word (DSP->ARM) */
	Uns d[1];	/* data */
};

struct ipbuf_p {
	Uns c;		/* count */
	Uns s;		/* sync word */
	Uns al;		/* data address lower */
	Uns ah;		/* data address upper */
};

struct ipbuf_sys {
	Uns s;		/* sync word */
	Uns d[31];	/* data */
};

struct sync_seq {
	Uns da_dsp;
	Uns da_arm;
	Uns ad_dsp;
	Uns ad_arm;
};

struct mem_sync_struct {
	struct sync_seq *DARAM;
	struct sync_seq *SARAM;
	struct sync_seq *SDRAM;
};

struct mailboxq {
	Uns cmd_h;
	Uns data;
};

struct procq {
	TAILQ_ENTRY(procq) qh;
	Uns (*func)(struct dsptask *task);
	Uns tid;
};

struct load_info {
	Uns a_10ms;
	Uns a_1s, h_1s;
	Uns a_1m, h_1m;
};

#define MBQ_MAX	8

/*
 * task private properties
 */
#define TASK_STAT_RUNNING	1
#define TASK_STAT_STOP		2

#define POLL_EXCLUDE_TEMPORARY	0x0001
#define POLL_EXCLUDE_ETERNAL	0x0002

struct task_prop {
	struct dsptask *dsptask;
	struct ipbuf_p ipbuf_p_snd, ipbuf_p_req;
	TSK_Handle tsk;
	SEM_Handle sem;
	Int stat;
	Uns mbq_wp, mbq_rp;
	struct mailboxq mbq[MBQ_MAX];
	Uns poll_exclude;
};

#define N_TASK_MAX	32	/* temporary */
extern Uns n_task;

extern struct task_prop *task_prop[];
extern struct dsptask task_anon;
#define DBG_BUF_SZ	2048
#define DBG_LINE_SZ	256
extern Char dbgbuf[];

extern struct ipbuf **ipbuf;

extern Uns _ipbuf_sz;
extern Uns _ipbuf_lines;
extern Int _ipbuf_body[];

extern struct sync_seq mbseq;
extern Uns mb_active;
extern struct mem_sync_struct mem_sync;
extern struct ipbuf_sys ipbuf_sys_ad, ipbuf_sys_da;
extern LgUns jiffies;

struct ipblink {
	Uns top;
	Uns tail;
};

#define INIT_IPBLINK(link) \
	do { \
		(link)->top  = MBCMD_BID_NULL; \
		(link)->tail = MBCMD_BID_NULL; \
	} while(0)

#define ipblink_empty(link)	((link)->top == MBCMD_BID_NULL)

static inline Void ipblink_del_top(struct ipblink *link, struct ipbuf **ipbuf)
{
	struct ipbuf *bufp = ipbuf[link->top];

	if ((link->top = bufp->next) == MBCMD_BID_NULL)
		link->tail = MBCMD_BID_NULL;
	else
		bufp->next = MBCMD_BID_NULL;
}

static inline Void ipblink_add_tail(struct ipblink *link, Uns bid,
				    struct ipbuf **ipbuf)
{
	if (ipblink_empty(link))
		link->top = bid;
	else
		ipbuf[link->tail]->next = bid;
	link->tail = bid;
}


#define mbcmd(cmd,tid)	((Uns)cmd << 8 | tid)
#define sys_cmderr(eid,dat)	mbsend(mbcmd(MBCMD_ERR,eid),dat)

static inline Void busywait(Uns cnt)
{
	Uns i;
	for (i = 0; i < cnt; i++) {}
}

static inline Void sync_with_arm(Uns *syncword, Uns val)
{
	while (*(volatile Uns *)syncword != val) {
		busywait(10);
	}
}

static inline Void enable_irq(Uns irq)
{
	if (irq < 16)
		*_IER0 |= (1 << irq);
	else
		*_IER1 |= (1 << (irq - 16));
}

static inline Void disable_irq(Uns irq)
{
	if (irq < 16)
		*_IER0 &= ~(1 << irq);
	else
		*_IER1 &= ~(1 << (irq - 16));
}

extern Int init_done(Void);
extern enum mem_type_e mem_type(Void *adr, Uns len, enum ptr_type_e type);
extern Void mbsend(Uns cmd, Uns data);
extern Uns release_ipbuf(Uns bid);
extern Void balance_ipbuf(Void);
extern Uns sync_ipbuf(Uns tid, Uns bid);
extern Void lock_ipbuf_sys_da(Void);
extern Void unlock_ipbuf_sys_da(Void);
extern Void init_icache(Void);
extern Uns icache_disable(Void);
extern Void icache_restore(Uns saved);
extern Void reset_mailbox(Void);
extern Void init_ipbuf(Void);
extern Void init_dbg(Void);
extern Void fb_enable(Void);
extern Void fb_disable(Void);

extern Uns task_config(struct dsptask *task, struct task_prop *prop, Uns tid);
extern Void task_unconfig(Uns tid);
extern Int register_mbq(Uns tid, Uns cmd_h, Uns data);

extern Uns init_tasks(Void);
extern Void init_super(Void);
extern Void supertask(Void);
extern Void super_recover_int(Void);
extern Void register_super_mbq(Uns cmd_h, Uns cmd_l, Uns data);

extern Void init_icr(Void);
extern Uns get_icr_mask(Void);
extern Void set_icr_mask(Uns val);
#ifdef DEBUG_WAKEUP_CNT
extern LgUns get_wakeup_cnt(Void);
extern Void clear_wakeup_cnt(Void);
#endif
extern Void init_clock(Void);

extern Void init_wdt(Void);
extern Int poll_broadcast(Uns flag);
extern Void poll_clear(Uns tid, Uns flag);

#define LOAD_STATE_IDLE 1
#define LOAD_STATE_BUSY 2

extern Void set_load_state(Int state);
extern Void get_load_info(struct load_info *linfo);
extern Void init_timer(Void);
extern Void *register_tq_1s(struct dsptask *task,
			    Uns (*f)(struct dsptask *task));
extern Void unregister_tq_1s(struct dsptask *task, Void *id);

#endif /* __TOKLIBIOSLIB_H */
