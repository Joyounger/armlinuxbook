/*
 * Copyright (c) 2004-2005, Nokia
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
 * 2004/11/05:  DSP Gateway version 3.3
 */

#include <std.h>
#include <clk.h>
#include "mailbox.h"
#include "tokliBIOS.h"

static Void busywait(LgUns cnt)
{
	LgUns i;
	for (i = 0; i < cnt; i++) {}
}

static Uns mm_rcv_bksnd(struct dsptask *task, Uns bid, Uns cnt)
{
	Int i;
	Uns *ipbp = ipbuf_d[bid];
	Uns off, len;
	#define OFF	228	/* arbitrary */
	Uns bid2;
	Uns *addr = (Uns *)task->udata;

	if (cnt != 2) {
		dbg(task, "[DSP]: illegal data cnt: %d\n", cnt);
		unuse_ipbuf(task, bid);
		return 0;
	}

	off = ipbp[0];	/* offset within the mapped space */
	len = ipbp[1];	/* len (in word count) */
	unuse_ipbuf(task, bid);

	busywait(2000000L);

	dbg(task, "\n"
		  "[DSP]: offset = %d, len = %d\n", off, len);

	for (i = 0; i < len; i++) {
		dbg(task, "[DSP] addr[%d] = %d\n", off+i, addr[off+i]);
		/* increment and copy */
		addr[OFF+i] = addr[off+i] + 1;
	}

	busywait(2000000L);

	dbg(task, "\n"
		  "[DSP]: sending back data to ARM:\n"
		  "       offset = %d, len = %d\n", OFF, len);
	bid2 = get_free_ipbuf(task);
	if(bid2 == MBCMD_BID_NULL)
		return MBCMD_EID_STVBUF;
	ipbp = ipbuf_d[bid2];
	ipbp[0] = OFF;
	ipbp[1] = len;
	bksnd(task, bid2, 2);

	return 0;
}

#pragma DATA_SECTION(my_vma, "shared_buf");
#define BUFSIZ 2048
static Uns my_vma[BUFSIZ];

static struct mmap_info mmap_info = {
	my_vma,	/* start */
	BUFSIZ	/* length */
};

#pragma DATA_SECTION(mmap, "dspgw_task")
struct dsptask mmap = {
	TID_MAGIC,	/* tid */
	"mmap",		/* name */
	MBCMD_TTYP_BKDM | MBCMD_TTYP_BKMD |
	MBCMD_TTYP_ASND | MBCMD_TTYP_PRCV,	/* ttyp */
	mm_rcv_bksnd,	/* rcv_snd */
	NULL,		/* rcv_req */
	NULL,		/* rcv_tctl */
	NULL,		/* tsk_attrs */
	&mmap_info,	/* mmap_info */
	my_vma		/* udata */
};
