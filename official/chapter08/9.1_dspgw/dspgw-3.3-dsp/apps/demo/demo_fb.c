/*
 * Copyright (c) 2004, Nokia
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
 * 2005/02/25:  DSP Gateway version 3.3
 */

#include <std.h>
#include <mem.h>
#include <tsk.h>
#include <sem.h>
#include "mailbox.h"
#include "tokliBIOS.h"

#include "char-bitmap.h"

struct fbinfo {
	Void *adr;
	Uns width;
	Uns height;
	Uns bpp;
};
struct fbinfo fbinfo = {
	(Void *)0x80000,	/* adr */
	100,			/* width */
	100,			/* height */
	16			/* bpp */
};

struct col {
	Uns bg;
	Uns fg;
} col[8] = {
	{ 0xf800, 0xffff }, /* red */
	{ 0x07e0, 0xffff }, /* green */
	{ 0x001f, 0xffff }, /* blue */
	{ 0xffe0, 0x0000 }, /* yellow */
	{ 0x07ff, 0x0000 }, /* cyan */
	{ 0xf81f, 0x0000 }, /* magenta */
	{ 0xffff, 0x0000 }, /* white */
	{ 0x0000, 0xffff }, /* black */
};
Uns map[8][BM_SZ] = {
	BITMAP_RED,
	BITMAP_GREEN,
	BITMAP_BLUE,
	BITMAP_YELLOW,
	BITMAP_CYAN,
	BITMAP_MAGENTA,
	BITMAP_WHITE,
	BITMAP_BLACK,
};

#define BUF_SZ	sizeof(struct fbinfo)

enum demo_mode { MODE_TIMER, MODE_BUSY };

struct demo_obj {
	SEM_Handle sem_timer;
	TSK_Handle tsk;
	enum demo_mode mode;
	Void *tq_id;
	Int rbuf[BUF_SZ];
};

static Uns fb_callback(struct dsptask *task)
{
	struct demo_obj *obj = (struct demo_obj *)task->udata;

	SEM_post(obj->sem_timer);
	return 0;
}

static Void fb_updator(Arg task_arg)
{
	struct dsptask *task = ArgToPtr(task_arg);
	struct demo_obj *obj = (struct demo_obj *)task->udata;
	Uns x, y;
	static Uns cnt = 0;
	Uns dotsz = fbinfo.width / BM_WID;
	Uns bm_pos;
	Uns shift;
	Uns x1, y1;

start:
	if (obj->mode == MODE_TIMER)
		SEM_pend(obj->sem_timer, SYS_FOREVER);

	/* BPP = 16 */
	for (y = 0; y < fbinfo.height; y++) {
		for (x = 0; x < fbinfo.width; x++) {
			*(Uns *)((LgUns)fbinfo.adr +
				 (LgUns)fbinfo.width * y + x) = col[cnt].bg;
		}
	}
	bm_pos = 0;
	shift = 0;
	x1 = 0, y1 = 0;
	for (y = (fbinfo.height - BM_HEI * dotsz) / 2;
	     y < (fbinfo.height + BM_HEI * dotsz) / 2; y++) {
		for (x = (fbinfo.width - BM_WID * dotsz) / 2;
		     x < (fbinfo.width + BM_WID * dotsz) / 2; x++) {
			if (map[cnt][bm_pos] & (1 << shift))
				*(Uns *)((LgUns)fbinfo.adr +
					 (LgUns)fbinfo.width * y + x) = col[cnt].fg;
			if (++x1 == dotsz) {
				x1 = 0;
				if (++shift == 16) {
					shift = 0;
					bm_pos++;
				}
			}
		}
		if (++y1 == dotsz) {
			y1 = 0;
		} else {
			bm_pos -= BM_WID_WORD;
		}
	}

	if (++cnt == 8)
		cnt = 0;

	goto start;
}

static Uns fb_rcv_bksnd(struct dsptask *task, Uns cnt)
{
	struct demo_obj *obj = (struct demo_obj *)task->udata;

	if (cnt != sizeof(struct fbinfo)) {
		dbg(task, "demo_fb: illegal data size!\n");
		goto out;
	}
	memcpy(&fbinfo, obj->rbuf, cnt);

	dbg(task, "adr=%08lx, width=%d, height=%d, bpp=%d\n",
	    fbinfo.adr, fbinfo.width, fbinfo.height, fbinfo.bpp);
	
out:
	bkreqp(task, obj->rbuf, BUF_SZ);
	return 0;
}

static Void init_obj(struct dsptask *task)
{
	struct demo_obj *obj = (struct demo_obj *)task->udata;

	obj->sem_timer = NULL;
	obj->tsk       = NULL;
	obj->tq_id     = MEM_ILLEGAL;
}

static Void free_obj(struct dsptask *task)
{
	struct demo_obj *obj = (struct demo_obj *)task->udata;
	Uns intm_saved;

	intm_saved = HWI_disable();
	if (obj->tq_id != MEM_ILLEGAL) {
		unregister_tq_1s(task, obj->tq_id);
		obj->tq_id = MEM_ILLEGAL;
	}
	if (obj->tsk != NULL) {
		TSK_delete(obj->tsk);
		obj->tsk = NULL;
	}
	if (obj->sem_timer != NULL) {
		SEM_delete(obj->sem_timer);
		obj->sem_timer = NULL;
	}
	HWI_restore(intm_saved);
}

static Uns alloc_obj(struct dsptask *task, enum demo_mode mode)
{
	struct demo_obj *obj = (struct demo_obj *)task->udata;

	obj->mode = mode;
	if (mode == MODE_TIMER) {
		obj->sem_timer = SEM_create(0, NULL);
		obj->tsk       = TSK_create((Fxn)fb_updator, NULL, (Arg)task);
		obj->tq_id     = register_tq_1s(task, fb_callback);
		if ((obj->sem_timer == NULL) ||
		    (obj->tsk       == NULL) ||
		    (obj->tq_id     == MEM_ILLEGAL))
			goto fail;
	} else {	/* MODE_BUSY */
		obj->sem_timer = NULL;
		obj->tsk       = TSK_create((Fxn)fb_updator, NULL, (Arg)task);
		obj->tq_id     = MEM_ILLEGAL;
		if (obj->tsk == NULL)
			goto fail;
	}

	return 0;

fail:
	free_obj(task);
	return MBCMD_EID_NOMEM;
}

static Uns fb_rcv_tctl(struct dsptask *task, Uns ctlcmd, Uns *ret, Uns arg)
{
	struct demo_obj *obj = (struct demo_obj *)task->udata;
	int r;

	switch (ctlcmd) {
		case MBCMD_TCTL_TINIT:
			init_obj(task);
			bkreqp(task, obj->rbuf, BUF_SZ);
			return 0;

		case 0x8000:	/* timer mode */
			if (fbinfo.bpp != 16) {
				dbg(task, "[DSP] unsupported bpp (%d).\n",
				    fbinfo.bpp);
				break;
			}
			if ((r = alloc_obj(task, MODE_TIMER)) != 0)
				return r;
			break;

		case 0x8001:	/* stop */
			free_obj(task);
			break;

		case 0x8002:	/* timer mode */
			if (fbinfo.bpp != 16) {
				dbg(task, "[DSP] unsupported bpp (%d).\n",
				    fbinfo.bpp);
				break;
			}
			if ((r = alloc_obj(task, MODE_BUSY)) != 0)
				return r;
			break;

		default:
			return MBCMD_EID_BADTCTL;
	}

	return 0;
}

static struct demo_obj demo_obj;

#pragma DATA_SECTION(task_demo_fb, "dspgw_task")
struct dsptask task_demo_fb = {
	TID_MAGIC,	/* tid */
	"demo_fb",	/* name */
	MBCMD_TTYP_GBDM | MBCMD_TTYP_PVMD |
	MBCMD_TTYP_WDDM | MBCMD_TTYP_BKMD |
	MBCMD_TTYP_ASND | MBCMD_TTYP_ARCV,
			/* ttyp: active word snd, active private block rcv */
	fb_rcv_bksnd,	/* rcv_snd */
	NULL,		/* rcv_req */
	fb_rcv_tctl,	/* rcv_tctl */
	NULL,		/* tsk_attrs */
	NULL,		/* mmap_info */
	&demo_obj	/* udata */
};
