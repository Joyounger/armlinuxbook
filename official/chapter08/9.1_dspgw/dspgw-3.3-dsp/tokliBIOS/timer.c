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
 * 2005/03/23:  DSP Gateway version 3.3
 */

#include <std.h>
#include <sys.h>
#include <hwi.h>
#include <tsk.h>
#include <mem.h>
#include "omap1510.h"
#include "tokliBIOSlib.h"
#include "timer.h"

/* 10 ms, with 12MHz base clock -- 12000000L/2/100-1 */
#define TIMER1_LOAD_VAL	59999L

TAILQ_HEAD(qh, procq) tq_1s = TAILQ_HEAD_INITIALIZER(tq_1s);

struct idle_tim_hist {
	Int wp;
	LgUns d[100];
};
static struct idle_tim_hist idle_tim_hist;
static LgUns tmp_idle_tim = 0;

struct load_info_hist {
	Int wp;
	Uns d[60];
};
static struct load_info_hist load_info_hist_1m;

Void set_load_state(Int state)
{
	static LgUns idle_start = 0;
	LgUns tim1 = inl(_READ_TIM1);

	switch (state) {
	case LOAD_STATE_IDLE:
		idle_start = tim1;
		break;
	case LOAD_STATE_BUSY:
		tmp_idle_tim += (idle_start > tim1) ?
					idle_start - tim1 :
					idle_start + TIMER1_LOAD_VAL - tim1;
		break;
	}
}

static Void init_load_info(Void)
{
	memset(idle_tim_hist.d, 0, sizeof(idle_tim_hist.d));
	idle_tim_hist.wp = 0;
	tmp_idle_tim = 0;
	memset(load_info_hist_1m.d, 0, sizeof(load_info_hist_1m.d));
	load_info_hist_1m.wp = 0;
}

static Void inc_idle_tim_hist_wp(Void)
{
	idle_tim_hist.d[idle_tim_hist.wp] = tmp_idle_tim;
	if(++idle_tim_hist.wp == 100)
		idle_tim_hist.wp = 0;
	tmp_idle_tim = 0;
}

/*
 * range of load_info is 0 - 10000
 */
static Uns get_load_info_10ms(Void)
{
	Uns wp_prev;

	wp_prev = (idle_tim_hist.wp == 0) ? 99 : idle_tim_hist.wp - 1;
	return 10000 - idle_tim_hist.d[wp_prev] * 10000 / TIMER1_LOAD_VAL;
}

static Void get_load_info_1s(Uns *a, Uns *h)
{
	LgUns sum = 0;
	LgUns low = TIMER1_LOAD_VAL;
	LgUns tmp;
	Int i;

	for (i = 0; i < 100; i++) {
		tmp = idle_tim_hist.d[i];
		sum += tmp;
		if (tmp < low)
			low = tmp;
	}
	*a = 10000 - sum * 100 / TIMER1_LOAD_VAL;
	*h = 10000 - low * 10000 / TIMER1_LOAD_VAL;
}

static Void get_load_info_1m(Uns *a, Uns *h)
{
	LgUns sum = 0;
	Uns high = 0;
	Uns tmp;
	Int i;

	for (i = 0; i < 60; i++) {
		tmp = load_info_hist_1m.d[i];
		sum += tmp;
		if (tmp > high)
			high = tmp;
	}
	*a = sum / 60;
	*h = high;
}

static Void inc_load_info_hist_wp(struct load_info_hist *h)
{
	Uns ave, high;

	get_load_info_1s(&ave, &high);
	h->d[h->wp] = ave;
	if(++h->wp == 60)
		h->wp = 0;
}

Void get_load_info(struct load_info *linfo)
{
	linfo->a_10ms = get_load_info_10ms();
	get_load_info_1s(&linfo->a_1s, &linfo->h_1s);
	get_load_info_1m(&linfo->a_1m, &linfo->h_1m);
}

/*
 * Timer1 is configured in DSP/BIOS configuration,
 * but we want to set it not to be changed by DSP domain clock
 * frequency.
 * Therefore, we need set it up again here.
 */
Void init_timer(Void)
{
	timer1_stop();
	/* 12MHz for timer clock */
	outw(inw(_DSP_CKCTL) & ~_DSP_CKCTL_TIMXO, _DSP_CKCTL);
	/* Autoload mode, PTV=0 */
	outl(TIMER1_LOAD_VAL, _LOAD_TIM1);
	outw(_CNTL_TIMERn_AR |
	     _CNTL_TIMERn_PTV(0) |
	     _CNTL_TIMERn_CLOCK_ENABLE, _CNTL_TIMER1);
	init_load_info();
	timer1_start();
}

Void *register_tq_1s(struct dsptask *task, Uns (*f)(struct dsptask *task))
{
	struct procq *new;
	Uns intm_saved;

	new = MEM_alloc(MEM->MALLOCSEG, sizeof(struct procq), 2);
	if (new == MEM_ILLEGAL)
		return MEM_ILLEGAL;
	new->func = f;
	new->tid = task->tid;
	intm_saved = HWI_disable();
	TAILQ_INSERT_TAIL(&tq_1s, new, qh);
	HWI_restore(intm_saved);
	return new;
}

/*
 * unregister_tq_1s():
 * if id == NULL, unregister all functions of the task registered the queue.
 */
Void unregister_tq_1s(struct dsptask *task, Void *id)
{

	struct procq *p, *tmp;
	Uns intm_saved;
	Uns tid = task->tid;

	intm_saved = HWI_disable();
	TAILQ_FOREACH_SAFE(p, tmp, &tq_1s, qh) {
		if ((p == id) || ((id == NULL) && (tid == p->tid))) {
			TAILQ_REMOVE(&tq_1s, p, qh);
			MEM_free(MEM->MALLOCSEG, p, sizeof(struct procq));
			break;
		}
	}
	HWI_restore(intm_saved);
}

LgUns jiffies = 0;

/*
 * PRD functions
 */
Void prd_10ms(Void)
{
	/*
	 * Power Management:
	 * if ARM seems to have received the previous mailbox command,
	 * power off the DMA Domain.
	 */
	if (mb_active) {
		if (mbseq.da_dsp == mbseq.da_arm) {
			disable_domain(_ICR_DMA_IDLE_DOMAIN);
			mb_active = 0;
		}
	}

	inc_idle_tim_hist_wp();
	TSK_yield();
	jiffies++;
}

Void prd_1s(Void)
{
	struct procq *p, *tmp;
	Uns eid;
#ifdef DEBUG_WAKEUP_CNT
	LgUns wakeup_cnt;
#endif

	/*
	 * each func can commit a suicide.
	 * (i.e. call unregister() itself in func().)
	 */
	TAILQ_FOREACH_SAFE(p, tmp, &tq_1s, qh) {
		Uns tid = p->tid;

		eid = p->func(task_prop[tid]->dsptask);
		/*
		 * after func(), we shold not to access p or pq
		 * because it can be deleted!
		 */
		if (eid)
			sys_cmderr(eid, mbcmd(0, tid));
	}

	inc_load_info_hist_wp(&load_info_hist_1m);

#ifdef DEBUG_WAKEUP_CNT
	if ((wakeup_cnt = get_wakeup_cnt()) > 0) {
		dbg(&task_anon, "wakeup_cnt = %ld\n", wakeup_cnt);
		clear_wakeup_cnt();
	}
#endif
}
