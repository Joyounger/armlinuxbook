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
#include <tsk.h>
#include "mailbox.h"
#include "tokliBIOS.h"

#define T8A_STR	"Hello from task8a! *   \n"
#define T8B_STR	"Hello from task8b!  *  \n"
#define T8C_STR	"Hello from task8c!   * \n"
#define T8A_INT	1040000L
#define T8B_INT	400000L
#define T8C_INT	400000L

struct t8_udata {
	char *s;
	LgUns interval;
};

static Void busywait(LgUns cnt)
{
	LgUns i;
	for (i = 0; i < cnt; i++) {}
}

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

static Uns t8_rcv_wdsnd(struct dsptask *task, Uns data)
{
	struct t8_udata *udata = task->udata;
	LgUns interval = udata->interval;
	Uns bid;
	Int cnt, i;

	if (data > 100)
		poll_disable(task);	/* prevent POLL during this function */
	for (i = 0; i < data; i++) {
		busywait(interval);
		bid = get_free_ipbuf(task);
		if (bid == MBCMD_BID_NULL)
			return MBCMD_EID_STVBUF;
		cnt = strcpy16to8((Char *)ipbuf_d[bid], udata->s);
		bksnd(task, bid, cnt);
	}

	return 0;
}

/*
 * task 8a: normal priority
 */
static struct t8_udata udata_a = { T8A_STR, T8A_INT };

#pragma DATA_SECTION(task_test8a, "dspgw_task")
struct dsptask task_test8a = {
	TID_MAGIC,	/* tid */
	"test8a",	/* name */
	MBCMD_TTYP_GBDM |
	MBCMD_TTYP_BKDM | MBCMD_TTYP_WDMD |
	MBCMD_TTYP_ASND | MBCMD_TTYP_PRCV,
			/* ttyp: active block snd, passive word rcv */
	t8_rcv_wdsnd,	/* rcv_snd */
	NULL,		/* rcv_req */
	NULL,		/* rcv_tctl */
	NULL,		/* tsk_attrs */
	NULL,		/* mmap_info */
	&udata_a	/* udata */
};

/*
 * task 8b: normal priority
 */
static struct t8_udata udata_b = { T8B_STR, T8B_INT };

#pragma DATA_SECTION(task_test8b, "dspgw_task")
struct dsptask task_test8b = {
	TID_MAGIC,	/* tid */
	"test8b",	/* name */
	MBCMD_TTYP_GBDM |
	MBCMD_TTYP_BKDM | MBCMD_TTYP_WDMD |
	MBCMD_TTYP_ASND | MBCMD_TTYP_PRCV,
			/* ttyp: active block snd, passive word rcv */
	t8_rcv_wdsnd,	/* rcv_snd */
	NULL,		/* rcv_req */
	NULL,		/* rcv_tctl */
	NULL,		/* tsk_attrs */
	NULL,		/* mmap_info */
	&udata_b	/* udata */
};

/*
 * task 8c: high priority
 */
static struct TSK_Attrs attr_c = {
	DSPTASK_DEFAULT_PRIORITY+1,	/* priority */
	NULL,				/* stack */
	DSPTASK_DEFAULT_STACKSIZE,	/* stacksize */
	DSPTASK_DEFAULT_SYSSTACKSIZE,	/* sysstacksize */
	0,				/* stackseg: can't be overridden */
	NULL,				/* environ */
	NULL,				/* name: can't be overridden */
	TRUE				/* exitflag */
};

static struct t8_udata udata_c = { T8C_STR, T8C_INT };

#pragma DATA_SECTION(task_test8c, "dspgw_task")
struct dsptask task_test8c = {
	TID_MAGIC,	/* tid */
	"test8c",	/* name */
	MBCMD_TTYP_GBDM |
	MBCMD_TTYP_BKDM | MBCMD_TTYP_WDMD |
	MBCMD_TTYP_ASND | MBCMD_TTYP_PRCV,
			/* ttyp: active block snd, passive word rcv */
	t8_rcv_wdsnd,	/* rcv_snd */
	NULL,		/* rcv_req */
	NULL,		/* rcv_tctl */
	&attr_c,	/* tsk_attrs */
	NULL,		/* mmap_info */
	&udata_c	/* udata */
};
