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
#include "omap1510.h"
#include "mailbox.h"
#include "tokliBIOS.h"

#define is_external(adr)	(((LgUns)(adr) <  DSPRAM_SIZE) ? FALSE : TRUE)
#define MKLONG(uw,lw)	(((LgUns)(uw))<<16 | (lw))

static Uns et_rcv_bksnd(struct dsptask *task, Uns bid, Uns cnt)
{
	LgUns *rcvdata = task->udata;

	if (is_external(ipbuf_d))
		*rcvdata = *((LgUns *)&ipbuf_d[bid][1]);
	else
		*rcvdata = MKLONG(ipbuf_d[bid][1], ipbuf_d[bid][0]);
	unuse_ipbuf(task, bid);

	return 0;
}

static Uns et_rcv_bkreq(struct dsptask *task, Uns cnt)
{
	Uns bid;
	LgUns *rcvdata = task->udata;
	LgUns dat_inc = *rcvdata+1;

	bid = get_free_ipbuf(task);
	if (bid == MBCMD_BID_NULL)
		return MBCMD_EID_STVBUF;
	if (is_external(ipbuf_d))
		*((LgUns *)&ipbuf_d[bid][1]) = dat_inc;
	else {
		ipbuf_d[bid][0] = dat_inc & 0xffff;
		ipbuf_d[bid][1] = dat_inc >> 16;
	}
	bksnd(task, bid, 2);

	return 0;
}

static LgUns rcvdata;

#pragma DATA_SECTION(task_endiantest, "dspgw_task")
struct dsptask task_endiantest = {
	TID_MAGIC,	/* tid */
	"endiantest",	/* name */
	MBCMD_TTYP_GBDM | MBCMD_TTYP_GBMD |
	MBCMD_TTYP_BKDM | MBCMD_TTYP_BKMD |
	MBCMD_TTYP_PSND | MBCMD_TTYP_PRCV,
			/* ttyp: passive block snd, passive block rcv */
	et_rcv_bksnd,	/* rcv_snd */
	et_rcv_bkreq,	/* rcv_req */
	NULL,		/* rcv_tctl */
	NULL,		/* tsk_attrs */
	NULL,		/* mmap_info */
	&rcvdata	/* udata */
};
