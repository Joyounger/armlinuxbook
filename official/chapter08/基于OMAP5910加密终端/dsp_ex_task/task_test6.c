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
 * 2004/07/15:  DSP Gateway version 3.3
 */

#include <std.h>
#include <mem.h>
#include "mailbox.h"
#include "tokliBIOS.h"

struct t6_udata {
	Void *t1q_id;
	Int waiting;
	Int req_cnt;
};

static Uns t6_rcv_bksnd(struct dsptask *task, Uns bid, Uns cnt)
{
	struct t6_udata *udata = task->udata;

	bksnd(task, bid, cnt);	/* echo back */
	udata->waiting = 0;

	return 0;
}

static Uns t6_snd_bkreq(struct dsptask *task)
{
	struct t6_udata *udata = task->udata;

	if (!udata->waiting) {
		bkreq(task, udata->req_cnt);
		udata->req_cnt++;
	}
	udata->waiting = 1;
	return 0;
}

static Uns t6_rcv_tctl(struct dsptask *task, Uns ctlcmd, Uns *ret, Uns arg)
{
	struct t6_udata *udata = task->udata;

	switch (ctlcmd) {
		case MBCMD_TCTL_TEN:
			if (udata->t1q_id != MEM_ILLEGAL)
				/* already registered */
				return 0;
			udata->req_cnt = 1;
			udata->t1q_id = register_tq_1s(task, t6_snd_bkreq);
			if (udata->t1q_id == MEM_ILLEGAL)
				return MBCMD_EID_NORES;
			udata->waiting = 0;
			return 0;
		case MBCMD_TCTL_TDIS:
			unregister_tq_1s(task, udata->t1q_id);
			udata->t1q_id = MEM_ILLEGAL;
			return 0;
		default:
			return MBCMD_EID_BADTCTL;
	}
}

static struct t6_udata udata = { MEM_ILLEGAL, 0, 0 };

#pragma DATA_SECTION(task_test6, "dspgw_task")
struct dsptask task_test6 = {
	TID_MAGIC,	/* tid */
	"test6",	/* name */
	MBCMD_TTYP_GBDM | MBCMD_TTYP_GBMD |
	MBCMD_TTYP_BKDM | MBCMD_TTYP_BKMD |
	MBCMD_TTYP_ASND | MBCMD_TTYP_ARCV,
			/* ttyp: active block snd, active block rcv */
	t6_rcv_bksnd,	/* rcv_snd */
	NULL,		/* rcv_req */
	t6_rcv_tctl,	/* rcv_tctl */
	NULL,		/* tsk_attrs */
	NULL,		/* mmap_info */
	&udata		/* udata */
};
