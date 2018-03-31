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
 * 2004/07/15:  DSP Gateway version 3.3
 */

#include <std.h>
#include "mailbox.h"
#include "tokliBIOS.h"

static Uns mo_rcv_wdsnd(struct dsptask *task, Uns data)
{
	LgUns i;

	for (i = 0; i < 500000L; i++) {}	/* busy wait */

	wdsnd(task, data+1);
	return 0;
}

#pragma DATA_SECTION(task_multiopen, "dspgw_task")
struct dsptask task_multiopen = {
	TID_MAGIC,	/* tid */
	"multiopen",	/* name */
	MBCMD_TTYP_WDDM | MBCMD_TTYP_WDMD |
	MBCMD_TTYP_ASND | MBCMD_TTYP_PRCV,	/* ttyp */
	mo_rcv_wdsnd,	/* rcv_snd */
	NULL,		/* rcv_req */
	NULL,		/* rcv_tctl */
	NULL,		/* tsk_attrs */
	NULL,		/* mmap_info */
	NULL		/* udata */
};
