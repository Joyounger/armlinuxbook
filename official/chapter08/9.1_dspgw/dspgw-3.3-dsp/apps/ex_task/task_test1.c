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
#include "mailbox.h"
#include "tokliBIOS.h"

struct t1_udata {
	Int wdwptr, wdrptr;
	Uns wddata[64];
};

static Uns t1_rcv_wdsnd(struct dsptask *task, Uns data)
{
	struct t1_udata *udata = task->udata;

	udata->wddata[udata->wdwptr] = data;
	if (udata->wdwptr < 64)
		udata->wdwptr++;

	return 0;
}

static Uns t1_rcv_wdreq(struct dsptask *task)
{
	struct t1_udata *udata = task->udata;

	if (udata->wdwptr == 0)
		return 0;
	wdsnd(task, udata->wddata[udata->wdrptr++]);
	if (udata->wdrptr == udata->wdwptr)
		udata->wdrptr = 0;
	
	return 0;
}

static Uns t1_rcv_tctl(struct dsptask *task, Uns ctlcmd, Uns *ret, Uns arg)
{
	struct t1_udata *udata = task->udata;

	switch (ctlcmd) {
		case MBCMD_TCTL_TINIT:
			udata->wdwptr = 0;
			udata->wdrptr = 0;
			return 0;
		default:
			return MBCMD_EID_BADTCTL;
	}
}

static struct t1_udata udata;

#pragma DATA_SECTION(task_test1, "dspgw_task")
struct dsptask task_test1 = {
	TID_MAGIC,	/* tid */
	"test1",	/* name */
	MBCMD_TTYP_WDDM | MBCMD_TTYP_WDMD |
	MBCMD_TTYP_PSND | MBCMD_TTYP_PRCV,
			/* ttyp: passive word snd, passive word rcv */
	t1_rcv_wdsnd,	/* rcv_snd */
	t1_rcv_wdreq,	/* rcv_req */
	t1_rcv_tctl,	/* rcv_tctl */
	NULL,		/* tsk_attrs */
	NULL,		/* mmap_info */
	&udata		/* udata */
};
