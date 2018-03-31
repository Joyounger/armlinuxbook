/*
 * dsp_dld/arm/dsp_dld.c
 *
 * DSP Dynamic Loader Daemon: dsp_dld.c
 *
 * Copyright (C) 2003-2005 Nokia Corporation
 *
 * Written by Toshihiro Kobayashi <toshihiro.kobayashi@nokia.com>
 * Written by Kiyotaka Takahashi <kiyotaka.takahashi@nokia.com>
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
 * 2005/07/11:  DSP Gateway version 3.3
 */

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <malloc.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <asm/arch/dsp.h>
#include "list.h"
#include "coff-c55x.h"
#include "dsp_dld.h"
#include "dld_malloc.h"
#include "dld_daemon.h"
#include "dld_server.h"
#include "dld_taskent.h"
#include "dld_memmgr.h"
#include "dld_cmd.h"
#include "dld_coff.h"
#include "dld_section.h"
#include "dld_symbol.h"

#define VERSION_STR	"3.3"
#define IFVER_STR	"3.3"	/* kernel I/F version name */

#define DFLT_CFGFN	"/etc/dsp/dsp_dld.conf"
#define DFLT_KNLFN	"/etc/dsp/tinkernel.out"
#define DFLT_CMDFN	"/etc/dsp/tinkernelcfg.cmd"

/*
 * function declarations
 */
static void setup_signal(void);
static int read_opt(int argc, char **argv);

#ifdef DSP_EMULATION
#define driver_version_check()	do {} while (0)
#else
static void driver_version_check(void);
#endif

static int dsp_cleanup(void);
static void read_binary_version(struct coffobj *cobj);
static int start_kernel(struct coffobj *cobj);
static int stop_kernel(void);
int task_clear(struct taskent *te);

/*
 * global variables
 */
struct dld_conf dld_conf;
static int go_daemon = 1;
unsigned long binary_version;

#ifdef DEBUG_BOOT
#  define taskent_printstat_boot()		taskent_printstat()
#  define lopt_printstat_boot(list)		lopt_printstat(list)
#  define globexpr_printstat_boot(list)		globexpr_printstat(list)
#  define directive_printstat_boot(list)	directive_printstat(list)
#  define memmgr_printstat_boot(list)		memmgr_printstat(list)
#  define section_printstat_boot(list)		section_printstat(list)
#  define symbol_printstat_boot(list)		symbol_printstat(list)
#endif

#ifdef DEBUG_MEMMGR
#  define memmgr_printstat_boot(list)	memmgr_printstat(list)
#endif

#ifdef DEBUG_SECTION
#  define section_printstat_boot(list)	section_printstat(list)
#endif

#ifdef DEBUG_SYMBOL
#  define symbol_printstat_boot(list)	symbol_printstat(list)
#endif

#ifndef taskent_printstat_boot
#  define taskent_printstat_boot()		do {} while (0)
#endif
#ifndef lopt_printstat_boot
#  define lopt_printstat_boot(list)		do {} while (0)
#endif
#ifndef globexpr_printstat_boot
#  define globexpr_printstat_boot(list)		do {} while (0)
#endif
#ifndef directive_printstat_boot
#  define directive_printstat_boot(list)	do {} while (0)
#endif
#ifndef memmgr_printstat_boot
#  define memmgr_printstat_boot(list)		do {} while (0)
#endif
#ifndef section_printstat_boot
#  define section_printstat_boot(list)		do {} while (0)
#endif
#ifndef symbol_printstat_boot
#  define symbol_printstat_boot(list)		do {} while (0)
#endif

#define SYS_DSP_DIR1	"/sys/devices/platform/dsp.0"
#define SYS_DSP_DIR2	"/sys/devices/platform/dsp0"
#define SYS_DSP_DIR3	"/sys/devices/platform/dsp"
char *sys_dsp_dir;

int main(int argc, char **argv)
{
	struct coffobj *knl_cobj = NULL;
	struct lkcmd *gbl_lkcmd = NULL;
	server_return_t status = SERVER_ERROR;
	int ret;

	/* parse command line option */
	if ((ret = read_opt(argc, argv)) != 0)
		return (ret < 0) ? 1 : 0;

	/* driver version check */
	driver_version_check();

	/* cleanup DSP */
	if (dsp_cleanup() < 0)
		goto cleanup;

	/* task configuration */
	if (!dld_conf.cfgfn)
		dld_conf.cfgfn = dld_strdup(DFLT_CFGFN, "dld_conf->cfgfn");
	if (taskent_readconfig(&dld_conf) < 0)
		goto cleanup;
	taskent_printstat_boot();

	/* configuration check */
	if (!dld_conf.cmdfn)
		dld_conf.cmdfn = dld_strdup(DFLT_CMDFN, "dld_conf->cmdfn");
	if (!dld_conf.knlfn)
		dld_conf.knlfn = dld_strdup(DFLT_KNLFN, "dld_conf->knlfn");
	if (go_daemon && (daemon_config_check(&dld_conf) < 0))
		goto cleanup;

	/* read command file */
	gbl_lkcmd = lkcmd_new(dld_conf.cmdfn);
	if (lkcmd_read(gbl_lkcmd) < 0) {
		lkcmd_free(gbl_lkcmd);
		goto cleanup;
	}
	if (memmgr_register_global(&gbl_lkcmd->memlist) < 0) {
		lkcmd_free(gbl_lkcmd);
		goto cleanup;
	}
#ifdef STICKY_LIST
	directive_register_global(&gbl_lkcmd->dirlist);
#endif

	lopt_printstat_boot(&gbl_lkcmd->loptlist);
	globexpr_printstat_boot(&gbl_lkcmd->exprlist);
	memmgr_printstat_boot(NULL);
	directive_printstat_boot(&gbl_lkcmd->dirlist);

	lkcmd_free(gbl_lkcmd);

	/* read and start KERNEL */
	knl_cobj = coff_new(dld_conf.knlfn);
	if (coff_read_kernel(knl_cobj) < 0) {
		coff_free(knl_cobj);
		goto cleanup;
	}
	section_printstat_boot(&knl_cobj->scnlist);
	symbol_printstat_boot(&knl_cobj->symlist);

	memmgr_occupy_kernel(&knl_cobj->scnlist);
	memmgr_printstat_boot(NULL);	/* state after kernel is read */

	read_binary_version(knl_cobj);

	if (start_kernel(knl_cobj) < 0) {
		coff_free(knl_cobj);
		goto cleanup;
	}

#ifdef STICKY_LIST
	section_register_global(&knl_cobj->scnlist);
	symbol_register_global(&knl_cobj->symlist);
#endif
	symbol_printstat_boot(NULL);	/* state after kernel is read */

	coff_free(knl_cobj);

	/* signal handler installation */
	setup_signal();

	/* daemonize */
	if (go_daemon && (daemon_open() < 0)) {
		stop_kernel();
		goto cleanup;
	}
	prmsg("starting.\n");

	/* server */
	status = server();

	/* stop KERNEL and cleanup */
	stop_kernel();
cleanup:
#ifdef STICKY_LIST
	section_freelist(NULL);
	symbol_freelist(NULL);
	lkcmd_clear(NULL);
#else
	memmgr_freelist(NULL);
#endif
	taskent_freelist();
	dld_free(dld_conf.cfgfn, "dld_conf->cfgfn");
	dld_free(dld_conf.knlfn, "dld_conf->knlfn");
	dld_free(dld_conf.cmdfn, "dld_conf->cmdfn");

	if (status == SERVER_RESTART) {
		pid_t pid;

		prmsg("restarting.\n");
		if ((pid = fork()) < 0) {
			prmsg("fork() failed\n");
			return 1;
		}
		if (pid != 0)	/* parent exits */
			return 0;

		/* child */
		sleep(1);	/* wait for old process completes */
		execvp(argv[0], argv);
	}

	prmsg("exitting.\n");
	daemon_close();
	return (status == SERVER_ERROR) ? 1 : 0;
}

static void setup_signal(void)
{
	struct sigaction sighup, sigint, sigterm, sigchld;

	/* SIGHUP */
	sighup.sa_handler = dspsig_handler;
	__sigemptyset(&sighup.sa_mask);
	sighup.sa_flags = SA_NOMASK | SA_ONESHOT;
	sighup.sa_restorer = NULL;
	sigaction(SIGHUP, &sighup, NULL);

	/* SIGINT */
	sigint.sa_handler = dspsig_handler;
	__sigemptyset(&sigint.sa_mask);
	sigint.sa_flags = SA_NOMASK | SA_ONESHOT;
	sigint.sa_restorer = NULL;
	sigaction(SIGINT, &sigint, NULL);

	/* SIGTERM */
	sigterm.sa_handler = dspsig_handler;
	__sigemptyset(&sigterm.sa_mask);
	sigterm.sa_flags = SA_NOMASK | SA_ONESHOT;
	sigterm.sa_restorer = NULL;
	sigaction(SIGTERM, &sigterm, NULL);

	/* SIGCHLD */
	sigchld.sa_handler = dspsig_handler;
	__sigemptyset(&sigchld.sa_mask);
	sigchld.sa_flags = SA_NOMASK | SA_RESTART;
	sigchld.sa_restorer = NULL;
	sigaction(SIGCHLD, &sigchld, NULL);
}

static int read_opt(int argc, char **argv)
{
	int ret = -1;
	int i;

	for (i = 1; i < argc; i++) {
		if (!strcmp(argv[i], "-c")) {
			if (++i >= argc)
				goto err;
			dld_conf.cfgfn = dld_strdup(argv[i], "dld_conf->cfgfn");
		} else if (!strcmp(argv[i], "-b")) {
			if (++i >= argc)
				goto err;
			dld_conf.knlfn = dld_strdup(argv[i], "dld_conf->knlfn");
		} else if (!strcmp(argv[i], "-m")) {
			if (++i >= argc)
				goto err;
			dld_conf.cmdfn = dld_strdup(argv[i], "dld_conf->cmdfn");
		} else if (!strcmp(argv[i], "-p")) {
			go_daemon = 0;
		} else if ((!strcmp(argv[i], "-v")) ||
			   (!strcmp(argv[i], "--version"))) {
			fprintf(stderr, "dsp_dld version %s\n\n", VERSION_STR);
			ret = 1;
			goto err;
		} else
			goto err;
	}

	return 0;

err:
	fprintf(stderr,
		"usage: %s\n"
		"  [-c configfile]\n"
		"  [-b kernelfile]\n"
		"  [-m cmdfile]\n"
		"  [-p]\n"
		"  [-v|--version]\n", argv[0]);
	return ret;
}

#ifndef DSP_EMULATION
static void driver_version_check(void)
{
	int fd;
	char buf[4096];
	char *s, *p;
	int cnt;

	if ((fd = open(SYS_DSP_DIR1 "/ifver", O_RDONLY)) >= 0)
		sys_dsp_dir = SYS_DSP_DIR1;
	else if ((fd = open(SYS_DSP_DIR2 "/ifver", O_RDONLY)) >= 0)
		sys_dsp_dir = SYS_DSP_DIR2;
	else if ((fd = open(SYS_DSP_DIR3 "/ifver", O_RDONLY)) >= 0)
		sys_dsp_dir = SYS_DSP_DIR3;
	else {
		fprintf(stderr,
			"Warning: none of following entries is found.\n"
			"  " SYS_DSP_DIR1 "ifver\n"
			"  " SYS_DSP_DIR2 "ifver\n"
			"  " SYS_DSP_DIR3 "ifver\n"
			"(DSP Gateway driver is too old or not loaded)\n");
		return;
	}
	cnt = read(fd, buf, 4096);
	close(fd);

	if (cnt < 0)
		goto fail;

	buf[cnt] = '\0';

	for (s = buf; s < buf + cnt; s = p + 1) {
		for (p = s; *p && (*p != '\n'); p++);
		if (!strncmp(s, IFVER_STR, p-s))
			return;	/* success */
	}

fail:
	fprintf(stderr,
"***************************************************************\n"
"Warning: versions of dspctl and DSP Gateway driver don't match!\n"
"  dspctl version: %s\n"
"  driver supported I/F version:\n"
"%s"
"***************************************************************\n\n",
		IFVER_STR, buf);
}

int dev_mklink(int n)
{
	int fd;
	int i;
	char name[OMAP_DSP_TNM_LEN];
	char path1[20];
	char path2[20 + OMAP_DSP_TNM_LEN];
	struct stat file_stat;

	for (i = 0; i < n; i++) {
		int retry = 5;

		sprintf(path1, "/dev/dsptask%d", i);
#if 0 // 3.1
		if ((fd = open(path1, O_RDWR)) < 0)
			/* dynamic device file system (devfs, udev) */
			continue;
#else
retry:
		if ((fd = open(path1, O_RDWR)) < 0) {
			/* wait for udevd creates the node */
			if (--retry) {
				sleep(1);
				goto retry;
			}
			prmsg("open %s failed\n", path1);
			return -1;
		}
#endif
		if (ioctl(fd, OMAP_DSP_TASK_IOCTL_GETNAME, name) < 0) {
			prmsg("GETNAME for %s failed\n", path1);
			return -1;
		}
		close(fd);
		sprintf(path2, TASKDEV_DIR "/%s", name);
		if (stat(path2, &file_stat) == 0)
			unlink(path2);	/* garbage */
		if (symlink(path1, path2) < 0) {
			prmsg("symlink(%s, %s) failed\n", path1, path2);
			return -1;
		}
	}

	return 0;
}

int dev_unlink(int n)
{
	int fd;
	int i;
	int ret = 0;
	char name[OMAP_DSP_TNM_LEN];
	char path1[20];
	char path2[20 + OMAP_DSP_TNM_LEN];

	for (i = 0; i < n; i++) {
		sprintf(path1, "/dev/dsptask%d", i);
		if ((fd = open(path1, O_RDWR)) < 0) {
			/* dynamic device file system (devfs, udev) */
			continue;
		}
		if (ioctl(fd, OMAP_DSP_TASK_IOCTL_GETNAME, name) < 0) {
			prmsg("GETNAME for %s failed\n", path1);
			ret = -1;
			continue;
		}
		close(fd);
		sprintf(path2, TASKDEV_DIR "/%s", name);
		if (unlink(path2) < 0) {
			prmsg("unlink(%s) failed\n", path2);
			ret = -1;
			continue;
		}
	}

	return ret;
}
#endif /* DSP_EMULATION */

static int dsp_cleanup(void)
{
#ifndef DSP_EMULATION
	int n_task;
	int fd_ctl, fd_mem;

	fd_ctl = open(DEVNAME_CONTROL, O_RDWR);
	if (fd_ctl < 0) {
		prmsg("%s open failed at %s line %d\n",
		      DEVNAME_CONTROL, __FILE__, __LINE__);
		return -1;
	}
	fd_mem = open(DEVNAME_DSPMEM, O_RDWR);
	if (fd_mem < 0) {
		prmsg("%s open failed at %s line %d\n",
		      DEVNAME_DSPMEM, __FILE__, __LINE__);
		close(fd_ctl);
		return -1;
	}

	if ((n_task = ioctl(fd_ctl, OMAP_DSP_IOCTL_TASKCNT)) < 0)
		prmsg("TASKCNT failed at %s line %d\n", __FILE__, __LINE__);
	else
		dev_unlink(n_task);
	ioctl(fd_ctl, OMAP_DSP_IOCTL_DSPUNCFG);
	ioctl(fd_ctl, OMAP_DSP_IOCTL_RESET);
	ioctl(fd_mem, OMAP_DSP_MEM_IOCTL_EXMAP_FLUSH);
	ioctl(fd_mem, OMAP_DSP_MEM_IOCTL_MMUINIT);

	close(fd_mem);
	close(fd_ctl);
#endif /* DSP_EMULATION */

	return 0;
}

struct COFF_dspgw_version {
	char major[2];
	char minor[2];
	char extra1[2];
	char extra2[2];
};
#define COFF_DSPGW_VERSION struct COFF_dspgw_version
#define COFF_DSPGW_VERSION_SZ	8

#define LEu16(p)	((u16)(((u16)(p)[0])<<8) | \
			       ((u16)(p)[1]))

static void read_binary_version(struct coffobj *cobj)
{
	struct section *scn;
	COFF_DSPGW_VERSION *v;
	u16 major, minor, extra1, extra2;

	binary_version = 0;

	if ((scn = section_find_by_name(&cobj->scnlist, "dspgw_version")) == NULL) {
		prmsg("failed to read binary version.\n");
		return;
	}
	if (!scn->data || (scn->size < COFF_DSPGW_VERSION_SZ))
		return;
	v = (COFF_DSPGW_VERSION *)scn->data;
	major  = LEu16(v->major);
	minor  = LEu16(v->minor);
	extra1 = LEu16(v->extra1);
	extra2 = LEu16(v->extra2);
	binary_version = (major << 16) | minor;
	prmsg("detected binary version %d.%d.%d.%d\n",
	      major, minor, extra1, extra2);
}

static int start_kernel(struct coffobj *cobj)
{
#ifndef DSP_EMULATION
	int n_task;
	int fd;
	struct stat dir_stat;

	if (stat(TASKDEV_DIR, &dir_stat) < 0) {
		if (mkdir(TASKDEV_DIR, 0755) < 0) {
			prmsg("mkdev(%s) failed at %s line %d\n",
			      TASKDEV_DIR, __FILE__, __LINE__);
			return -1;
		}
	} else {
		if (!S_ISDIR(dir_stat.st_mode)) {
			prmsg(TASKDEV_DIR " is not a directory.\n");
			return -1;
		}
	}

	fd = open(DEVNAME_CONTROL, O_RDWR);
	if (fd < 0) {
		prmsg("%s open failed at %s line %d\n",
		      DEVNAME_CONTROL, __FILE__, __LINE__);
		return -1;
	}

	/* load */
	ioctl(fd, OMAP_DSP_IOCTL_MPUI_BYTESWAP_OFF);
#endif /* DSP_EMULATION */
	if (section_load(&cobj->scnlist, NULL) < 0)
		return -1;
#ifndef DSP_EMULATION
	usleep(10000);	/* wait for DSP memory settled */
	ioctl(fd, OMAP_DSP_IOCTL_RESET);
	prmsg("setting DSP reset vector to 0x%06lx\n", cobj->entry);
	ioctl(fd, OMAP_DSP_IOCTL_SETRSTVECT, cobj->entry);
	prmsg("releasing DSP reset\n");
	ioctl(fd, OMAP_DSP_IOCTL_RUN);
	usleep(10000);	/* wait for DSP initialization */
	prmsg("DSP configuration ...\n");
	if (ioctl(fd, OMAP_DSP_IOCTL_DSPCFG) < 0) {
		prmsg("  failed\n");
		close(fd);
		return -1;
	}
	prmsg("  succeeded.\n");

	if ((n_task = ioctl(fd, OMAP_DSP_IOCTL_TASKCNT)) < 0) {
		prmsg("TASKCNT failed at %s line %d\n", __FILE__, __LINE__);
		close(fd);
		return -1;
	}
	close(fd);

	if (dev_mklink(n_task) < 0)
		return -1;
#endif /* DSP_EMULATION */

	return 0;
}

static int stop_kernel(void)
{
#ifndef DSP_EMULATION
	int n_task;
	int fd;

	fd = open(DEVNAME_CONTROL, O_RDWR);
	if (fd < 0) {
		prmsg("%s open failed at %s line %d\n",
		      DEVNAME_CONTROL, __FILE__, __LINE__);
		return -1;
	}

	if ((n_task = ioctl(fd, OMAP_DSP_IOCTL_TASKCNT)) < 0) {
		prmsg("TASKCNT failed at %s line %d\n", __FILE__, __LINE__);
	} else
		dev_unlink(n_task);

	prmsg("releasing resources for DSP\n");
	ioctl(fd, OMAP_DSP_IOCTL_DSPUNCFG);
	prmsg("DSP reset\n");
	ioctl(fd, OMAP_DSP_IOCTL_RESET);
	close(fd);
#endif /* DSP_EMULATION */
	return 0;
}


#define __symbol_printstat_1(list) \
	do { \
		prmsg("\nsymbols before resolve\n"); \
		symbol_printstat(list); \
	} while (0)

#define __symbol_printstat_2(list) \
	do { \
		prmsg("\nsymbols after resolve\n"); \
		symbol_printstat(list); \
	} while (0)

#define __symbol_printstat_3(list) \
	do { \
		prmsg("\nsymbols after placement\n"); \
		symbol_printstat(list); \
	} while (0)


#define __memmgr_printstat_1(list) \
	do { \
		prmsg("\nlocal memmgr before load\n"); \
		memmgr_printstat(list); \
	} while (0)

#define __memmgr_printstat_2(list) \
	do { \
		prmsg("\nlocal memmgr after load\n"); \
		memmgr_printstat(list); \
	} while (0)

#define __memmgr_printstat_3(list) \
	do { \
		prmsg("\nkernel memmgr after load\n"); \
		memmgr_printstat(list); \
	} while (0)


#define __section_printstat_1(list) \
	do { \
		prmsg("\nsections before placement\n"); \
		section_printstat(list); \
	} while (0)

#define __section_printstat_2(list) \
	do { \
		prmsg("\nsections after placement\n"); \
		section_printstat(list); \
	} while (0)

#ifdef DEBUG_SYMBOL
#  define symbol_printstat_1(list)	__symbol_printstat_1(list)
#  define symbol_printstat_2(list)	__symbol_printstat_2(list)
#  define symbol_printstat_3(list)	__symbol_printstat_3(list)
#endif

#ifdef DEBUG_MEMMGR
#  define memmgr_printstat_1(list)	__memmgr_printstat_1(list)
#  define memmgr_printstat_2(list)	__memmgr_printstat_2(list)
#  define memmgr_printstat_3(list)	__memmgr_printstat_3(list)
#endif

#ifdef DEBUG_SECTION
#  define section_printstat_1(list)	__section_printstat_1(list)
#  define section_printstat_2(list)	__section_printstat_2(list)
#endif

#ifdef DEBUG_RESOLVE
#  define symbol_printstat_1(list)	__symbol_printstat_1(list)
#  define symbol_printstat_2(list)	__symbol_printstat_2(list)
#endif

#ifdef DEBUG_PLACEMENT
#  define symbol_printstat_2(list)	__symbol_printstat_2(list)
#  define symbol_printstat_3(list)	__symbol_printstat_3(list)
#  define section_printstat_1(list)	__section_printstat_1(list)
#  define section_printstat_2(list)	__section_printstat_2(list)
#  define memmgr_printstat_2(list)	__memmgr_printstat_2(list)
#  define memmgr_printstat_3(list)	__memmgr_printstat_3(list)
#endif

#ifndef symbol_printstat_1
#  define symbol_printstat_1(list)	do {} while (0)
#endif
#ifndef symbol_printstat_2
#  define symbol_printstat_2(list)	do {} while (0)
#endif
#ifndef symbol_printstat_3
#  define symbol_printstat_3(list)	do {} while (0)
#endif

#ifndef memmgr_printstat_1
#  define memmgr_printstat_1(list)	do {} while (0)
#endif
#ifndef memmgr_printstat_2
#  define memmgr_printstat_2(list)	do {} while (0)
#endif
#ifndef memmgr_printstat_3
#  define memmgr_printstat_3(list)	do {} while (0)
#endif

#ifndef section_printstat_1
#  define section_printstat_1(list)	do {} while (0)
#endif
#ifndef section_printstat_2
#  define section_printstat_2(list)	do {} while (0)
#endif

u32 task_load(struct taskent *te)
{
#ifndef STICKY_LIST
	struct coffobj *knl_cobj = NULL;
#endif
	struct lkcmd *gbl_lkcmd = NULL;
	struct list_head *knl_symlist = NULL;
	struct coffobj *cobj = te->cobj;
	struct symbol *tasksym;
	LIST_HEAD(memlist);

	/*
	 * if this task has been loaded already, share the object.
	 */
	if (cobj->usecount++ > 0)
		goto find_tasksym;

#ifndef STICKY_LIST
	knl_cobj = coff_new(dld_conf.knlfn);
	if (coff_read_kernel(knl_cobj) < 0)
		goto abort;
	knl_symlist = &knl_cobj->symlist;

	gbl_lkcmd = lkcmd_new(dld_conf.cmdfn);
	if (lkcmd_read(gbl_lkcmd) < 0)
		goto abort;
	memmgr_freelist(&gbl_lkcmd->memlist);
#endif

	prmsg("loading %s.\n", cobj->fn);
	if (coff_read_task(cobj) < 0)
		goto abort;
	symbol_printstat_1(&cobj->symlist);

	if (te->lkcmd) {
		struct lkcmd *lcl_lkcmd = lkcmd_new(te->lkcmd->fn);
		if (lkcmd_read(lcl_lkcmd) < 0)
			goto abort;
		if (taskent_register_lkcmd(te, lcl_lkcmd) < 0)
			goto abort;
		lkcmd_free(lcl_lkcmd);
		memmgr_printstat_1(&te->lkcmd->memlist);
	}

	if (symbol_resolve(knl_symlist, &cobj->symlist) < 0) {
		prmsg("link failed.\n");
		goto abort;
	}
	symbol_printstat_2(&cobj->symlist);

	section_printstat_1(&cobj->scnlist);
	if (memmgr_placetask(te, cobj, gbl_lkcmd) < 0)
		goto abort;
	symbol_printstat_3(&cobj->symlist);
	section_printstat_2(&cobj->scnlist);
	if (section_relocate(&cobj->scnlist) < 0)
		goto abort;

	if (section_load(&cobj->scnlist, te) < 0)
		goto abort;
	memmgr_printstat_2(NULL);
	if (te->lkcmd)
		memmgr_printstat_3(&te->lkcmd->memlist);

#ifndef STICKY_LIST
	coff_free(knl_cobj);
	lkcmd_free(gbl_lkcmd);
#endif

find_tasksym:
	tasksym = symbol_find_by_name(&cobj->symlist, te->taskname);
	if (tasksym == NULL) {
		prmsg("symbol %s not found.\n", te->taskname);
		return OMAP_DSP_TADD_ABORTADR;
	}

#if 0
	/*
	 * We do not free the coff objecct here...
	 * It can be used for debugging.
	 */
	coff_clear(cobj);
#endif

	prmsg("adding %s(@%06lx) into system.\n", te->taskname, tasksym->value);

	return tasksym->value;

abort:
#ifndef STICKY_LIST
	if (knl_cobj)
		coff_free(knl_cobj);
	if (gbl_lkcmd)
		lkcmd_free(gbl_lkcmd);
#endif
	task_clear(te);
	return OMAP_DSP_TADD_ABORTADR;
}

int task_clear(struct taskent *te)
{
	struct coffobj *cobj = te->cobj;

	prmsg("cleaning task %s...\n", te->devname);

	if (--cobj->usecount > 0) {
		return 0;
	}

	if (te->lkcmd)
		lkcmd_clear(te->lkcmd);
	coff_clear(cobj);

	prmsg("unloading %s.\n", cobj->fn);
	memmgr_cleartask(te);
	return 0;
}
