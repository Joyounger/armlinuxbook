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
 * 2005/06/29:  DSP Gateway version 3.3
 */

#ifndef __TOKLIBIOS_H
#define __TOKLIBIOS_H

#include <std.h>
#include <tsk.h>

#define TID_MAGIC	0xdead

struct dsptask {
	Uns tid;
	String name;
	Uns ttyp;
	Uns (*rcv_snd)();
	/* (*rcv_wdsnd)(struct dsptask *task, Uns data); */
	/* (*rcv_bksnd)(struct dsptask *task, Uns bid, Uns cnt); */
	/* (*rcv_bksndp)(struct dsptask *task); */
	Uns (*rcv_req)();
	/* (*rcv_wdreq)(struct dsptask *task); */
	/* (*rcv_bkreq)(struct dsptask *task, Uns cnt); */
	Uns (*rcv_tctl)();
	/* (*rcv_tctl)(struct dsptask *task, Uns ctlcmd, Uns *ret, Uns arg); */
	struct TSK_Attrs *tsk_attrs;
	struct mmap_info *mmap_info;
	Void *udata;
};

struct mmap_info {
	Void *start;
	LgUns length;
};

#define DSPTASK_DEFAULT_PRIORITY	2
#define DSPTASK_DEFAULT_STACKSIZE	512	/* default 1024 */
#define DSPTASK_DEFAULT_SYSSTACKSIZE	256	/* default 256 */

/* this must be multiple of 2 for long word alignment */
#define IPBUF_HEADER_SZ		6
#define ASM_IPBUF_HEADER_SZ	"6"
#define DECLARE_IPBUF(sz, lines) \
	asm(" .global __ipbuf_sz\n" \
	    " .global __ipbuf_lines\n" \
	    " .global __ipbuf_body\n" \
	    " .data\n" \
	    "__ipbuf_sz	.uword " #sz "\n" \
	    "__ipbuf_lines .uword " #lines "\n" \
	    "__ipbuf_body .usect \"ipbuf\", (" #sz "+" ASM_IPBUF_HEADER_SZ ")*" #lines ", , 2\n")

extern Uns **ipbuf_d;

extern Uns get_free_ipbuf(struct dsptask *task);
extern Void unuse_ipbuf(struct dsptask *task, Uns bid);
extern Void *register_tq_1s(struct dsptask *task,
			    Uns (*f)(struct dsptask *task));
extern Void unregister_tq_1s(struct dsptask *task, Void *id);
extern Void wdsnd(struct dsptask *task, Uns data);
extern Void bksnd(struct dsptask *task, Uns bid, Uns cnt);
extern Void bksndp(struct dsptask *task, Void *data, Uns cnt);
extern Void wdreq(struct dsptask *task);
extern Void bkreq(struct dsptask *task, Uns cnt);
extern Void bkreqp(struct dsptask *task, Void *data, Uns cnt);
extern Void cmderr(struct dsptask *task, Uns eid);
extern Void dbg(struct dsptask *task, Char *fmt, ...);
extern Void enable_domain(Uns val);
extern Void disable_domain(Uns val);
extern Void poll_disable(struct dsptask *task);
extern Void poll_exclude(struct dsptask *task);

extern Bool fb_lock_timeout(Uns timeout);
#define fb_lock()	fb_lock_timeout(SYS_FOREVER)
extern Void fb_unlock(Void);

#endif /* __TOKLIBIOS_H */
