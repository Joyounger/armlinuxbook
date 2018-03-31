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

static Void busywait(Uns cnt)
{
	Uns i;
	for (i = 0; i < cnt; i++) {}
}

static Uns t4_rcv_wdsnd(struct dsptask *task, Uns data)
{
	Uns bid;
	Int ipbuf_trycnt = 0;
	Int i, j, k;
	Uns *p;

	for (i = 0; i < data; i++) {
		ipbuf_trycnt = 0;
		for (;;) {
			bid = get_free_ipbuf(task);
			if (bid != MBCMD_BID_NULL)
				break;
			if (++ipbuf_trycnt >= 100)
				return MBCMD_EID_STVBUF;
			busywait(100);
		}
		p = ipbuf_d[bid];
		for(j = 0; j < 4; j++) {
			for(k = 0; k < 30; k++) {
				*(p++) = 0x4141 + 0x0101 * j;
			}
			*(p++) = 0x0a00 | (0x30 + (i&0x07));
		}
		bksnd(task, bid, 124);
	}
	return 0;
}

#pragma DATA_SECTION(task_test4, "dspgw_task")
struct dsptask task_test4 = {
	TID_MAGIC,	/* tid */
	"test4",	/* name */
	MBCMD_TTYP_GBDM |
	MBCMD_TTYP_BKDM | MBCMD_TTYP_WDMD |
	MBCMD_TTYP_ASND | MBCMD_TTYP_PRCV,
			/* ttyp: active block snd, passive word rcv */
	t4_rcv_wdsnd,	/* rcv_snd */
	NULL,		/* rcv_req */
	NULL,		/* rcv_tctl */
	NULL,		/* tsk_attrs */
	NULL,		/* mmap_info */
	NULL		/* udata */
};
