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

#define T7_BUF_SZ	0x10

struct t7_udata {
	Int *rbuf;
	Int *wbuf;
};

static Uns t7_rcv_bksnd(struct dsptask *task, Uns cnt)
{
	struct t7_udata *udata = task->udata;

	if (cnt > T7_BUF_SZ)
		return MBCMD_EID_BADCNT;

	memcpy(udata->wbuf, udata->rbuf, cnt);
	bksndp(task, udata->wbuf, cnt);	/* echo back */
	bkreqp(task, udata->rbuf, T7_BUF_SZ);

	return 0;
}

static Uns t7_rcv_tctl(struct dsptask *task, Uns ctlcmd, Uns *ret, Uns arg)
{
	struct t7_udata *udata = task->udata;

	switch (ctlcmd) {
		case MBCMD_TCTL_TINIT:
			memset(udata->rbuf, 0xbeef, T7_BUF_SZ);
			memset(udata->wbuf, 0xdead, T7_BUF_SZ);
			bkreqp(task, udata->rbuf, T7_BUF_SZ);
			return 0;
		default:
			return MBCMD_EID_BADTCTL;
	}
}

#pragma DATA_SECTION(t7_rbuf_data, "ipbuf");
#pragma DATA_SECTION(t7_wbuf_data, "ipbuf");
static Int t7_rbuf_data[T7_BUF_SZ];
static Int t7_wbuf_data[T7_BUF_SZ];
static struct t7_udata udata = { t7_rbuf_data, t7_wbuf_data };

#pragma DATA_SECTION(task_test7, "dspgw_task")
struct dsptask task_test7 = {
	TID_MAGIC,	/* tid */
	"test7",	/* name */
	MBCMD_TTYP_PVDM | MBCMD_TTYP_PVMD |
	MBCMD_TTYP_BKDM | MBCMD_TTYP_BKMD |
	MBCMD_TTYP_ASND | MBCMD_TTYP_ARCV,
			/* ttyp: active block snd, active block rcv */
	t7_rcv_bksnd,	/* rcv_snd */
	NULL,		/* rcv_req */
	t7_rcv_tctl,	/* rcv_tctl */
	NULL,		/* tsk_attrs */
	NULL,		/* mmap_info */
	&udata		/* udata */
};
