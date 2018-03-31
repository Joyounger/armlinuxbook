/*
 * Copyright (c) 2002-2004, Nokia
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
 * 2005/05/19:  DSP Gateway version 3.3
 */

#include <std.h>
#include <sem.h>
#include <tsk.h>
#include "omap1510.h"
#include "mailbox.h"
#include "tokliBIOS.h"
#include "tokliBIOSlib.h"
#include "tokliBIOScfg.h"

Uns n_task = 0;
static struct TSK_Attrs attrs_default;

#define sndtyp_acv(ttyp)	((ttyp) & MBCMD_TTYP_ASND)
#define sndtyp_psv(ttyp)	(!((ttyp) & MBCMD_TTYP_ASND))
#define sndtyp_bk(ttyp)		((ttyp) & MBCMD_TTYP_BKDM)
#define sndtyp_wd(ttyp)		(!((ttyp) & MBCMD_TTYP_BKDM))
#define sndtyp_pv(ttyp)		((ttyp) & MBCMD_TTYP_PVDM)
#define sndtyp_gb(ttyp)		(!((ttyp) & MBCMD_TTYP_PVDM))
#define rcvtyp_acv(ttyp)	((ttyp) & MBCMD_TTYP_ARCV)
#define rcvtyp_psv(ttyp)	(!((ttyp) & MBCMD_TTYP_ARCV))
#define rcvtyp_bk(ttyp)		((ttyp) & MBCMD_TTYP_BKMD)
#define rcvtyp_wd(ttyp)		(!((ttyp) & MBCMD_TTYP_BKMD))
#define rcvtyp_pv(ttyp)		((ttyp) & MBCMD_TTYP_PVMD)
#define rcvtyp_gb(ttyp)		(!((ttyp) & MBCMD_TTYP_PVMD))


static Void get_from_mbq(struct mailboxq *mbq, Uns *rp, SEM_Handle sem,
			 Uns *cmd, Uns *data);

static Uns mbq_wdsnd(Uns tid, Uns data)
{
	struct dsptask *task = task_prop[tid]->dsptask;

	if (!rcvtyp_wd(task->ttyp))
		/* This task doesn't use word transfer. */
		return MBCMD_EID_BADTCN;
	return task->rcv_snd(task, data);
}

static Uns mbq_wdreq(Uns tid)
{
	struct dsptask *task = task_prop[tid]->dsptask;

	if (!sndtyp_wd(task->ttyp) || !sndtyp_psv(task->ttyp))
		/* This task doesn't receive WDREQ. */
		return MBCMD_EID_BADTCN;
	return task->rcv_req(task);
}

static Uns mbq_bksnd(Uns tid, Uns bid)
{
	struct dsptask *task = task_prop[tid]->dsptask;
	Uns cnt;

	/* (note) sync_ipbuf() has been done in interrupt routine */
	if (!rcvtyp_bk(task->ttyp) || !rcvtyp_gb(task->ttyp)) {
		/* This task doesn't use global ipbuf. */
		unuse_ipbuf(task, bid);
		return MBCMD_EID_BADTCN;
	}
	cnt = ipbuf[bid]->c;
	return task->rcv_snd(task, bid, cnt);
}

static Uns mbq_bkreq(Uns tid, Uns cnt)
{
	struct dsptask *task = task_prop[tid]->dsptask;

	if (rcvtyp_gb(task->ttyp) && (cnt > _ipbuf_sz))
		return MBCMD_EID_BADCNT;
	if (!sndtyp_bk(task->ttyp) || !sndtyp_psv(task->ttyp))
		/* This task doesn't receive BKREQ. */
		return MBCMD_EID_BADTCN;
	return task->rcv_req(task, cnt);
}

static Uns mbq_bksndp(Uns tid)
{
	struct dsptask *task = task_prop[tid]->dsptask;
	struct ipbuf_p *ipbuf_p = &task_prop[tid]->ipbuf_p_req;
	Uns cnt;

	if (!rcvtyp_bk(task->ttyp) || !rcvtyp_pv(task->ttyp))
		/* This task doesn't use private block transfer. */
		return MBCMD_EID_BADTCN;

	sync_with_arm(&ipbuf_p->s, tid);
	/*
	 * We are not going to lock this buffer any longer.
	 * If you need lock operation, you should use
	 * active receiving function. (bkreqp)
	 */
	ipbuf_p->s = MBCMD_TID_FREE;

	cnt = ipbuf_p->c;
	return task->rcv_snd(task, cnt);
}

static Uns generic_tctl(Uns ctlcmd)
{
	switch (ctlcmd) {
	case MBCMD_TCTL_TINIT:
		return 0;
	case MBCMD_TCTL_TCLR:
		return 0;
	case MBCMD_TCTL_TKILL:
		return 0;
	case MBCMD_TCTL_TEN:
	case MBCMD_TCTL_TDIS:
	default:
		return MBCMD_EID_BADTCTL;
	}
}

static Uns mbq_tctl(Uns tid, Uns ctlcmd)
{
	struct task_prop *prop = task_prop[tid];
	struct dsptask *task = prop->dsptask;
	Uns eid = MBCMD_EID_BADTCTL;
	Uns arg = 0;
	Uns ret = 0;

	if (((ctlcmd >= 0x8100) && (ctlcmd < 0x8200)) ||
	    ((ctlcmd >= 0x9100) && (ctlcmd < 0x9200))) {
		/* This TCTL has 1 arg */
		Uns cmd;
		get_from_mbq(prop->mbq, &prop->mbq_rp, prop->sem, &cmd, &arg);
	}

	if (task->rcv_tctl)
		eid = task->rcv_tctl(task, ctlcmd, &ret, arg);
	if (eid == MBCMD_EID_BADTCTL)
		eid = generic_tctl(ctlcmd);

	if ((ctlcmd >= 0x9000) && (ctlcmd < 0x9200))
		/* This TCTL is interactive */
		mbsend(mbcmd(MBCMD_TCTL, tid), ret);

	return eid;
}

static Uns mbq_tcfg(Uns tid)
{
	struct task_prop *prop = task_prop[tid];
	struct dsptask *task = prop->dsptask;
	Uns ttyp = task->ttyp;
	LgUns val;

	lock_ipbuf_sys_da();

	ipbuf_sys_da.d[0] = ttyp;

	val = sndtyp_pv(ttyp) ? (LgUns)&prop->ipbuf_p_snd : 0;
	ipbuf_sys_da.d[1] = val >> 16;
	ipbuf_sys_da.d[2] = val & 0xffff;

	val = rcvtyp_pv(ttyp) ? (LgUns)&prop->ipbuf_p_req : 0;
	ipbuf_sys_da.d[3] = val >> 16;
	ipbuf_sys_da.d[4] = val & 0xffff;

	val = (task->mmap_info) ? (LgUns)task->mmap_info->start : 0;
	ipbuf_sys_da.d[5] = val >> 16;
	ipbuf_sys_da.d[6] = val & 0xffff;

	val = (task->mmap_info) ? task->mmap_info->length : 0;
	ipbuf_sys_da.d[7] = val >> 16;
	ipbuf_sys_da.d[8] = val & 0xffff;

	ipbuf_sys_da.d[9]  = ((LgUns)task->name) >> 16;
	ipbuf_sys_da.d[10] = ((LgUns)task->name) & 0xffff;

	ipbuf_sys_da.s = tid;

	mbsend(mbcmd(MBCMD_TCFG, tid), 0);
	unlock_ipbuf_sys_da();

	return 0;
}

static Void do_mbq(Uns tid, Uns cmd_h, Uns data)
{
	Uns eid = 0;

	switch (cmd_h) {
	case MBCMD_WDSND:
		eid = mbq_wdsnd(tid, data);
		break;

	case MBCMD_WDREQ:
		eid = mbq_wdreq(tid);
		break;

	case MBCMD_BKSND:
		eid = mbq_bksnd(tid, data);
		break;

	case MBCMD_BKREQ:
		eid = mbq_bkreq(tid, data);
		break;

	case MBCMD_BKSNDP:
		eid = mbq_bksndp(tid);
		break;

	case MBCMD_TCTL:
		eid = mbq_tctl(tid, data);
		break;

	case MBCMD_POLL:
		poll_clear(tid, data);
		break;

	case MBCMD_TCFG:
		eid = mbq_tcfg(tid);
		break;

	case MBCMD_TDEL:
		register_super_mbq(MBCMD_TDEL, tid, MBCMD_TDEL_SAFE);
		/* never returns! */
		break;

	case MBCMD_TSTOP:
		TSK_exit();

	case MBCMD_BKREQP:
	default:
		eid = MBCMD_EID_BADCMD;
	}

	if (eid)
		sys_cmderr(eid, mbcmd(cmd_h, tid));
}

/*
 * mailbox process task
 */
static Void get_from_mbq(struct mailboxq *mbq, Uns *rp, SEM_Handle sem,
			 Uns *cmd, Uns *data)
{
	SEM_pend(sem, SYS_FOREVER);	/* wait for next one */
	*cmd  = mbq[*rp].cmd_h;
	*data = mbq[*rp].data;
	if (++*rp == MBQ_MAX)
		*rp = 0;
}

static Void procmb(Arg tid_arg)
{
	Uns tid = (Uns)ArgToInt(tid_arg);
	struct task_prop *prop = task_prop[tid];
	Uns cmd, data;

	for (;;) {
		get_from_mbq(prop->mbq, &prop->mbq_rp, prop->sem, &cmd, &data);
		do_mbq(tid, cmd, data);
		prop->poll_exclude &= ~POLL_EXCLUDE_TEMPORARY;
	}
}

/*
 * registering command to mbq.
 */
Int register_mbq(Uns tid, Uns cmd_h, Uns data)
{
	struct task_prop *prop;
	struct mailboxq *mbq_ent;
	Uns wp, newwp;

	if ((tid >= N_TASK_MAX) || (task_prop[tid] == NULL))
		return MBCMD_EID_BADTID;

	prop = task_prop[tid];
	if (prop->stat == TASK_STAT_STOP)
		return MBCMD_EID_TASKBSY;
	wp = prop->mbq_wp;
	mbq_ent = &prop->mbq[wp];
	mbq_ent->cmd_h = cmd_h;
	mbq_ent->data  = data;

	if ((newwp = wp+1) == MBQ_MAX)
		newwp = 0;
	if (newwp == prop->mbq_rp) {	/* mbq is full! */
		mbq_ent->cmd_h = MBCMD_TSTOP;	/* stop this task */
		prop->stat = TASK_STAT_STOP;
		return MBCMD_EID_TASKBSY;
	}
	prop->mbq_wp = newwp;

	SEM_post(prop->sem);
	return 0;
}

extern struct dsptask task_user[];
extern struct dsptask task_user_end;

Uns init_tasks(Void)
{
	struct task_prop *prop_buf;
	Uns ret;
	Uns i;

	memset(task_prop, 0, sizeof(Void *) * N_TASK_MAX);

	n_task = ((LgUns)&task_user_end - (LgUns)task_user) / sizeof(struct dsptask);
	if (n_task > N_TASK_MAX)
		n_task = N_TASK_MAX;

	if (n_task > 0) {
		Uns allocsz = sizeof(struct task_prop) * n_task;

		/* allocate all at once, and never be released. */
		prop_buf = MEM_alloc(MEM->MALLOCSEG, allocsz, 2);
		if (prop_buf == MEM_ILLEGAL) {
			sys_cmderr(MBCMD_EID_NOMEM, MBCMD_TID_ANON);
			return MBCMD_EID_NOMEM;
		}
	}

	// TSK attrs default value
	attrs_default.priority     = DSPTASK_DEFAULT_PRIORITY;
	attrs_default.stack        = NULL;
	attrs_default.stacksize    = DSPTASK_DEFAULT_STACKSIZE;
	attrs_default.sysstacksize = DSPTASK_DEFAULT_SYSSTACKSIZE;
	attrs_default.stackseg     = MEM->MALLOCSEG;
	attrs_default.environ      = NULL;
	attrs_default.name         = NULL;
	attrs_default.exitflag     = TRUE;

	/*
	 * user task initialization
	 */
	for (i = 0; i < n_task; i++) {
		if ((ret = task_config(&task_user[i], &prop_buf[i], i)) != 0) {
			sys_cmderr(ret, i);
			return ret;
		}
	}

	/*
	 * anonymous task initialization
	 */
	task_anon.tid = MBCMD_TID_ANON;

	return 0;
}

static Uns badfunc(struct dsptask *task)
{
	dbg(task, "DSP: task %s: function not implemented!\n", task->name);
	return MBCMD_EID_TASKERR;
}

Uns task_config(struct dsptask *task, struct task_prop *prop, Uns tid)
{
	struct TSK_Attrs *attrs;

	if (task->tid != TID_MAGIC)
		return MBCMD_EID_BADTID;

	task_prop[tid] = prop;
	prop->dsptask = task;
	task->tid = tid;
	if (task->rcv_snd == NULL)
		task->rcv_snd = badfunc;
	if (task->rcv_req == NULL)
		task->rcv_req = badfunc;
	if (task->tsk_attrs == NULL)
		attrs = &attrs_default;
	else {
		attrs = task->tsk_attrs;
		attrs->stackseg = MEM->MALLOCSEG;
	}
	attrs->name = task->name;
	attrs->environ = prop;
	prop->ipbuf_p_snd.s = MBCMD_TID_FREE;
	prop->ipbuf_p_req.s = MBCMD_TID_FREE;
	prop->stat = TASK_STAT_RUNNING;
	prop->mbq_wp = prop->mbq_rp = 0;
	prop->poll_exclude = 0;
	prop->sem = SEM_create(0, NULL);
	prop->tsk = TSK_create((Fxn)procmb, attrs, (Arg)tid);
	if ((prop->tsk == NULL) || (prop->sem == NULL)) {
		if (prop->tsk != NULL)
			TSK_delete(prop->tsk);
		if (prop->sem != NULL)
			SEM_delete(prop->sem);
		return MBCMD_EID_NOMEM;
	}

	return 0;
}

Void task_unconfig(Uns tid)
{
	struct task_prop *prop = task_prop[tid];

	unregister_tq_1s(prop->dsptask, NULL);	/* clear all functions in the queue */
	TSK_delete(prop->tsk);
	SEM_delete(prop->sem);
	task_prop[tid] = NULL;
	prop->dsptask->tid = TID_MAGIC;
}

asm(" .sect \"dspgw_task\"");	/* dummy section */
