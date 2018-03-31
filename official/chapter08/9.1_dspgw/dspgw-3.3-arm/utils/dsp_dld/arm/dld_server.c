/*
 * dsp_dld/arm/dld_server.c
 *
 * DSP Dynamic Loader Daemon: dld_server.c
 *
 * Copyright (C) 2003-2005 Nokia Corporation
 *
 * Written by Toshihiro Kobayashi <toshihiro.kobayashi@nokia.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 * 2005/07/06:  DSP Gateway version 3.3
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <malloc.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/ioctl.h>
#include <sys/wait.h>
#include <asm/arch/dsp.h>
#include "dsp_dld.h"
#include "dld_daemon.h"
#include "dld_coff.h"
#include "dld_cmd.h"
#include "dld_taskent.h"
#include "dld_memmgr.h"
#include "dld_symbol.h"
#include "dld_section.h"

#ifdef USE_FORK
#define MAX_CHILDREN	10
#endif

struct server_fds {
	fd_set set;
	int fd_twch;
	int fd_sock;
	int fd_acc;
	int fd_err;
#ifdef USE_FORK
	int fd_pipe[MAX_CHILDREN];
#endif
};
static struct server_fds server_fds;

#ifndef DSP_EMULATION
extern int dev_mklink(int n);
extern int dev_unlink(int n);
#endif
extern u32 task_load(struct taskent *te);
extern int task_clear(struct taskent *te);

static int signal_received = 0;
static server_return_t signal_action;
#ifndef DSP_EMULATION
static int suspend = 0;
#endif

#ifndef DSP_EMULATION
#define TID_MAX	254
static long tstat_prev[TID_MAX];
#endif

extern unsigned long binary_version;

/*
 * event packet operation
 */
static int read_event(int fd, struct server_event *e, size_t maxsz)
{
	if (read(fd, e, SERVER_EVENT_HDRSZ) < 0) {
		prmsg("failed to read event header\n");
		return -1;
	}
	if (e->len > SERVER_EVENT_HDRSZ) {
		if (e->len > maxsz) {
			prmsg("event packet size too large (%d)\n", e->len);
			return -1;
		}
		if (read(fd, &e->data, e->len - SERVER_EVENT_HDRSZ) < 0) {
			prmsg("failed to read event body\n");
			return -1;
		}
	}

	return 0;
}

static void sendback_event(int fd, u32 event)
{
	struct server_event e_sb;

	e_sb.len = sizeof(struct server_event);
	e_sb.event = event;
	write(fd, &e_sb, e_sb.len);
}

#ifdef USE_FORK
/*
 * parent-child communication
 */
static int pipe_alloc(void)
{
	int i;
	int pp[2];

	for (i = 0; i < MAX_CHILDREN; i++) {
		if (server_fds.fd_pipe[i] < 0) {
			pipe(pp);
			server_fds.fd_pipe[i] = pp[0];
			FD_SET(pp[0], &server_fds.set);
			return pp[1];
		}
	}
	/* failed */
	return -1;
}

static int read_pipe(int fd)
{
	char buf[256];
	struct server_event *e = (struct server_event *)buf;
	struct taskent *te;
	u8 minor;
	int ret;

	if (read_event(fd, e, 256) < 0) {
		prmsg("failed to read event packet from pipe\n");
		return -1;
	}

	switch (e->event) {
		case DLD_EVENT_TADD_DONE:
#ifndef DSP_EMULATION
			minor = e->data.task.minor;
			te = taskent_find_by_minor(minor);
			if (--te->cobj->request_lock > 0) {
				prmsg("there're some waiting request.\n");
				te->cobj->request_lock = 0;
				return taskent_process_request_all();
			}
#endif
			return 0;

		case DLD_EVENT_TDEL_DONE:
		case DLD_EVENT_TKILL_DONE:
			minor = e->data.task.minor;
			te = taskent_find_by_minor(minor);
			ret = task_clear(te);
#ifndef DSP_EMULATION
			if (--te->cobj->request_lock > 0) {
				prmsg("there're some waiting request.\n");
				te->cobj->request_lock = 0;
				return taskent_process_request_all();
			}
#endif
			return ret;

		default:
			prmsg("unknown event from pipe: %d\n", e->event);
			return -1;
	}
}
#endif /* USE_FORK */

/*
 * fork for tadd, tdel, tkill
 */
static pid_t fork_task_add(u8 minor, u32 taskadr)
{
#ifndef DSP_EMULATION
	struct omap_dsp_taddinfo taddinfo;
#endif
#ifdef USE_FORK
	struct server_event e;
	int fd_pipe;
	pid_t pid;

	if ((fd_pipe = pipe_alloc()) < 0) {
		prmsg("pipe alloc failed\n");
		return -1;
	}
	if ((pid = fork()) < 0) {
		prmsg("fork failed\n");
		return -1;
	}
	if (pid > 0) {	/* parent */
		close(fd_pipe);
		return pid;
	}
#endif

	/*
	 * child:
	 * theoretically we should close pp[0] and other server fds.
	 * (but not done :p)
	 */
#ifndef DSP_EMULATION
	taddinfo.minor = minor;
	taddinfo.taskadr = taskadr;
	ioctl(server_fds.fd_twch, OMAP_DSP_TWCH_IOCTL_TADD, &taddinfo);
#endif

#ifdef USE_FORK
	e.len = sizeof(struct server_event);
	e.event = DLD_EVENT_TADD_DONE;
	e.data.task.minor = minor;
	write(fd_pipe, &e, e.len);
	close(fd_pipe);
#endif
	return 0;
}

static pid_t fork_task_del(u8 minor)
{
#ifdef USE_FORK
	struct server_event e;
	int fd_pipe;
	pid_t pid;

	if ((fd_pipe = pipe_alloc()) < 0) {
		prmsg("pipe alloc failed\n");
		return -1;
	}
	if ((pid = fork()) < 0) {
		prmsg("fork failed\n");
		return -1;
	}
	if (pid > 0) {	/* parent */
		close(fd_pipe);
		return pid;
	}
#endif

	/*
	 * child:
	 * theoretically we should close pp[0] and other server fds.
	 * (but not done :p)
	 */
#ifndef DSP_EMULATION
	ioctl(server_fds.fd_twch, OMAP_DSP_TWCH_IOCTL_TDEL, minor);
#endif
#ifdef USE_FORK
	e.len = sizeof(struct server_event);
	e.event = DLD_EVENT_TDEL_DONE;
	e.data.task.minor = minor;
	write(fd_pipe, &e, e.len);
	close(fd_pipe);
#endif
	return 0;
}

static pid_t fork_task_kill(u8 minor)
{
#ifdef USE_FORK
	struct server_event e;
	int fd_pipe;
	pid_t pid;

	if ((fd_pipe = pipe_alloc()) < 0) {
		prmsg("pipe alloc failed\n");
		return -1;
	}
	if ((pid = fork()) < 0) {
		prmsg("fork failed\n");
		return -1;
	}
	if (pid > 0) {	/* parent */
		close(fd_pipe);
		return pid;
	}
#endif

	/*
	 * child:
	 * theoretically we should close pp[0] and other server fds.
	 * (but not done :p)
	 */
#ifndef DSP_EMULATION
	ioctl(server_fds.fd_twch, OMAP_DSP_TWCH_IOCTL_TKILL, minor);
#endif
#ifdef USE_FORK
	e.len = sizeof(struct server_event);
	e.event = DLD_EVENT_TKILL_DONE;
	e.data.task.minor = minor;
	write(fd_pipe, &e, e.len);
	close(fd_pipe);
#endif
	return 0;
}

/*
 *
 */
static int server_tadd(struct server_event *e)
{
	u8 minor = e->data.task.minor;
	struct taskent *te = taskent_find_by_minor(minor);
	int fd = server_fds.fd_acc;
	u32 taskadr;
	pid_t pid;

	if (te == NULL)
		goto fail;
	taskadr = task_load(te);
	if (taskadr == OMAP_DSP_TADD_ABORTADR)
		/*
		 * in this case we don't need to process
		 * fork_task_add() with ABORTADR.
		 */
		goto fail;

#ifdef USE_FORK
	if (te->cobj->request_lock) {
		prmsg("device %s is locked. rejecting TADD request.\n",
		      te->devname);
		goto fail;
	}
	te->cobj->request_lock++;
#endif

	pid = fork_task_add(minor, taskadr);
	if (pid < 0) {
		sendback_event(fd, DLD_EVENT_ERROR);
		exit(1);
	}
#ifdef USE_FORK
	if (pid == 0) {	/* child */
		sendback_event(fd, DLD_EVENT_DONE);
		exit(0);
	}
#else
	sendback_event(fd, DLD_EVENT_DONE);
#endif
	/* parent */
	return 0;

fail:
	sendback_event(fd, DLD_EVENT_ERROR);
	return 1;	/* not fatail */
}

static int server_tdel(struct server_event *e)
{
	u8 minor = e->data.task.minor;
	struct taskent *te = taskent_find_by_minor(minor);
	int fd = server_fds.fd_acc;
	pid_t pid;

	if (te == NULL)
		goto fail;

#ifdef USE_FORK
	if (te->cobj->request_lock) {
		prmsg("device %s is locked. rejecting TDEL request.\n",
		      te->devname);
		goto fail;
	}
	te->cobj->request_lock++;
#endif

	pid = fork_task_del(minor);
	if (pid < 0) {
		sendback_event(fd, DLD_EVENT_ERROR);
		exit(1);
	}
#ifdef USE_FORK
	if (pid == 0) {	/* child */
		/*
		 * task_clear() will be done when
		 * the server received TDEL_DONE event.
		 */
		sendback_event(fd, DLD_EVENT_DONE);
		exit(0);
	}
	/* parent */
	return 0;
#else
	sendback_event(fd, DLD_EVENT_DONE);
	return task_clear(te);
#endif

fail:
	sendback_event(fd, DLD_EVENT_ERROR);
	return 1;	/* not fatal */
}

static int server_tkill(struct server_event *e)
{
	u8 minor = e->data.task.minor;
	struct taskent *te = taskent_find_by_minor(minor);
	int fd = server_fds.fd_acc;
	pid_t pid;

	if ((te == NULL) || (te->cobj->usecount == 0))
		goto fail;

#ifdef USE_FORK
	if (te->cobj->request_lock) {
		prmsg("device %s is locked. rejecting TKILL request.\n",
		      te->devname);
		goto fail;
	}
	te->cobj->request_lock++;
#endif

	pid = fork_task_kill(minor);
	if (pid < 0) {
		sendback_event(fd, DLD_EVENT_ERROR);
		exit(1);
	}
#ifdef USE_FORK
	if (pid == 0) {	/* child */
		/*
		 * task_clear() will be done when
		 * the server received TKILL_DONE event.
		 */
		sendback_event(fd, DLD_EVENT_DONE);
		exit(0);
	}
	/* parent */
	return 0;
#else
	sendback_event(fd, DLD_EVENT_DONE);
	return task_clear(te);
#endif

fail:
	sendback_event(fd, DLD_EVENT_ERROR);
	return 1;	/* not fatal */
}

static int server_sendstat_memmgr(int fd)
{
	memmgr_sendstat(NULL, fd);
	taskent_mem_sendstat(fd);
	sendback_event(fd, DLD_EVENT_DONE);
	return 0;
}

extern struct dld_conf dld_conf;

static int server_sendstat_symbol(int fd)
{
#ifdef STICKY_LIST
	symbol_sendstat(NULL, fd);
#else
	struct coffobj *knl_cobj = coff_new(dld_conf.knlfn);
	if (coff_read_kernel(knl_cobj) < 0) {
		sendback_event(fd, DLD_EVENT_ERROR);
		return -1;
	}
	symbol_sendstat(&knl_cobj->symlist, fd);
	coff_free(knl_cobj);
#endif
	taskent_sym_sendstat(fd);
	sendback_event(fd, DLD_EVENT_DONE);
	return 0;
}

static int server_sendstat_section(int fd)
{
#ifdef STICKY_LIST
	section_sendstat(NULL, fd);
#else
	struct coffobj *knl_cobj = coff_new(dld_conf.knlfn);
	if (coff_read_kernel(knl_cobj) < 0) {
		sendback_event(fd, DLD_EVENT_ERROR);
		return -1;
	}
	section_sendstat(&knl_cobj->scnlist, fd);
	coff_free(knl_cobj);
#endif
	taskent_scn_sendstat(fd);
	sendback_event(fd, DLD_EVENT_DONE);
	return 0;
}

static int server_memdump(int fd, struct server_event *e)
{
	u32 addr = e->data.memdump.addr;
	u32 size = e->data.memdump.size;
	char buf[256];
	struct server_event *e1 = (struct server_event *)buf;
	size_t strsz;
	int cnt;
	u32 i;
	u8 *p;

#ifdef DSP_EMULATION

	struct memmgr *mem;

	mem = memmgr_find_by_addr(NULL, addr, size);
	if (mem == NULL) {
		sendback_event(fd, DLD_EVENT_ERROR);
		return -1;
	}

	p = &mem->img[addr - mem->base];

#else

	int space;
	int devfd;
	char *dbuf;

	space = space_find_by_addr(addr, size);
	if (space == SPACE_CROSSING) {
		prmsg("section crossing memory boundary!\n");
		return -1;
	}

	if ((devfd = open(DEVNAME_DSPMEM, O_RDWR)) < 0) {
		prmsg("Can't open %s\n", DEVNAME_DSPMEM);
		return -1;
	}
	lseek(devfd, addr, SEEK_SET);
	if ((dbuf = malloc(size)) == NULL) {
		prmsg("Can't alloc memory at %s line %d\n", __FILE__, __LINE__);
		close(devfd);
		return -1;
	}
	read(devfd, dbuf, size);
	close(devfd);

	p = dbuf;

#endif

	e1->event = DLD_EVENT_STRING;

	cnt = 0;
	for (i = 0; i < size; i++, p++) {
		strsz = 256 - SERVER_EVENT_HDRSZ - cnt;
		if (i % 16 == 15) {
			cnt += snprintf(&e1->data.s[cnt], strsz, " %02x\n", *p);
			e1->len = SERVER_EVENT_HDRSZ + cnt;
			write(fd, e1, e1->len);
			cnt = 0;
		} else {
			cnt += snprintf(&e1->data.s[cnt], strsz, " %02x", *p);
		}
	}
	if (cnt > 0) {
		cnt += snprintf(&e1->data.s[cnt], strsz, " \n");
		e1->len = SERVER_EVENT_HDRSZ + cnt;
		write(fd, e1, e1->len);
	}

	e->event = DLD_EVENT_DONE;
	e->len = SERVER_EVENT_HDRSZ;
	write(fd, e, e->len);

#ifndef DSP_EMULATION
	free(dbuf);
#endif
	return 0;
}

#ifndef DSP_EMULATION
static int server_dsp_run(int fd)
{
	int cfd;

	if ((cfd = open(DEVNAME_CONTROL, O_RDWR)) < 0) {
		prmsg("%s open failed at %s line %d\n",
		      DEVNAME_CONTROL, __FILE__, __LINE__);
		sendback_event(fd, DLD_EVENT_ERROR);
		return 1;
	}
	prmsg("releasing DSP reset\n");
	ioctl(cfd, OMAP_DSP_IOCTL_RUN);
	close(cfd);

	sendback_event(fd, DLD_EVENT_DONE);
	return 0;
}

static int server_dsp_reset(int fd)
{
	int cfd;

	if ((cfd = open(DEVNAME_CONTROL, O_RDWR)) < 0) {
		prmsg("%s open failed at %s line %d\n",
		      DEVNAME_CONTROL, __FILE__, __LINE__);
		sendback_event(fd, DLD_EVENT_ERROR);
		return 1;
	}
	prmsg("DSP reset\n");
	ioctl(cfd, OMAP_DSP_IOCTL_RESET);
	close(cfd);

	sendback_event(fd, DLD_EVENT_DONE);
	return 0;
}

static int server_setrstvect(int fd, struct server_event *e)
{
	u32 addr = e->data.rstvect.addr;
	int cfd;

	if ((cfd = open(DEVNAME_CONTROL, O_RDWR)) < 0) {
		prmsg("%s open failed at %s line %d\n",
		      DEVNAME_CONTROL, __FILE__, __LINE__);
		sendback_event(fd, DLD_EVENT_ERROR);
		return 1;
	}
	prmsg("setting DSP reset vector to 0x%06lx\n", addr);
	ioctl(cfd, OMAP_DSP_IOCTL_SETRSTVECT, addr);
	close(cfd);

	sendback_event(fd, DLD_EVENT_DONE);
	return 0;
}

static int server_cpu_idle(int fd)
{
	int cfd;

	if ((cfd = open(DEVNAME_CONTROL, O_RDWR)) < 0) {
		prmsg("%s open failed at %s line %d\n",
		      DEVNAME_CONTROL, __FILE__, __LINE__);
		sendback_event(fd, DLD_EVENT_ERROR);
		return 1;
	}
	prmsg("setting DSP idle\n");
	ioctl(cfd, OMAP_DSP_IOCTL_CPU_IDLE);
	close(cfd);

	sendback_event(fd, DLD_EVENT_DONE);
	return 0;
}

static int server_dspconfig(int fd)
{
	int cfd;
	int n_task;

	if ((cfd = open(DEVNAME_CONTROL, O_RDWR)) < 0) {
		prmsg("%s open failed at %s line %d\n",
		      DEVNAME_CONTROL, __FILE__, __LINE__);
		sendback_event(fd, DLD_EVENT_ERROR);
		return 1;
	}
	prmsg("DSP configuration ...\n");
	if (ioctl(cfd, OMAP_DSP_IOCTL_DSPCFG) < 0) {
		prmsg("  failed");
		sendback_event(fd, DLD_EVENT_ERROR);
		close(cfd);
		return 1;
	}
	prmsg("  succeeded.");

	if ((n_task = ioctl(cfd, OMAP_DSP_IOCTL_TASKCNT)) < 0) {
		prmsg("TASKCNT failed at %s line %d\n",
		      DEVNAME_CONTROL, __FILE__, __LINE__);
		sendback_event(fd, DLD_EVENT_ERROR);
		close(cfd);
		return 1;
	}
	close(cfd);

	if (dev_mklink(n_task) < 0) {
		sendback_event(fd, DLD_EVENT_ERROR);
		return 1;
	}

	sendback_event(fd, DLD_EVENT_DONE);
	return 0;
}

static int server_dspunconfig(int fd)
{
	int cfd;
	int n_task;
	int status = 0;

	if ((cfd = open(DEVNAME_CONTROL, O_RDWR)) < 0) {
		prmsg("%s open failed at %s line %d\n",
		      DEVNAME_CONTROL, __FILE__, __LINE__);
		sendback_event(fd, DLD_EVENT_ERROR);
		return 1;
	}

	if ((n_task = ioctl(fd, OMAP_DSP_IOCTL_TASKCNT)) < 0) {
		prmsg("TASKCNT failed at %s line %d\n",
		      DEVNAME_CONTROL, __FILE__, __LINE__);
		status = -1;
	} else
		status = dev_unlink(n_task);

	prmsg("releasing resources for DSP\n");
	ioctl(cfd, OMAP_DSP_IOCTL_DSPUNCFG);
	close(cfd);

	if (status < 0)
		sendback_event(fd, DLD_EVENT_ERROR);
	else
		sendback_event(fd, DLD_EVENT_DONE);
	return 0;
}

static int server_suspend(int fd)
{
	int cfd;
	int ret;

	if ((cfd = open(DEVNAME_CONTROL, O_RDWR)) < 0) {
		prmsg("%s open failed at %s line %d\n",
		      DEVNAME_CONTROL, __FILE__, __LINE__);
		sendback_event(fd, DLD_EVENT_ERROR);
		return 1;
	}
	prmsg("suspending DSP\n");
	ret = ioctl(cfd, OMAP_DSP_IOCTL_SUSPEND);
	close(cfd);

	if (ret < 0) {
		prmsg("SUSPEND failed\n");
		sendback_event(fd, DLD_EVENT_ERROR);
		return 1;
	}

	sendback_event(fd, DLD_EVENT_DONE);
	suspend = 1;
	return 0;
}

static int server_resume(int fd)
{
	int cfd;
	int ret;

	suspend = 0;

	if ((cfd = open(DEVNAME_CONTROL, O_RDWR)) < 0) {
		prmsg("%s open failed at %s line %d\n",
		      DEVNAME_CONTROL, __FILE__, __LINE__);
		sendback_event(fd, DLD_EVENT_ERROR);
		return 1;
	}
	prmsg("resuming DSP\n");
	ret = ioctl(cfd, OMAP_DSP_IOCTL_RESUME);
	close(cfd);

	if (ret < 0) {
		prmsg("RESUME failed\n");
		sendback_event(fd, DLD_EVENT_ERROR);
		return 1;
	}

	sendback_event(fd, DLD_EVENT_DONE);
	return 0;
}

static int server_exmap(int fd, struct server_event *e)
{
	struct omap_dsp_mapinfo mapinfo = { e->data.exmap.dspadr,
					    e->data.exmap.size };
	int ret;
	int cfd;

	if ((cfd = open(DEVNAME_DSPMEM, O_RDWR)) < 0) {
		prmsg("%s open failed at %s line %d\n",
		      DEVNAME_CONTROL, __FILE__, __LINE__);
		sendback_event(fd, DLD_EVENT_ERROR);
		return 1;
	}
	prmsg("mapping external memory for DSP: "
	      "dspadr = 0x%06lx, size = 0x%lx\n",
	      mapinfo.dspadr, mapinfo.size);
	ret = ioctl(cfd, OMAP_DSP_MEM_IOCTL_EXMAP, &mapinfo);
	close(cfd);

	if (ret < 0) {
		prmsg("EXMAP failed\n");
		sendback_event(fd, DLD_EVENT_ERROR);
		return 1;
	}

	prmsg("%d bytes mapped.\n", ret);
	sendback_event(fd, DLD_EVENT_DONE);

	return 0;
}

static int server_mmuinit(int fd)
{
	int cfd;

	if ((cfd = open(DEVNAME_DSPMEM, O_RDWR)) < 0) {
		prmsg("%s open failed at %s line %d\n",
		      DEVNAME_CONTROL, __FILE__, __LINE__);
		sendback_event(fd, DLD_EVENT_ERROR);
		return 1;
	}
	prmsg("DSP MMU initialize.\n");
	ioctl(cfd, OMAP_DSP_MEM_IOCTL_MMUINIT);
	close(cfd);

	sendback_event(fd, DLD_EVENT_DONE);
	return 0;
}

static int server_mapflush(int fd)
{
	int cfd;

	if ((cfd = open(DEVNAME_DSPMEM, O_RDWR)) < 0) {
		prmsg("%s open failed at %s line %d\n",
		      DEVNAME_CONTROL, __FILE__, __LINE__);
		sendback_event(fd, DLD_EVENT_ERROR);
		return 1;
	}
	prmsg("DSP TLB has been flushed.\n");
	ioctl(cfd, OMAP_DSP_MEM_IOCTL_EXMAP_FLUSH);
	close(cfd);

	sendback_event(fd, DLD_EVENT_DONE);
	return 0;
}

static int server_mbsend(int fd, struct server_event *e)
{
	struct omap_dsp_mailbox_cmd mbcmd = { e->data.mbsend.cmd,
					      e->data.mbsend.data };
	int cfd;

	if ((cfd = open(DEVNAME_CONTROL, O_RDWR)) < 0) {
		prmsg("%s open failed at %s line %d\n",
		      DEVNAME_CONTROL, __FILE__, __LINE__);
		sendback_event(fd, DLD_EVENT_ERROR);
		return 1;
	}
	prmsg("sending mailbox command to DSP: cmd = 0x%02x, data = 0x%02x\n",
	      mbcmd.cmd, mbcmd.data);

	ioctl(cfd, OMAP_DSP_IOCTL_MBSEND, &mbcmd);
	close(cfd);

	sendback_event(fd, DLD_EVENT_DONE);
	return 0;
}
#endif /* DSP_EMULATION */

#ifndef DSP_EMULATION
/*
 *
 */
int twch_tadd(struct taskent *te)
{
	u32 taskadr;
	pid_t pid;

#ifdef USE_FORK
	if (te->cobj->request_lock++) {
		prmsg("device %s is locked. delaying TADD request.\n",
		      te->devname);
		return 0;
	}
#endif
	/*
	 * clear request so that the server detect next request.
	 */
	te->request = TREQ_NONE;

	prmsg("starting TADD process for device %s.\n", te->devname);
	taskadr = task_load(te);
	pid = fork_task_add(te->minor, taskadr);
	if (pid < 0)
		exit(1);
#ifdef USE_FORK
	if (pid == 0)	/* child */
		exit(0);
#endif
	/* parent */
	if (taskadr == OMAP_DSP_TADD_ABORTADR)
		return 1;

	return 0;
}

int twch_tdel(struct taskent *te)
{
	pid_t pid;

#ifdef USE_FORK
	if (te->cobj->request_lock++) {
		prmsg("device %s is locked. delaying TDEL request.\n",
		      te->devname);
		return 0;
	}
#endif
	/*
	 * clear request so that the server detect next request.
	 */
	te->request = TREQ_NONE;

	prmsg("starting TDEL process for device %s.\n", te->devname);
	pid = fork_task_del(te->minor);
	if (pid < 0)
		exit(1);
#ifdef USE_FORK
	if (pid == 0) {	/* child */
		/*
		 * task_clear() will be done when
		 * the server received TDEL_DONE event.
		 */
		exit(0);
	}
	/* parent */
	return 0;
#else
	return task_clear(te);
#endif
}

static int read_twch(void)
{
	long tstat[TID_MAX];
	struct taskent *te;
	size_t cnt;
	int devcnt;
	int i;

	if ((cnt = read(server_fds.fd_twch, tstat, TID_MAX * sizeof(long))) < 0) {
		prmsg("read from dsptwch failed\n");
		return -1;
	}
	devcnt = cnt / 4;
	prmsg("dsp_dld: event detected.\n");

	for (i = 0; i < devcnt; i++) {
		if ((te = taskent_find_by_minor(i)) == NULL)
			continue;
		switch (tstat[i]) {
			/* if STALE bit is set, it means the request has
			 * accepted already, and we ignore it. */
			case OMAP_DSP_DEVSTATE_ADDREQ:
				prmsg("device %s is requesting for TADD.\n",
				      te->devname);
				te->request = TREQ_ADD;
				break;
			case OMAP_DSP_DEVSTATE_DELREQ:
				prmsg("device %s is requesting for TDEL.\n",
				      te->devname);
				te->request = TREQ_DEL;
				break;
		}
	}

	memcpy(tstat_prev, tstat, devcnt * sizeof(long));
	return taskent_process_request_all();
}

static server_return_t read_errdt(int fd)
{
	unsigned long errcode;
	size_t cnt;

	if ((cnt = read(fd, &errcode, 4)) < 0) {
		prmsg("read from dsperr failed\n");
		return SERVER_RESTART;
	}
	prmsg("dsp_dld: DSP error detected.\n");

	if (errcode & OMAP_DSP_ERRDT_WDT) {
		prmsg("  DSP watchdog timer expired.\n"
		      "  (some tasks may be in busy loop)\n");
		return SERVER_RESTART;
	}

	if (errcode & OMAP_DSP_ERRDT_MMU) {
#if 0 // recovery
		int fd_dspmem;

		prmsg("  DSP MMU fault. trying to recover...\n");
		if ((fd_dspmem = open(DEVNAME_DSPMEM, O_RDWR)) < 0) {
			prmsg("%s open failed\n", DEVNAME_DSPMEM);
			return SERVER_RESTART;
		}
		ioctl(fd_dspmem, OMAP_DSP_MEM_IOCTL_MMUITACK);
		close(fd_dspmem);
#else
		prmsg("  DSP MMU fault.\n");
		return SERVER_RESTART;
#endif
	}

	return SERVER_OK;
}
#endif /* !DSP_EMULATION */

static int server_open(char *name)
{
	int fd;
	struct sockaddr_un addr;

	if ((fd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) {
		prmsg("socket() failed at %s line %d\n", __FILE__, __LINE__);
		return -1;
	}

	if (!access(name, F_OK))
		unlink(name);

	addr.sun_family = AF_UNIX;
	strcpy(addr.sun_path, name);
	if (bind(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
		prmsg("bind() failed at %s line %d\n", __FILE__, __LINE__);
		return -1;
	}

	if (listen(fd, 1) < 0) {
		prmsg("listen() failed at %s line %d\n", __FILE__, __LINE__);
		return -1;
	}

	return fd;
}

static int server_close(int fd, char *name)
{
	close(fd);
	unlink(name);
	return 0;
}

static int server_accept(int fd)
{
	int fd_acc;
	struct sockaddr_un addr;
	int len = sizeof(struct sockaddr_un);

	if ((fd_acc = accept(fd, (struct sockaddr *)&addr, &len)) < 0) {
		prmsg("accept() failed at %s line %d\n", __FILE__, __LINE__);
		return -1;
	}
	return fd_acc;
}

static server_return_t server_read(void)
{
	char buf[256];
	struct server_event *e = (struct server_event *)buf;
	int fd = server_fds.fd_acc;
	int status = 0;

	if (read_event(fd, e, 256) < 0) {
		prmsg("failed to read event packet from socket\n");
		return SERVER_RESTART;
	}

	switch (e->event) {
		case DLD_EVENT_TADD:
			status = server_tadd(e);
			break;

		case DLD_EVENT_TDEL:
			status =  server_tdel(e);
			break;

		case DLD_EVENT_TKILL:
			status = server_tkill(e);
			break;

		case DLD_EVENT_GETSTAT_TASKENT:
			taskent_sendstat(fd);
			sendback_event(fd, DLD_EVENT_DONE);
			break;

		case DLD_EVENT_GETSTAT_MEMMGR:
			status = server_sendstat_memmgr(fd);
			break;

		case DLD_EVENT_GETSTAT_SYMBOL:
			status = server_sendstat_symbol(fd);
			break;

		case DLD_EVENT_GETSTAT_SECTION:
			status = server_sendstat_section(fd);
			break;

		case DLD_EVENT_MEMDUMP:
			status = server_memdump(fd, e);
			break;
		   
#ifndef DSP_EMULATION
		case DLD_EVENT_DSP_RUN:
			status = server_dsp_run(fd);
			break;

		case DLD_EVENT_DSP_RESET:
			status = server_dsp_reset(fd);
			break;

		case DLD_EVENT_RSTVECT:
			status = server_setrstvect(fd, e);
			break;

		case DLD_EVENT_CPU_IDLE:
			status = server_cpu_idle(fd);
			break;
		   
		case DLD_EVENT_DSPCFG:
			status = server_dspconfig(fd);
			break;
		   
		case DLD_EVENT_DSPUNCFG:
			status = server_dspunconfig(fd);
			break;
		   
		case DLD_EVENT_SUSPEND:
			status = server_suspend(fd);
			break;
		   
		case DLD_EVENT_RESUME:
			status = server_resume(fd);
			break;
		   
		case DLD_EVENT_EXMAP:
			status = server_exmap(fd, e);
			break;

		case DLD_EVENT_MMUINIT:
			status = server_mmuinit(fd);
			break;

		case DLD_EVENT_MAPFLUSH:
			status = server_mapflush(fd);
			break;

		case DLD_EVENT_MBSEND:
			status = server_mbsend(fd, e);
			break;
#endif /* DSP_EMULATION */

		case DLD_EVENT_TERMINATE:
			prmsg("Terminating...\n");
			sendback_event(fd, DLD_EVENT_DONE);
			return SERVER_DONE;

		case DLD_EVENT_RESTART:
			prmsg("Restarting...\n");
			sendback_event(fd, DLD_EVENT_DONE);
			return SERVER_RESTART;

		default:
			sendback_event(fd, DLD_EVENT_ERROR);
			prmsg("unknown event packet (%d) received on socket\n",
			      e->event);
			return SERVER_RESTART;
	}

	if (status < 0)
		return SERVER_RESTART;
	
	return SERVER_OK;
}

static server_return_t read_all_fds(fd_set testfds)
{
#ifdef USE_FORK
	int i;
#endif

	if (FD_ISSET(server_fds.fd_sock, &testfds)) {
		if ((server_fds.fd_acc = server_accept(server_fds.fd_sock)) < 0)
			return SERVER_RESTART;
		FD_SET(server_fds.fd_acc, &server_fds.set);
	}
	if ((server_fds.fd_acc >= 0) && FD_ISSET(server_fds.fd_acc, &testfds)) {
		server_return_t ret;

		ret = server_read();
		close(server_fds.fd_acc);
		FD_CLR(server_fds.fd_acc, &server_fds.set);
		server_fds.fd_acc = -1;
		return ret;
	}
#ifdef USE_FORK
	for (i = 0; i < MAX_CHILDREN; i++) {
		int fd = server_fds.fd_pipe[i];
		if ((fd >= 0) && FD_ISSET(fd, &testfds)) {
			int ret;

			ret = read_pipe(fd);
			close(fd);
			FD_CLR(fd, &server_fds.set);
			server_fds.fd_pipe[i] = -1;
			if (ret < 0)
				return SERVER_RESTART;
		}
	}
#endif
#ifndef DSP_EMULATION
	if (FD_ISSET(server_fds.fd_twch, &testfds)) {
		if (read_twch() < 0)
			return SERVER_RESTART;
		FD_SET(server_fds.fd_twch, &server_fds.set);
	}
	if (FD_ISSET(server_fds.fd_err, &testfds)) {
		server_return_t ret;
		if ((ret = read_errdt(server_fds.fd_err)) != SERVER_OK)
			return ret;
		FD_SET(server_fds.fd_err, &server_fds.set);
	}
#endif

	return SERVER_OK;
}

server_return_t server(void)
{
	server_return_t ret;
	struct timeval timeout;
#ifdef USE_FORK
	int i;
#endif

#ifndef DSP_EMULATION
	if ((server_fds.fd_twch = open(DEVNAME_TWCH, O_RDONLY)) < 0) {
		prmsg("%s open failed\n", DEVNAME_TWCH);
		return SERVER_ERROR;
	}
	if ((server_fds.fd_err = open(DEVNAME_ERR, O_RDONLY)) < 0) {
		prmsg("%s open failed\n", DEVNAME_ERR);
		return SERVER_ERROR;
	}
#endif

	if (taskent_mkdev(server_fds.fd_twch) < 0)
		return SERVER_ERROR;

	if ((server_fds.fd_sock = server_open(SOCK_NAME)) < 0)
		return SERVER_ERROR;

	FD_ZERO(&server_fds.set);
#ifndef DSP_EMULATION
	FD_SET(server_fds.fd_twch, &server_fds.set);
	FD_SET(server_fds.fd_err, &server_fds.set);
#endif
	FD_SET(server_fds.fd_sock, &server_fds.set);
	server_fds.fd_acc = -1;
#ifdef USE_FORK
	for (i = 0; i < MAX_CHILDREN; i++) {
		server_fds.fd_pipe[i] = -1;
	}
#endif /* USE_FORK */

	timeout.tv_sec  = 10;
	timeout.tv_usec = 0;
	for (;;) {
		fd_set testfds = server_fds.set;
		int sel_result;

		sel_result = select(FD_SETSIZE, &testfds, NULL, NULL, &timeout);
		if (signal_received) {
			if (signal_action == SERVER_OK) {
				signal_received = 0;
				continue;
			} else
				break;
		}

		if (sel_result < 0) {
			prmsg("select() failed at %s line %d\n",
			      __FILE__, __LINE__);
			ret = SERVER_ERROR;
			break;
		} else if (sel_result == 0) {	/* timeout */
#if !(defined DSP_EMULATION) && (defined ENABLE_POLL)
			int cfd;
			int status;

			if (suspend)
				goto poll_out;
			if (binary_version < 0x00030003)
				goto poll_out;

			/*
			 * we assume this program runs on Linux,
			 * so the timeout value is decreased within
			 * select(), therefore timeout occurs even if
			 * other events wakes select() up frequently.
			 */
			if ((cfd = open(DEVNAME_CONTROL, O_RDWR)) < 0) {
				prmsg("%s open failed at %s line %d\n",
				      DEVNAME_CONTROL, __FILE__, __LINE__);
				ret = SERVER_RESTART;
				break;
			}
			/* FIXME: fork this polling process */
			status = ioctl(cfd, OMAP_DSP_IOCTL_POLL);
			close(cfd);

			if (status < 0) {
				ret = SERVER_RESTART;
				break;
			}
poll_out:
#endif /* !(defined DSP_EMULATION) && (defined ENABLE_POLL) */
			timeout.tv_sec  = 10;
			timeout.tv_usec = 0;
		} else {
			if ((ret = read_all_fds(testfds)) != SERVER_OK)
				break;
		}
	}

	taskent_rmdev(server_fds.fd_twch);

	if (server_fds.fd_acc > 0)
		close(server_fds.fd_acc);
	server_close(server_fds.fd_sock, SOCK_NAME);
#ifndef DSP_EMULATION
	close(server_fds.fd_twch);
	close(server_fds.fd_err);
#endif

	/* signal overrides poll timeout or other error */
	if (signal_received)
		ret = signal_action;

	return ret;
}

#undef SIGHUP_RESTART

void dspsig_handler(int signum)
{
	prmsg("receiving signal %d\n", signum);
	signal_received = signum;

	if (signum == SIGCHLD) {
		pid_t pid;

		signal_action = SERVER_OK;
		do {
			pid = waitpid(-1, NULL, WNOHANG);
		} while (pid > 0);
		return;
	}

	if (signum == SIGKILL)
		exit(0);

	signal_action = SERVER_DONE;
#ifdef SIGHUP_RESTART
	if (signum == SIGHUP)
		signal_action = SERVER_RESTART;
#endif
}
