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
 * 2004/09/27:  DSP Gateway version 3.3
 */

#include <std.h>
#include "mailbox.h"
#include "tokliBIOS.h"

static Void busywait(LgUns cnt)
{
	LgUns i;
	for (i = 0; i < cnt; i++) {}
}

static Uns demo_rcv_wdsnd(struct dsptask *task, Uns data)
{
	Int i;
	Char *s = "Congraturations! DSP is working!";

	for (i = 0; i < 32; i++) {
		wdsnd(task, 0x003a);	/* ':' */
		busywait(0x80000);
	}
	wdsnd(task, 0x000d);	/* CR */
	for (i = 0; i < 32; i++) {
		wdsnd(task, 0x003c);	/* '<' */
		wdsnd(task, 0x0008);	/* BS */
		busywait(0x100000);
		wdsnd(task, 0x002d);	/* '-' */
		wdsnd(task, 0x0008);	/* BS */
		busywait(0x100000);
		wdsnd(task, s[i]);
	}
	wdsnd(task, 0x000a);	/* LF */
	return 0;
}

#pragma DATA_SECTION(task_demo_console, "dspgw_task")
struct dsptask task_demo_console = {
	TID_MAGIC,	/* tid */
	"demo_console",	/* name */
	MBCMD_TTYP_WDDM | MBCMD_TTYP_WDMD |
	MBCMD_TTYP_ASND | MBCMD_TTYP_PRCV,
			/* ttyp: active word snd, passive word rcv */
	demo_rcv_wdsnd,	/* rcv_snd */
	NULL,		/* rcv_req */
	NULL,		/* rcv_tctl */
	NULL,		/* tsk_attrs */
	NULL,		/* mmap_info */
	NULL		/* udata */
};
