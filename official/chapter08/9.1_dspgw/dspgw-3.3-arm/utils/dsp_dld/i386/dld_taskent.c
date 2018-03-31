/*
 * dsp_dld/arm/dld_taskent.c
 *
 * DSP Dynamic Loader Daemon: dld_taskent.c
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
 * 2005/07/07:  DSP Gateway version 3.3
 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <malloc.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <asm/arch/dsp.h>
#include "list.h"
#include "dsp_dld.h"
#include "dld_malloc.h"
#include "dld_server.h"
#include "dld_daemon.h"
#include "dld_cmd.h"
#include "dld_coff.h"
#include "dld_memmgr.h"
#include "dld_symbol.h"
#include "dld_section.h"

#define g_taskent_for_each(pos)	taskent_for_each(pos, &g_tasklist)

static LIST_HEAD(g_tasklist);

static struct taskent *taskent_new(char *dnm, char *tnm, unsigned int pri,
				   char *ofn, char *cnm);
static struct taskent *taskent_find_by_coffname(char *fn);

struct taskent *taskent_next(struct taskent *te)
{
	if (te == NULL)
		return list_entry(g_tasklist.next, struct taskent, list_head);
	if (te->list_head.next == &g_tasklist)
		return NULL;
	return list_entry(te->list_head.next, struct taskent, list_head);
}

static struct taskent *taskent_new(char *dnm, char *tnm, unsigned int pri,
				   char *ofn, char *cnm)
{
	struct taskent *te = dld_malloc(sizeof(struct taskent), "taskent");
	struct taskent *te_share;

	if (te == NULL)
		return NULL;
	INIT_LIST_HEAD(&te->list_head);
	te->devname = dld_strdup(dnm, "taskent->devname");
	if (te->devname == NULL)
		goto fail;
	te->taskname = dld_strdup(tnm, "taskent->taskname");
	if (te->taskname == NULL)
		goto fail;
	te->pri = pri;
	te->lkcmd = (strlen(cnm) > 0) ? lkcmd_new(cnm) : NULL;
	te->minor = -1;
	te->request = TREQ_NONE;
	if ((te_share = taskent_find_by_coffname(ofn)) != NULL)
		te->cobj = te_share->cobj;
	else
		te->cobj = coff_new(ofn);
	te->cobj->taskcount++;

	return te;

fail:
	if (te->devname)
		dld_free(te, "taskent->devname");
	if (te->taskname)
		dld_free(te, "taskent->taskname");
	if (te)
		dld_free(te, "taskent");
	return NULL;
}

void taskent_freelist(void)
{
	struct taskent *te, *tmp;

	taskent_for_each_safe(te, tmp, &g_tasklist) {
		list_del(&te->list_head);
		if (te->devname)
			dld_free(te->devname, "taskent->devname");
		if (te->taskname)
			dld_free(te->taskname, "taskent->taskname");
		if (te->lkcmd)
			lkcmd_free(te->lkcmd);
		if (--te->cobj->taskcount == 0)
			coff_free(te->cobj);
		dld_free(te, "taskent");
	}
}

static int parse_config_line(struct dld_conf *conf, char *s, int line_num)
{
	if (s[0] == '$') {
		/* var */
		char key[32] = { '\0' };
		char val[128] = { '\0' };

		sscanf(s, "%s %s", key, val);
		if (!strcmp(key, "$kernel")) {
			if (!conf->knlfn)
				conf->knlfn = dld_strdup(val, "dld_conf->knlfn");
		} else if (!strcmp(key, "$cmd")) {
			if (!conf->cmdfn)
				conf->cmdfn = dld_strdup(val, "dld_conf->cmdfn");
		} else {
			prmsg("error in %s: unknown variable %s at line %d\n",
			      conf->cfgfn, key, line_num);
			return -1;
		}
	} else {
		/* task entry */
		char dnm[128] = { '\0' };
		char tnm[128] = { '\0' };
		unsigned int pri = 0;
		char ofn[128] = { '\0' };
		char cnm[128] = { '\0' };
		struct taskent *te;

		switch (sscanf(s, "%s %s %d %s %s", dnm, tnm, &pri, ofn, cnm)) {
			case 4:
			case 5:
				te = taskent_new(dnm, tnm, pri, ofn, cnm);
				list_add_tail(&te->list_head, &g_tasklist);
				break;
			default:
				prmsg("error in %s at line %d.\n",
				      conf->cfgfn, line_num);
				return -1;
		}
	}

	return 0;
}

int taskent_readconfig(struct dld_conf *conf)
{
	int fd;
	struct stat stat;
	size_t size;
	char *src, *endp;
	char *p, *bl;
	int comment;
	int line_num;

	if ((fd = open(conf->cfgfn, O_RDONLY)) < 0) {
		prmsg("Can't open %s\n", conf->cfgfn);
		return -1;
	}

	/* acquiring file size */
	if (fstat(fd, &stat) < 0) {
		prmsg("%s: fstat() failed\n", conf->cfgfn);
		return -1;
	}
	size = stat.st_size;

	/* mmap */
	if ((src = mmap(0, size, PROT_READ, MAP_SHARED, fd, 0)) < 0) {
		prmsg("%s: mmap() failed\n", conf->cfgfn);
		return -1;
	}

	endp = src + size;
	comment = 0;
	line_num = 1;
	bl = src;
	for (p = src; ; p++) {
		int eol;

		eol = (*p == '#') || (*p == '\n') || (p == endp);
		if (eol && (p > bl) && (!comment)) {
			char lbuf[256];

			strncpy(lbuf, bl, p - bl);
			lbuf[p - bl] = '\0';

			if (parse_config_line(conf, lbuf, line_num) < 0)
				return -1;
		}
		if (p == endp)
			break;
		if (*p == '#')
			comment = 1;
		if (*p == '\n') {
			comment = 0;
			bl = p + 1;
			line_num++;
		}
	}

	munmap(src, size);
	close(fd);

	return 0;
}

int taskent_mkdev(int fd)
{
#ifdef DSP_EMULATION
	struct taskent *te;
	static int minor = 0;

	g_taskent_for_each(te) {
		te->minor = minor++;
	}

	return 0;
#else
	struct taskent *te;
	int minor;
	char path1[20];
	char path2[20 + OMAP_DSP_TNM_LEN];
	struct stat file_stat;

	/*
	 * Firstly we make all task devices.
	 */
	g_taskent_for_each(te) {
		prmsg("mkdev %s\n", te->devname);
		minor = ioctl(fd, OMAP_DSP_TWCH_IOCTL_MKDEV, te->devname);
		if (minor < 0) {
			prmsg("MKDEV failed\n");
			return -1;
		}
		te->minor = minor;
	}

	/*
	 * Then wait for udevd to create the nodes.
	 */
	g_taskent_for_each(te) {
		int retry = 5;

		sprintf(path1, "/dev/dsptask%d", te->minor);
		sprintf(path2, TASKDEV_DIR "/%s", te->devname);
#if 0 // 3.1
		if (stat(path1, &file_stat) < 0)
			continue;	/* probably devfs system */
#else
retry:
		if (stat(path1, &file_stat) < 0) {
			/* wait for udevd creates the node */
			if (--retry) {
				sleep(1);
				goto retry;
			}
			prmsg("stat failed for %s\n", path1);
			return -1;
		}
#endif
		if (stat(path2, &file_stat) >= 0)
			unlink(path2);	/* garbage */
		if (symlink(path1, path2) < 0) {
			prmsg("symlink(%s,%s) failed\n", path1, path2);
			return -1;
		}
	}

	return 0;
#endif
}

int taskent_rmdev(int fd)
{
#ifdef DSP_EMULATION
	return 0;
#else
	int ret = 0;
	struct taskent *te;
#if 0 // 3.1
	char path1[20];
#endif
	char path2[20 + OMAP_DSP_TNM_LEN];
#if 0 // 3.1
	struct stat file_stat;
#endif

	g_taskent_for_each(te) {
#if 0 // 3.1
		sprintf(path1, "/dev/dsptask%d", te->minor);
#endif
		sprintf(path2, TASKDEV_DIR "/%s", te->devname);
#if 0 // 3.1
		if (stat(path1, &file_stat) < 0)
			continue;	/* probably devfs system */
#endif
		if (unlink(path2) < 0) {
			prmsg("unlink(%s) failed\n", path2);
			ret = -1;
		}
		prmsg("rmdev %s\n", te->devname);
		if (ioctl(fd, OMAP_DSP_TWCH_IOCTL_RMDEV, te->devname) < 0) {
			prmsg("rmdev failed\n");
			ret = -1;
		}
	}
	return ret;
#endif
}

#ifndef DSP_EMULATION
int taskent_process_request_all(void)
{
	struct taskent *te;
	int ret;

	g_taskent_for_each(te) {
		switch (te->request) {
			case TREQ_ADD:
				if ((ret = twch_tadd(te)) < 0)
					return ret;
				break;
			case TREQ_DEL:
				if ((ret =twch_tdel(te)) < 0)
					return ret;
				break;
			default:
				break;
		}
	}

	return 0;
}
#endif /* !DSP_EMULATION */

struct taskent *taskent_find_by_minor(u8 minor)
{
	struct taskent *te;

	g_taskent_for_each(te) {
		if (te->minor == minor)
			return te;
	}
	/* not found */
	return NULL;
}

static struct taskent *taskent_find_by_coffname(char *fn)
{
	struct taskent *te;

	g_taskent_for_each(te) {
		if (!strcmp(te->cobj->fn, fn))
			return te;
	}
	/* not found */
	return NULL;
}

int taskent_register_lkcmd(struct taskent *te, struct lkcmd *lkcmd)
{
	struct memmgr *mem, *tmp;

	if (!list_empty(&te->lkcmd->memlist)) {
		prmsg("te->lkcmd->memlist is not empty at %s line %d\n",
		      __FILE__, __LINE__);
		return -1;
	}

	memmgr_for_each_safe(mem, tmp, &lkcmd->memlist) {
		if (!memmgr_validate(mem))
			return -1;
		mem->lkcmd = te->lkcmd;
		list_del(&mem->list_head);
		list_add_tail(&mem->list_head, &te->lkcmd->memlist);
	}
#ifndef DSP_EMULATION
	memmgr_exmap(&te->lkcmd->memlist);
#endif

	list_splice(&lkcmd->dirlist, &te->lkcmd->dirlist);
	INIT_LIST_HEAD(&lkcmd->dirlist);
	list_splice(&lkcmd->exprlist, &te->lkcmd->exprlist);
	INIT_LIST_HEAD(&lkcmd->exprlist);
	list_splice(&lkcmd->loptlist, &te->lkcmd->loptlist);
	INIT_LIST_HEAD(&lkcmd->loptlist);

	return 0;
}

struct memmgr *taskent_mem_range_user(u32 base, u32 size)
{
	struct taskent *te;

	g_taskent_for_each(te) {
		struct list_head *memlist;
		struct memmgr *mem;

		if (!te->lkcmd)
			continue;
		memlist = &te->lkcmd->memlist;
		if (list_empty(memlist))
			continue;

		mem = memmgr_range_user(memlist, base, size);
		if (mem)
			return mem;
	}

	return NULL;
}

void taskent_mem_sendstat(int fd)
{
	char buf[256];
	struct server_event *e = (struct server_event *)buf;
	size_t strsz = 256 - SERVER_EVENT_HDRSZ;
	int cnt;
	struct taskent *te;

	g_taskent_for_each(te) {
		struct list_head *memlist;

		if (!te->lkcmd)
			continue;
		memlist = &te->lkcmd->memlist;
		if (list_empty(memlist))
			continue;

		cnt = snprintf(e->data.s, strsz, "\n%s:\n", te->taskname);
		e->event = DLD_EVENT_STRING;
		e->len = SERVER_EVENT_HDRSZ + cnt;
		write(fd, e, e->len);

		memmgr_sendstat(memlist, fd);
	}
}

void taskent_sym_sendstat(int fd)
{
	char buf[256];
	struct server_event *e = (struct server_event *)buf;
	size_t strsz = 256 - SERVER_EVENT_HDRSZ;
	int cnt;
	struct taskent *te;

	g_taskent_for_each(te) {
		struct list_head *symlist = &te->cobj->symlist;

		if (list_empty(symlist))
			continue;

		cnt = snprintf(e->data.s, strsz, "\n%s:\n", te->taskname);
		e->event = DLD_EVENT_STRING;
		e->len = SERVER_EVENT_HDRSZ + cnt;
		write(fd, e, e->len);

		symbol_sendstat(symlist, fd);
	}
}

void taskent_scn_sendstat(int fd)
{
	char buf[256];
	struct server_event *e = (struct server_event *)buf;
	size_t strsz = 256 - SERVER_EVENT_HDRSZ;
	int cnt;
	struct taskent *te;

	g_taskent_for_each(te) {
		struct list_head *scnlist = &te->cobj->scnlist;

		if (list_empty(scnlist))
			continue;

		cnt = snprintf(e->data.s, strsz, "\n%s:\n", te->taskname);
		e->event = DLD_EVENT_STRING;
		e->len = SERVER_EVENT_HDRSZ + cnt;
		write(fd, e, e->len);

		section_sendstat(scnlist, fd);
	}
}

void taskent_sendstat(int fd)
{
	char buf[256];
	struct server_event *e = (struct server_event *)buf;
	size_t strsz = 256 - SERVER_EVENT_HDRSZ;
	int cnt;
	struct taskent *te;

	e->event = DLD_EVENT_STRING;
	g_taskent_for_each(te) {
		cnt = snprintf(e->data.s, strsz, "device %s (minor=%d):\n",
			       te->devname, te->minor);
		e->len = SERVER_EVENT_HDRSZ + cnt;
		write(fd, e, e->len);

		cnt = snprintf(e->data.s, strsz, "  priority    = %d\n", te->pri);
		e->len = SERVER_EVENT_HDRSZ + cnt;
		write(fd, e, e->len);

		cnt = snprintf(e->data.s, strsz, "  task symbol = %s\n",
			       te->taskname);
		e->len = SERVER_EVENT_HDRSZ + cnt;
		write(fd, e, e->len);

		cnt = snprintf(e->data.s, strsz,
			       "  module      = %s (usecount = %d)\n",
			       te->cobj->fn, te->cobj->usecount);
		e->len = SERVER_EVENT_HDRSZ + cnt;
		write(fd, e, e->len);

		cnt = snprintf(e->data.s, strsz, "  cmd file    = %s\n\n",
			       te->lkcmd ? te->lkcmd->fn : "");
		e->len = SERVER_EVENT_HDRSZ + cnt;
		write(fd, e, e->len);
	}
}

/*
 * debug stuff
 */
void taskent_printstat(void)
{
	struct taskent *te;

	prmsg("taskent list status...\n");
	g_taskent_for_each(te) {
		prmsg("device %s (minor=%d):\n"
		      "  priority    = %d\n"
		      "  task symbol = %s\n"
		      "  module      = %s (usecount = %d)\n"
		      "  cmd file    = %s\n\n",
		      te->devname, te->minor, te->pri, te->taskname,
		      te->cobj->fn, te->cobj->usecount,
		      te->lkcmd ? te->lkcmd->fn : "");
	}
}
