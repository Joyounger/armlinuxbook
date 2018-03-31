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
 * 2005/06/02:  DSP Gateway version 3.3
 */

#include <std.h>
#include <sys.h>
#include <sem.h>
#include <tsk.h>

#include "omap1510.h"
#include "mailbox.h"
#include "tokliBIOS.h"
#include "tokliBIOSlib.h"
#include "tokliBIOScfg.h"
#include "timer.h"

struct mailboxq_super {
	Uns cmd_h;
	Uns cmd_l;
	Uns data;
};

#define MBQ_MAX	8

struct supertask {
	Uns mbq_wp, mbq_rp;
	struct mailboxq_super mbq[MBQ_MAX];
};

struct supertask super;
static TSK_Handle faulty_tsk;

Void super_recover_int(Void)
{
	/*
	 * This function is called in interrupt handler.
	 */
#if 0
	TSK_Stat stat;
#endif

	faulty_tsk = TSK_self();
#if 0
	TSK_stat(faulty_tsk, &stat);
	dbg(&task_anon, " sp    = %p\n", stat.sp);
#endif
	/*
	 * FIXME: lock the faulty_tsk and prevent from deleted.
	 */
}

static Uns super_recover(Void)
{
	TSK_Stat stat;
	struct task_prop *prop;
	struct dsptask *task;
	Uns *sp, *ssp;
	LgUns fadr;
	Uns i;

	TSK_stat(faulty_tsk, &stat);
	sp = (Uns *)stat.sp;
	ssp = (Uns *)sp[0];
	fadr = ((LgUns)ssp[0x20] << 16) | sp[0x52];
	dbg(&task_anon,
	    "task %s: sp = %p, ssp = %p\n"
	    "it caused MMU fault at 0x%06lx.\n"
	    "(This address calculation has not been well tested,\n"
	    " so please don't trust me!)\n",
	    TSK_getname(faulty_tsk), sp, ssp, fadr);
#if 0
	dbg(&task_anon, " stack = %p\n", stat.attrs.stack);
	dbg(&task_anon, " sp    = %p\n", stat.sp);
#endif

	prop = TSK_getenv(faulty_tsk);
	if (prop == NULL)	/* faulty_tsk is not usertask. */
		goto done;
	for (i = 0; i < N_TASK_MAX; i++) {
		if (prop == task_prop[i])
			goto senderr;
	}
	/* faulty_tsk is not usertask. */
	goto done;

senderr:
	task = prop->dsptask;
	sys_cmderr(MBCMD_EID_FATAL, task->tid);

done:
	return 0;
}

static Uns super_runlevel(Uns level)
{
	switch (level) {
	case MBCMD_RUNLEVEL_USER:
		TSK_setpri(&TSK_sleep, 1);
		return 0;
	case MBCMD_RUNLEVEL_SUPER:
		TSK_setpri(&TSK_sleep, 14);
		return 0;
	case MBCMD_RUNLEVEL_RECOVERY:
		TSK_setpri(&TSK_sleep, 14);
		return super_recover();
	default:
		return MBCMD_EID_BADPARAM;
	}
}

static Uns super_pm(Uns subcmd, Uns mask)
{
	switch (subcmd) {
	case MBCMD_PM_ENABLE:
		enable_domain(mask);
		return 0;
	case MBCMD_PM_DISABLE:
		disable_domain(mask);
		return 0;
	default:
		return MBCMD_EID_BADCMD;
	}
}

extern Void __suspend(Void);
extern Void __resume(Void);

static Uns super_suspend(Void)
{
	Uns intm_saved;
	LgUns *reset_vect = (LgUns *)((LgUns)(*_IVPD) << 7);
	struct {
		Uns ier0;
		Uns ier1;
	} reg_content;

	intm_saved = HWI_disable();

	/* save register contents */
	reg_content.ier0 = *_IER0;
	reg_content.ier1 = *_IER1;

	/* set reset vector for resume */
	*reset_vect = (LgUns)__resume;

	mbsend(mbcmd(MBCMD_SUSPEND, 0), 0);
	__suspend();

	/* restore register contents */
	*_IER0 = reg_content.ier0;
	*_IER1 = reg_content.ier1;

	/* re-init */
	init_icache();
	__wdt_disable();
	init_wdt();
	init_timer();

	HWI_restore(intm_saved);
	return 0;
}

static Uns kfunc_fbctl(Uns data)
{
	switch (data) {
	case MBCMD_FBCTL_ENABLE:
		fb_enable();
		return 0;
	case MBCMD_FBCTL_DISABLE:
		fb_disable();
		return 0;
	default:
		return MBCMD_EID_BADCMD;
	}
}

static Uns super_kfunc(Uns subcmd, Uns data)
{
	switch (subcmd) {
	case MBCMD_KFUNC_FBCTL:
		return kfunc_fbctl(data);
	default:
		return MBCMD_EID_BADCMD;
	}
}

static Uns super_dspcfg(Uns cfgtyp)
{
	reset_mailbox();

	if (!init_done()) {
		/* init was not successful. abort dspcfg. */
		mbsend(mbcmd(MBCMD_DSPCFG,
			     MBCMD_DSPCFG_ABORT | MBCMD_DSPCFG_LAST), 0);
		return 0;
	}

	init_timer();	/* overwriting DSP/BIOS's settings */

	if (cfgtyp != MBCMD_DSPCFG_REQ)
		return MBCMD_EID_BADCFGTYP;

	/*
	 * system ipbuf initialization
	 */
	ipbuf_sys_da.s = MBCMD_TID_FREE;
	ipbuf_sys_ad.s = MBCMD_TID_FREE;

	ipbuf_sys_da.d[ 0] = n_task;
	ipbuf_sys_da.d[ 1] = _ipbuf_lines;
	ipbuf_sys_da.d[ 2] = _ipbuf_sz;
	ipbuf_sys_da.d[ 3] = (LgUns)&_ipbuf_body >> 16;
	ipbuf_sys_da.d[ 4] = (LgUns)&_ipbuf_body & 0xffff;
	ipbuf_sys_da.d[ 5] = (LgUns)&ipbuf_sys_da >> 16;
	ipbuf_sys_da.d[ 6] = (LgUns)&ipbuf_sys_da & 0xffff;
	ipbuf_sys_da.d[ 7] = (LgUns)&ipbuf_sys_ad >> 16;
	ipbuf_sys_da.d[ 8] = (LgUns)&ipbuf_sys_ad & 0xffff;
	ipbuf_sys_da.d[ 9] = (LgUns)&mbseq >> 16;
	ipbuf_sys_da.d[10] = (LgUns)&mbseq & 0xffff;
	ipbuf_sys_da.d[11] = (LgUns)&dbgbuf >> 16;
	ipbuf_sys_da.d[12] = (LgUns)&dbgbuf & 0xffff;
	ipbuf_sys_da.d[13] = DBG_BUF_SZ;
	ipbuf_sys_da.d[14] = DBG_LINE_SZ;
	ipbuf_sys_da.d[15] = (LgUns)mem_sync.DARAM >> 16;
	ipbuf_sys_da.d[16] = (LgUns)mem_sync.DARAM & 0xffff;
	ipbuf_sys_da.d[17] = (LgUns)mem_sync.SARAM >> 16;
	ipbuf_sys_da.d[18] = (LgUns)mem_sync.SARAM & 0xffff;
	ipbuf_sys_da.d[19] = (LgUns)mem_sync.SDRAM >> 16;
	ipbuf_sys_da.d[20] = (LgUns)mem_sync.SDRAM & 0xffff;
	ipbuf_sys_da.s = MBCMD_TID_ANON;

	mbsend(mbcmd(MBCMD_DSPCFG, MBCMD_DSPCFG_PROTREV),
	       MBPROT_REVISION);
	mbsend(mbcmd(MBCMD_DSPCFG, MBCMD_DSPCFG_SYSADRH),
	       (LgUns)&ipbuf_sys_da >> 16);
	mbsend(mbcmd(MBCMD_DSPCFG, MBCMD_DSPCFG_SYSADRL | MBCMD_DSPCFG_LAST),
	       (LgUns)&ipbuf_sys_da & 0xffff);
	init_ipbuf();
	init_dbg();

	/*
	 * dspcfg assumes DMA domain is already enabled by ARM.
	 */
	enable_domain(_ICR_DMA_IDLE_DOMAIN);

	return 0;
}

static Uns super_poll(Void)
{
	/*
	 * poll_broadcast returns the task count which should respond to poll.
	 * if the return value is 0, return POLL response immediately.
	 */
	if (poll_broadcast(MBCMD_POLL_ARM) == 0)
		mbsend(mbcmd(MBCMD_POLL, 0), 0);

	return 0;
}

static Uns super_tadd(Void)
{
	struct dsptask *new;
	struct task_prop *prop = MEM_ILLEGAL;
	Uns tid;
	Uns ret;

	sync_with_arm(&ipbuf_sys_ad.s, MBCMD_TID_ANON);
	sync_mem_all();
	new = (Void *)MKLONG(ipbuf_sys_ad.d[0], ipbuf_sys_ad.d[1]);
	ipbuf_sys_ad.s = MBCMD_TID_FREE;

	for (tid = n_task; tid < N_TASK_MAX; tid++) {
		if (task_prop[tid] == NULL)
			goto found;
	}
	/* no room in task pointer */
	ret = MBCMD_EID_NORES;
	goto fail;

found:
	prop = MEM_alloc(MEM->MALLOCSEG, sizeof(struct task_prop), 2);
	if (prop == MEM_ILLEGAL) {
		ret = MBCMD_EID_NOMEM;
		goto fail;
	}
	if ((ret = task_config(new, prop, tid)) != 0) {
		if (ret == MBCMD_EID_BADTID)	/* tid != TID_MAGIC */
			ret = MBCMD_EID_BADADR;
		goto fail;
	}
	mbsend(mbcmd(MBCMD_TADD, tid), 0);
	return 0;

fail:
	if (prop != MEM_ILLEGAL)
		MEM_free(MEM->MALLOCSEG, prop, sizeof(struct task_prop));
	mbsend(mbcmd(MBCMD_TADD, MBCMD_TID_ANON), 0);
	return ret;
}

static Uns super_tdel(Uns tid, Uns how)
{
	struct task_prop *prop = task_prop[tid];
	Uns ret;

	if ((tid < n_task) || (tid >= N_TASK_MAX) || (task_prop[tid] == NULL)) {
		ret = MBCMD_EID_BADTID;
		goto fail;
	}
	if (how == MBCMD_TDEL_KILL) {
		struct dsptask *task = prop->dsptask;

		/* return value of TKILL will be ignored */
		if (task->rcv_tctl)
			task->rcv_tctl(task, MBCMD_TCTL_TKILL, NULL, 0);
	}
	// note: task_unconfig() sets task_prop[tid]=NULL.
	task_unconfig(tid);
	MEM_free(MEM->MALLOCSEG, prop, sizeof(struct task_prop));

	/*
	 * cache clear and wait for the process done
	 */
	asm(" BSET CACLR");
	do {} while(*_ST3_55 & _ST3_55_CACLR);

	mbsend(mbcmd(MBCMD_TDEL, tid), 0);
	return 0;

fail:
	mbsend(mbcmd(MBCMD_TDEL, MBCMD_TID_ANON), 0);
	return ret;
}

static Uns super_regrw(Uns cmd_l, Uns ad)
{
	switch (cmd_l) {
	case MBCMD_REGRW_MEMR:
		mbsend(mbcmd(MBCMD_REGRW, MBCMD_REGRW_DATA), *(Uns *)ad);
		break;

	case MBCMD_REGRW_IOR:
		mbsend(mbcmd(MBCMD_REGRW, MBCMD_REGRW_DATA), inw(ad));
		break;

	case MBCMD_REGRW_MEMW:
		sync_with_arm(&ipbuf_sys_ad.s, MBCMD_TID_ANON);
		*(Uns *)ad = ipbuf_sys_ad.d[0];
		ipbuf_sys_ad.s = MBCMD_TID_FREE;
		break;
	case MBCMD_REGRW_IOW:
		sync_with_arm(&ipbuf_sys_ad.s, MBCMD_TID_ANON);
		outw(ipbuf_sys_ad.d[0], ad);
		ipbuf_sys_ad.s = MBCMD_TID_FREE;
		break;

	case MBCMD_REGRW_DATA:
	default:
		return MBCMD_EID_BADPARAM;
	}

	return 0;
}

static Uns super_setvar(Uns varid, Uns val)
{
	switch (varid) {
	case MBCMD_VARID_ICRMASK:
		set_icr_mask(val);
		return 0;
	default:
		return MBCMD_EID_BADPARAM;
	}
}

static Uns super_getvar(Uns varid)
{
	switch (varid) {
	case MBCMD_VARID_ICRMASK:
		mbsend(mbcmd(MBCMD_GETVAR, MBCMD_VARID_ICRMASK),
		       get_icr_mask());
		return 0;
	case MBCMD_VARID_LOADINFO:
		{
			struct load_info linfo;
			lock_ipbuf_sys_da();
			get_load_info(&linfo);
			ipbuf_sys_da.d[0] = linfo.a_10ms;
			ipbuf_sys_da.d[1] = linfo.a_1s;
			ipbuf_sys_da.d[2] = linfo.h_1s;
			ipbuf_sys_da.d[3] = linfo.a_1m;
			ipbuf_sys_da.d[4] = linfo.h_1m;
			ipbuf_sys_da.s = MBCMD_TID_ANON;
			mbsend(mbcmd(MBCMD_GETVAR, MBCMD_VARID_LOADINFO), 0);
			unlock_ipbuf_sys_da();
		}
		return 0;
	default:
		return MBCMD_EID_BADPARAM;
	}
}

static Void do_super(Uns cmd_h, Uns cmd_l, Uns data)
{
	Uns eid;

	switch (cmd_h) {
	case MBCMD_BKYLD:
		eid = release_ipbuf(data);
		break;

	case MBCMD_RUNLEVEL:
		eid = super_runlevel(cmd_l);
		break;

	case MBCMD_PM:
		eid = super_pm(cmd_l, data);
		break;

	case MBCMD_SUSPEND:
		eid = super_suspend();
		break;

	case MBCMD_KFUNC:
		eid = super_kfunc(cmd_l, data);
		break;

	case MBCMD_DSPCFG:
		eid = super_dspcfg(cmd_l);
		break;

	case MBCMD_POLL:
		eid = super_poll();
		break;

	case MBCMD_TADD:
		eid = super_tadd();
		break;

	case MBCMD_TDEL:
		eid = super_tdel(cmd_l, data);
		break;

	case MBCMD_REGRW:
		eid = super_regrw(cmd_l, data);
		break;

	case MBCMD_GETVAR:
		eid = super_getvar(cmd_l);
		break;

	case MBCMD_SETVAR:
		eid = super_setvar(cmd_l, data);
		break;

	default:
		eid = MBCMD_EID_BADCMD;
	}

	if (eid)
		sys_cmderr(eid, mbcmd(cmd_h, cmd_l));
}

/*
 * supervisor task function
 */
Void supertask(Void)
{
	struct mailboxq_super *mbq = super.mbq;
	Uns *rp = &super.mbq_rp;
	Uns intm_saved;
	Int mbq_full;

start:
	SEM_pend(&SEM_super, SYS_FOREVER);	/* wait for next one */

	do_super(mbq[*rp].cmd_h, mbq[*rp].cmd_l, mbq[*rp].data);
	intm_saved = HWI_disable();
	mbq_full = (*rp == super.mbq_wp);
	if (++*rp == MBQ_MAX)
		*rp = 0;
	/*
	 * interrupt had been disabled in case the mbq has been full.
	 * now we have a space to accept.
	 */
	if (mbq_full)
		enable_irq(INT_MAILBOX1);
	HWI_restore(intm_saved);
	goto start;
}

/*
 * registering command to supertask's mbq.
 */
Void register_super_mbq(Uns cmd_h, Uns cmd_l, Uns data)
{
	struct mailboxq_super *mbq_ent;
	Uns *wp;

	wp = &super.mbq_wp;
	mbq_ent = &super.mbq[*wp];
	mbq_ent->cmd_h = cmd_h;
	mbq_ent->cmd_l = cmd_l;
	mbq_ent->data  = data;

	if (++*wp == MBQ_MAX)
		*wp = 0;
	if (*wp == super.mbq_rp)	/* mbq is full! */
		disable_irq(INT_MAILBOX1);

	SEM_post(&SEM_super);
}

