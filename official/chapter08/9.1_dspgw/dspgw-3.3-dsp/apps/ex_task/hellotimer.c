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
 * 2005/06/06:  DSP Gateway version 3.3
 */

#include <std.h>
#include <sys.h>
#include <mem.h>
#include <sem.h>
#include <tsk.h>
#include "mailbox.h"
#include "tokliBIOS.h"

struct ht_udata {
	Int cnt;
	SEM_Handle sem_timer;
	TSK_Handle tsk;
	Void *tq_id;
};

static Int strcpy16to8(Char *d, Char *s)
{
	Int cnt = 0;

	for (; *s; s++, d++) {
		*d = *s;
		*d |= *++s << 8;
		cnt++;
		if (!*s)
			return cnt;
	}
	*d = '\0';
	return cnt;
}

static Uns ht_callback(struct dsptask *task)
{
	struct ht_udata *udata = (struct ht_udata *)task->udata;

	SEM_post(udata->sem_timer);
	return 0;
}

static Void hello_timer(Arg task_arg)
{
	struct dsptask *task = ArgToPtr(task_arg);
	struct ht_udata *udata = (struct ht_udata *)task->udata;
	Uns bid;
	Int cnt;

	TSK_setpri(TSK_self(), DSPTASK_DEFAULT_PRIORITY);

	for (;;) {
		SEM_pend(udata->sem_timer, SYS_FOREVER);

		dbg(task, "ht_loop(): udata->cnt = %d\n", udata->cnt);
		bid = get_free_ipbuf(task);
		if (bid == MBCMD_BID_NULL) {
			cmderr(task, MBCMD_EID_STVBUF);
			unregister_tq_1s(task, udata->tq_id);
		}
		cnt = strcpy16to8((Char *)ipbuf_d[bid], "Hello from DSP!\n");
		bksnd(task, bid, cnt);
		if (--udata->cnt == 0)
			unregister_tq_1s(task, udata->tq_id);
	}
}

static Void init_obj(struct dsptask *task)
{
	struct ht_udata *udata = (struct ht_udata *)task->udata;

	udata->cnt       = 0;
	udata->sem_timer = NULL;
	udata->tsk       = NULL;
	udata->tq_id     = MEM_ILLEGAL;
}

static Void free_obj(struct dsptask *task)
{
	struct ht_udata *udata = (struct ht_udata *)task->udata;
	Uns intm_saved;

	intm_saved = HWI_disable();
	udata->cnt = 0;
	if (udata->tq_id != MEM_ILLEGAL) {
		unregister_tq_1s(task, udata->tq_id);
		udata->tq_id = MEM_ILLEGAL;
	}
	if (udata->tsk != NULL) {
		TSK_delete(udata->tsk);
		udata->tsk = NULL;
	}
	if (udata->sem_timer != NULL) {
		SEM_delete(udata->sem_timer);
		udata->sem_timer = NULL;
	}
	HWI_restore(intm_saved);
}

static Uns ht_rcv_wdsnd(struct dsptask *task, Uns data)
{
	struct ht_udata *udata = (struct ht_udata *)task->udata;
	free_obj(task);
	udata->cnt = data;
	udata->sem_timer = SEM_create(0, NULL);
	udata->tsk       = TSK_create((Fxn)hello_timer, NULL, (Arg)task);
	udata->tq_id     = register_tq_1s(task, ht_callback);
	if ((udata->sem_timer == NULL) ||
	    (udata->tsk       == NULL) ||
	    (udata->tq_id     == MEM_ILLEGAL)) {
		free_obj(task);
		return MBCMD_EID_NOMEM;
	}
	return 0;
}

static Uns ht_rcv_tctl(struct dsptask *task, Uns ctlcmd, Uns *ret, Uns arg)
{
	switch (ctlcmd) {
		case MBCMD_TCTL_TINIT:
			init_obj(task);
			break;

		case MBCMD_TCTL_TCLR:
			free_obj(task);
			break;

		case 0x8001:	/* stop */
			dbg(task, "[DSP] hellotimer TCTL 8001: stop");
			free_obj(task);
			break;

		default:
			return MBCMD_EID_BADTCTL;
	}

	return 0;
}

static struct ht_udata udata;

#pragma DATA_SECTION(task_hellotimer, "dspgw_task")
struct dsptask task_hellotimer = {
	TID_MAGIC,	/* tid */
	"hellotimer",	/* name */
	MBCMD_TTYP_GBDM |
	MBCMD_TTYP_BKDM | MBCMD_TTYP_WDMD |
	MBCMD_TTYP_ASND | MBCMD_TTYP_PRCV,
			/* ttyp: active block snd, passive word rcv */
	ht_rcv_wdsnd,	/* rcv_snd */
	NULL,		/* rcv_req */
	ht_rcv_tctl,	/* rcv_tctl */
	NULL,		/* tsk_attrs */
	NULL,		/* mmap_info */
	&udata		/* udata */
};
