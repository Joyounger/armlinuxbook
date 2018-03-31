/*
 * Copyright (c) 2002-2004, Nokia
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
 * 2005/05/20:  DSP Gateway version 3.3
 */

#include <std.h>
#include "omap1510.h"
#include "mailbox.h"
#include "tokliBIOSlib.h"
#include "timer.h"

/*
 * WDT is used in timer mode, and two-level state machine is defined for WDT.
 * WDT_L1 state:
 *   If WDT expires in this state, the DSP kernel puts a POLL command to
 *   all dsptask mbq. Then the state machine moves to WDT_L2 state.
 *   Each task is responsible to respond to the POLL command within next
 *   WDT duration.
 * WDT_L2 state:
 *   If all the dsptask responded to the POLL command, the state machine
 *   get back to the WDT_L1 state.
 *   Otherwise, if WDT expires again, the DSP kernel sends a WDT expiration
 *   error command to ARM.
 */
enum wdt_st {
	WDT_L1,
	WDT_L2
} wdt_st;

Void init_wdt(Void)
{
	__wdt_ck_enable();

	wdt_st = WDT_L1;
	/*
	 * Start WDT with autoreload mode
	 * if we set 0xffff to _LOAD_TIM, the WDT expires in 19.57s.
	 * here we set 0x7fff, for two-level WDT.
	 */
	outw(0x7fff, _LOAD_TIM);
	outw((inw(_CNTL_TIMER) & ~_CNTL_TIMER_PTV_MASK) |
	     _CNTL_TIMER_AR |
	     _CNTL_TIMER_PTV(7) |
	     _CNTL_TIMER_ST,
	     _CNTL_TIMER);

	enable_irq(INT_WDGTIMER);
}

Void wdt_start(Void)
{
	__wdt_start();
}

Void wdt_stop(Void)
{
	__wdt_stop();
	wdt_st = WDT_L1;
}

Void wdt_reflesh(Void)
{
	Uns intm_saved;

	intm_saved = HWI_disable();
	__wdt_stop();
	wdt_st = WDT_L1;
	__wdt_start();
	HWI_restore(intm_saved);
}

static Uns poll_flag[N_TASK_MAX];
static Uns poll_pending_cnt_wdt;
static Uns poll_pending_cnt_arm;

Int poll_broadcast(Uns flag)
{
	Uns tid;
	struct task_prop *prop;
	Uns registered_cnt;
	Uns intm_saved;

	intm_saved = HWI_disable();
	registered_cnt = 0;
	for (tid = n_task; tid < N_TASK_MAX; tid++) {
		prop = task_prop[tid];
		if ((prop != NULL) && (!prop->poll_exclude)) {
			poll_flag[tid] |= flag;
			registered_cnt++;
			register_mbq(tid, MBCMD_POLL, flag);
		} else {
			poll_flag[tid] &= ~flag;
		}
	}
	if (flag & MBCMD_POLL_WDT)
		poll_pending_cnt_wdt = registered_cnt;
	if (flag & MBCMD_POLL_ARM)
		poll_pending_cnt_arm = registered_cnt;
	HWI_restore(intm_saved);

	return registered_cnt;
}

Void poll_clear(Uns tid, Uns flag)
{
	if (poll_flag[tid] & flag & MBCMD_POLL_WDT) {
		poll_flag[tid] &= ~MBCMD_POLL_WDT;
		poll_pending_cnt_wdt--;
	}
	if (poll_flag[tid] & flag & MBCMD_POLL_ARM) {
		poll_flag[tid] &= ~MBCMD_POLL_ARM;
		if (--poll_pending_cnt_arm == 0)
			mbsend(mbcmd(MBCMD_POLL, 0), 0);
	}
}

/*
 * task should call poll_disable() manually when one mbx command takes
 * more than POLL timeout (i.e. WDT dulation or ARM expectation for POLL
 * command response), or call poll_exclude() at initialization.
 * In poll_exclude() case, this task won't be polled at all.
 */
Void poll_disable(struct dsptask *task)
{
	Uns tid = task->tid;
	struct task_prop *prop = task_prop[tid];

	/* POLL_EXCLUDE_TEMPORARY will be cleared at every mbq execution */
	prop->poll_exclude = POLL_EXCLUDE_TEMPORARY;
	poll_clear(task->tid, 0xffff);
}

Void poll_exclude(struct dsptask *task)
{
	Uns tid = task->tid;
	struct task_prop *prop = task_prop[tid];

	prop->poll_exclude = POLL_EXCLUDE_ETERNAL;
}

static Void poll_pending_info(Void)
{
	Uns tid;
	struct dsptask *task;

	for (tid = n_task; tid < N_TASK_MAX; tid++) {
		if (poll_flag[tid]) {
			task = task_prop[tid]->dsptask;
			dbg(&task_anon, "[DSP]: task %s not responding\n",
			    task->name);
		}
	}
}

/*
 * interrupt routine
 */
Void wdt_handle(Void)
{
	if ((wdt_st == WDT_L2) && poll_pending_cnt_wdt) {
		poll_pending_info();
		mbsend(mbcmd(MBCMD_ERR, MBCMD_EID_WDT), 0x0a21);	/* "!\n" */
	} else {
		wdt_st = WDT_L2;
		poll_broadcast(MBCMD_POLL_WDT);
	}
}
