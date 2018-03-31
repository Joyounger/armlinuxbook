/*
 * dspapps/utils/dspctl.c
 *
 * control utility for DSP Gateway
 *
 * Copyright (C) 2002-2005 Nokia Corporation
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
 * 2005/06/02:  DSP Gateway version 3.3
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <asm/arch/dsp.h>
#include "dspctl.h"

enum regspace { SPACE_MEM, SPACE_IO };

struct ctlcmd {
	char *name;
	int argc;
	int (*f)(char **argv);
};

static void usage(char *cmd);
static void driver_version_check(void);
static int do_start(char **argv);
static int do_stop(char **argv);
static int do_version(char **argv);
static int do_load(char **argv);
static int do_run(char **argv);
static int do_reset(char **argv);
static int do_cleanup(char **argv);
static int do_setrstvect(char **argv);
static int do_gbl_idle(char **argv);
static int do_cpu_idle(char **argv);
static int do_suspend(char **argv);
static int do_resume(char **argv);
static int do_dspcfg(char **argv);
static int do_dspuncfg(char **argv);
static int do_poll(char **argv);
static int do_kmem_reserve(char **argv);
static int do_kmem_release(char **argv);
static int do_exmap(char **argv);
static int do_exunmap(char **argv);
static int do_mapflush(char **argv);
static int do_fbexport(char **argv);
static int do_mmuinit(char **argv);
static int do_mmuitack(char **argv);
static int do_mkdev(char **argv);
static int do_rmdev(char **argv);
static int do_tadd(char **argv);
static int do_tdel(char **argv);
static int do_getvar(char **argv);
static int do_setvar_1(char **argv);
static int do_regread(char **argv);
static int do_regread_io(char **argv);
static int __regread(enum regspace space, unsigned short adr);
static int do_regwrite(char **argv);
static int do_regwrite_io(char **argv);
static int __regwrite(enum regspace space, unsigned short adr, unsigned short val);
static int do_runlevel(char **argv);
static int do_mbsend(char **argv);

static int do_showmyprintk(char **argv);
static int do_setuart(char **argv);
static int do_display(char **argv);


static void usage(char *cmd)
{
	fprintf(stderr, "dspctl version %s\n\n", VERSION_STR);
	fprintf(stderr, "usage: %s <command> ...\n", cmd);
	fprintf(stderr, "  command:\n"
		"    start <cofffile>\n"
		"    stop\n"
		"\n"
		"    version <cofffile>\n"
		"    load <cofffile>\n"
		"    run (=unreset)\n"
		"    reset\n"
		"    cleanup\n"
		"    setrstvect <addr>\n"
		"    gbl_idle\n"
		"    cpu_idle\n"
		"    suspend\n"
		"    resume\n"
		"    dspcfg\n"
		"    dspuncfg\n"
		"    poll\n"
		"    kmem_reserve <request size>\n"
		"    kmem_release\n"
		"    exmap <dspadr> <request size>\n"
		"    exunmap <dspadr>\n"
		"    mapflush\n"
		"    fbexport <dspadr>\n"
		"    mmuinit\n"
		"    mmuitack\n"
		"    mkdev <devname>\n"
		"    rmdev <devname>\n"
		"    tadd <minor> <addr>\n"
		"    tdel <minor>\n"
		"    getvar <varid>\n"
		"    setvar <varid> <data>\n"
		"    regread [-io] <adr>\n"
		"    regwrite [-io] <adr> <val>\n"
		"    runlevel <user|super>\n"
		"    mbsend <cmd> <data>\n"
		"    setuart \n");
}

static struct ctlcmd ctlcmd[] = {
	{ "start",        3, do_start },
	{ "stop",         2, do_stop },
	{ "version",      3, do_version },
	{ "load",         3, do_load },
	{ "run",          2, do_run },
	{ "unreset",      2, do_run },
	{ "reset",        2, do_reset },
	{ "cleanup",      2, do_cleanup },
	{ "setrstvect",   3, do_setrstvect },
	{ "gbl_idle",     2, do_gbl_idle },
	{ "cpu_idle",     2, do_cpu_idle },
	{ "suspend",      2, do_suspend },
	{ "resume",       2, do_resume },
	{ "dspcfg",       2, do_dspcfg },
	{ "dspuncfg",     2, do_dspuncfg },
	{ "poll",         2, do_poll },
	{ "kmem_reserve", 3, do_kmem_reserve },
	{ "kmem_release", 2, do_kmem_release },
	{ "exmap",        4, do_exmap },
	{ "exunmap",      3, do_exunmap },
	{ "mapflush",     2, do_mapflush },
	{ "fbexport",     3, do_fbexport },
	{ "mmuinit",      2, do_mmuinit },
	{ "mmuitack",     2, do_mmuitack },
	{ "mkdev",        3, do_mkdev },
	{ "rmdev",        3, do_rmdev },
	{ "tadd",         4, do_tadd },
	{ "tdel",         3, do_tdel },
	{ "getvar",       3, do_getvar },
	{ "setvar",       4, do_setvar_1 },
	{ "regread",      3, do_regread },
	{ "regread",      4, do_regread_io },
	{ "regwrite",     4, do_regwrite },
	{ "regwrite",     5, do_regwrite_io },
	{ "runlevel",     3, do_runlevel },
	{ "mbsend",       4, do_mbsend },
	{ "showmyprintk", 2, do_showmyprintk},
	{ "setuart",      2, do_setuart},
        { "display",      2, do_display },
	{ NULL, 0, NULL },
};

static int do_showmyprintk( char **argv)
{
        int fd;
	
        if ((fd = open(CTLDEVNM, O_RDWR)) < 0) {
	  perror("open control device");
	  return 1;
        }
	printf("Myprintkbuf: \n");
        ioctl(fd, OMAP_DSP_IOCT_SHOWMYPRINTK);
	close(fd);

	return 0;
}   

static int do_setuart( char **argv)
{
        int fd;

        if ((fd = open(CTLDEVNM, O_RDWR)) < 0) {
          perror("open control device");
          return 1;
        }
        printf("To switch uart: \n");
        ioctl(fd, OMAP_DSP_IOCT_SETUART);
        close(fd);

	printf("ioctl over\n");
        return 0;

}

static int do_display(char **argv)
{
        int fd;

	if ((fd = open(DSPMEMDEVNM, O_RDWR)) < 0) {
	   perror("open dspmem device");
	   return 1;
	}
	printf("DSP display\n");
	ioctl(fd, OMAP_DSP_MEM_IOCTL_DISPLAY, NULL);
	close(fd);
	return 0;
}



int main(int argc, char **argv)
{
	struct ctlcmd *cmd;

	if (argc < 2) {
		usage(argv[0]);
		return 1;
	}
	
	/* driver version chck */
	driver_version_check();

	/* uniformed functions */
	for (cmd = &ctlcmd[0]; cmd->name; cmd++) {
		if (!strcmp(argv[1], cmd->name) && (argc == cmd->argc))
			return cmd->f(argv);
	}

	usage(argv[0]);
	return 1;
}

static void driver_version_check(void)
{
	int fd;
	char *fn1 = "/sys/devices/platform/dsp.0/ifver";
	char *fn2 = "/sys/devices/platform/dsp0/ifver";
	char *fn3 = "/sys/devices/platform/dsp/ifver";
	char buf[4096];
	char *s, *p;
	int cnt;

	if (((fd = open(fn1, O_RDONLY)) < 0) &&
	    ((fd = open(fn2, O_RDONLY)) < 0) &&
	    ((fd = open(fn3, O_RDONLY)) < 0)) {
		fprintf(stderr,
			"Warning: none of %s, %s or %s is found.\n"
			"(DSP Gateway driver is too old or not loaded)\n",
			fn1, fn2, fn3);
		sleep(1);
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
	sleep(1);
}

static int dsptask_dir_check(void)
{
	struct stat dir_stat;

	if (stat(TASKDEV_DIR, &dir_stat) < 0) {
		if (mkdir(TASKDEV_DIR, 0755) < 0) {
			perror("mkdev " TASKDEV_DIR);
			return 1;
		}
	} else {
		if (!S_ISDIR(dir_stat.st_mode)) {
			fprintf(stderr, TASKDEV_DIR " is not a directory.\n");
			return 1;
		}
	}

	return 0;
}

static int dev_mklink(int n)
{
	int fd;
	int i;
	char name[OMAP_DSP_TNM_LEN];
	char path1[20];
	char path2[20 + OMAP_DSP_TNM_LEN];

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
			perror("open taskdev");
			return 1;
		}
#endif
		if (ioctl(fd, OMAP_DSP_TASK_IOCTL_GETNAME, name) < 0) {
			perror("GETNAME");
			return 1;
		}
		close(fd);
		sprintf(path2, TASKDEV_DIR "/%s", name);
		if (symlink(path1, path2) < 0) {
			perror("symlink");
			return 1;
		}
	}

	return 0;
}

static int dev_unlink(int n)
{
	int fd;
	int i;
	int ret = 0;
	char name[OMAP_DSP_TNM_LEN];
	char path1[20];
	char path2[20 + OMAP_DSP_TNM_LEN];

	for (i = 0; i < n; i++) {
		sprintf(path1, "/dev/dsptask%d", i);
		if ((fd = open(path1, O_RDWR)) < 0)
			/* dynamic device file system (devfs, udev) */
			continue;
		if (ioctl(fd, OMAP_DSP_TASK_IOCTL_GETNAME, name) < 0) {
			perror("GETNAME");
			ret = 1;
			continue;
		}
		close(fd);
		sprintf(path2, TASKDEV_DIR "/%s", name);
		if (unlink(path2) < 0) {
			perror("unlink");
			ret = 1;
			continue;
		}
	}

	return ret;
}

static int do_start(char **argv)
{
	int fd;
	int ret;
	int n_task;

	if ((ret = dsptask_dir_check()) != 0)
		return ret;
	if ((ret = do_load(argv)) != 0)
		return ret;

	if ((fd = open(CTLDEVNM, O_RDWR)) < 0) {
		perror("open control device");
		return 1;
	}
	printf("releasing DSP reset\n");
	ioctl(fd, OMAP_DSP_IOCTL_RUN);
	usleep(10000);	/* wait for DSP initialization */
	printf("DSP configuration ... ");
	if (ioctl(fd, OMAP_DSP_IOCTL_DSPCFG) < 0) {
		printf("failed\n");
		perror("");
		return 1;
	}
	printf("succeeded\n");

	if ((n_task = ioctl(fd, OMAP_DSP_IOCTL_TASKCNT)) < 0) {
		perror("TASKCNT");
		return 1;
	}
	close(fd);

	return dev_mklink(n_task);
}

static int do_stop(char **argv)
{
	int fd;
	int n_task;
	int ret = 0;

	if ((fd = open(CTLDEVNM, O_RDWR)) < 0) {
		perror("open control device");
		return 1;
	}

	if ((n_task = ioctl(fd, OMAP_DSP_IOCTL_TASKCNT)) < 0) {
		perror("TASKCNT");
		return 1;
	}
	ret = dev_unlink(n_task);

	printf("releasing resources for DSP\n");
	ioctl(fd, OMAP_DSP_IOCTL_DSPUNCFG);
	printf("DSP reset\n");
	ioctl(fd, OMAP_DSP_IOCTL_RESET);
	close(fd);
	return ret;
}

static int do_version(char **argv)
{
	char *coffname = argv[2];
	struct dspgw_version version;

	if (strcmp(&coffname[strlen(coffname)-4], ".out")) {
		fprintf(stderr, "cofffile name must be *.out!\n");
		return 1;
	}

	if (read_dspgw_version(&version, coffname) < 0)
		printf("DSP Gateway binary version 3.1 or before.\n");
	else
		printf("DSP Gateway binary version %d.%d.%d.%d\n",
		       version.major, version.minor,
		       version.extra1, version.extra2);

	return 0;
}

static int do_load(char **argv)
{
	char *coffname = argv[2];
	unsigned long resetvect_adr;
	int ctl_fd;

	if (strcmp(&coffname[strlen(coffname)-4], ".out")) {
		fprintf(stderr, "cofffile name must be *.out!\n");
		return 1;
	}

	ctl_fd = open(CTLDEVNM, O_RDWR);
	if (ctl_fd < 0) {
		perror("open control device");
		return 1;
	}

	/*
	 * Theoretically this is not needed because kernel sets
	 * no byte-swapping mode, but it is reported that
	 * in OMAP1510, DSP MMU fault happens without this operation.
	 */
	ioctl(ctl_fd, OMAP_DSP_IOCTL_MPUI_BYTESWAP_OFF);

	printf("loading %s...\n", coffname);
	resetvect_adr = load_coff(coffname);

	if (resetvect_adr > 0x00ffffff) {
		fprintf(stderr, "Can't determine reset vector address\n");
		return 1;
	}
	usleep(10000);	/* wait for DSP memory settled */
	ioctl(ctl_fd, OMAP_DSP_IOCTL_RESET);
	printf("setting DSP reset vector to 0x%06lx\n", resetvect_adr);
	ioctl(ctl_fd, OMAP_DSP_IOCTL_SETRSTVECT, resetvect_adr);

	close(ctl_fd);

	return 0;
}

static int do_run(char **argv)
{
	int fd;

	if ((fd = open(CTLDEVNM, O_RDWR)) < 0) {
		perror("open control device");
		return 1;
	}
	printf("releasing DSP reset\n");
	ioctl(fd, OMAP_DSP_IOCTL_RUN);
	close(fd);
	return 0;
}

static int do_reset(char **argv)
{
	int fd;

	if ((fd = open(CTLDEVNM, O_RDWR)) < 0) {
		perror("open control device");
		return 1;
	}
	printf("DSP reset\n");
	ioctl(fd, OMAP_DSP_IOCTL_RESET);
	close(fd);
	return 0;
}

static int do_cleanup(char **argv)
{
	int fd_ctl, fd_mem;
	int n_task;

	if ((fd_ctl = open(CTLDEVNM, O_RDWR)) < 0) {
		perror("open control device");
		return 1;
	}

	if ((n_task = ioctl(fd_ctl, OMAP_DSP_IOCTL_TASKCNT)) < 0)
		perror("TASKCNT");
	else
		dev_unlink(n_task);	/* ignoring the result */

	printf("releasing resources for DSP\n");
	ioctl(fd_ctl, OMAP_DSP_IOCTL_DSPUNCFG);
	printf("DSP reset\n");
	ioctl(fd_ctl, OMAP_DSP_IOCTL_RESET);
	close(fd_ctl);

	if ((fd_mem = open(DSPMEMDEVNM, O_RDWR)) < 0) {
		perror("open dspmem device");
		return 1;
	}

	printf("flush DSP TLB.\n");
	ioctl(fd_mem, OMAP_DSP_MEM_IOCTL_EXMAP_FLUSH);
	printf("initialize DSP MMU.\n");
	ioctl(fd_mem, OMAP_DSP_MEM_IOCTL_MMUINIT);
	close(fd_mem);

	return 0;
}

static int do_setrstvect(char **argv)
{
	int fd;
	unsigned long adr = strtoul(argv[2], NULL, 16);

	if ((fd = open(CTLDEVNM, O_RDWR)) < 0) {
		perror("open control device");
		return 1;
	}
	printf("setting DSP reset vector to 0x%06lx\n", adr);
	ioctl(fd, OMAP_DSP_IOCTL_SETRSTVECT, adr);
	close(fd);
	return 0;
}

static int do_gbl_idle(char **argv)
{
	int fd;

	if ((fd = open(CTLDEVNM, O_RDWR)) < 0) {
		perror("open control device");
		return 1;
	}
	printf("setting DSP global idle\n");
	ioctl(fd, OMAP_DSP_IOCTL_GBL_IDLE);
	close(fd);
	return 0;
}

static int do_cpu_idle(char **argv)
{
	int fd;

	if ((fd = open(CTLDEVNM, O_RDWR)) < 0) {
		perror("open control device");
		return 1;
	}
	printf("setting DSP CPU idle\n");
	ioctl(fd, OMAP_DSP_IOCTL_CPU_IDLE);
	close(fd);
	return 0;
}

static int do_suspend(char **argv)
{
	int fd;

	if ((fd = open(CTLDEVNM, O_RDWR)) < 0) {
		perror("open control device");
		return 1;
	}
	printf("suspending DSP\n");
	ioctl(fd, OMAP_DSP_IOCTL_SUSPEND);
	close(fd);
	return 0;
}

static int do_resume(char **argv)
{
	int fd;

	if ((fd = open(CTLDEVNM, O_RDWR)) < 0) {
		perror("open control device");
		return 1;
	}
	printf("resuming DSP\n");
	ioctl(fd, OMAP_DSP_IOCTL_RESUME);
	close(fd);
	return 0;
}

static int do_dspcfg(char **argv)
{
	int fd;
	int ret;
	int n_task;

	if ((ret = dsptask_dir_check()) != 0)
		return ret;
	if ((fd = open(CTLDEVNM, O_RDWR)) < 0) {
		perror("open control device");
		return 1;
	}
	printf("DSP configuration ...");
	if (ioctl(fd, OMAP_DSP_IOCTL_DSPCFG) < 0) {
		printf("failed\n");
		perror("");
		return 1;
	}
	printf("succeeded\n");

	if ((n_task = ioctl(fd, OMAP_DSP_IOCTL_TASKCNT)) < 0) {
		perror("TASKCNT");
		return 1;
	}
	close(fd);

	return dev_mklink(n_task);
}

static int do_dspuncfg(char **argv)
{
	int fd;
	int n_task;
	int ret = 0;

	if ((fd = open(CTLDEVNM, O_RDWR)) < 0) {
		perror("open control device");
		return 1;
	}

	if ((n_task = ioctl(fd, OMAP_DSP_IOCTL_TASKCNT)) < 0) {
		perror("TASKCNT");
		return 1;
	}
	ret = dev_unlink(n_task);

	printf("releasing resources for DSP\n");
	ioctl(fd, OMAP_DSP_IOCTL_DSPUNCFG);
	close(fd);
	return ret;
}

static int do_poll(char **argv)
{
	int fd;

	if ((fd = open(CTLDEVNM, O_RDWR)) < 0) {
		perror("open control device");
		return 1;
	}
	printf("polling DSP ... ");
	if (ioctl(fd, OMAP_DSP_IOCTL_POLL) < 0) {
		printf("failed\n");
		return 1;
	}
	printf("succeeded\n");
	close(fd);

	return 0;
}

static int do_kmem_reserve(char **argv)
{
	unsigned long size;
	int ret;
	int fd;

	size = strtoul(argv[2], NULL, 16);

	if ((fd = open(DSPMEMDEVNM, O_RDWR)) < 0) {
		perror("open dspmem device");
		return 1;
	}
	printf("reserving kernel memory for DSP: size = 0x%lx\n", size);
	ret = ioctl(fd, OMAP_DSP_MEM_IOCTL_KMEM_RESERVE, &size);
	close(fd);

	if (ret < 0) {
		perror("KMEM_RESERVE");
		return 1;
	}
	if (ret < size) {
		printf("!!! only %d (0x%x) bytes could be reserved.\n", ret, ret);
	}
	printf("%d (0x%x) bytes reserved.\n", ret, ret);
	return 0;
}

static int do_kmem_release(char **argv)
{
	int fd;

	if ((fd = open(DSPMEMDEVNM, O_RDWR)) < 0) {
		perror("open dspmem device");
		return 1;
	}
	printf("releasing kernel memory for DSP\n");
	ioctl(fd, OMAP_DSP_MEM_IOCTL_KMEM_RELEASE);
	close(fd);
	return 0;
}

static int do_exmap(char **argv)
{
	struct omap_dsp_mapinfo mapinfo;
	int ret;
	int fd;

	mapinfo.dspadr = strtoul(argv[2], NULL, 16);
	mapinfo.size   = strtoul(argv[3], NULL, 16);

	if ((fd = open(DSPMEMDEVNM, O_RDWR)) < 0) {
		perror("open dspmem device");
		return 1;
	}
	printf("mapping external memory for DSP: "
	       "dspadr = 0x%06lx, size = 0x%lx\n",
	       mapinfo.dspadr, mapinfo.size);
	ret = ioctl(fd, OMAP_DSP_MEM_IOCTL_EXMAP, &mapinfo);
	close(fd);

	if (ret < 0) {
		perror("EXMAP");
		return 1;
	}
	if (ret < mapinfo.size) {
		printf("!!! only %d (0x%x) bytes could be mapped.\n", ret, ret);
	}
	printf("%d (0x%x) bytes mapped.\n", ret, ret);
	return 0;
}

static int do_exunmap(char **argv)
{
	unsigned long dspadr = strtoul(argv[2], NULL, 16);
	int ret;
	int fd;

	if ((fd = open(DSPMEMDEVNM, O_RDWR)) < 0) {
		perror("open dspmem device");
		return 1;
	}
	printf("releasing external memory map for DSP: "
	       "dspadr = 0x%06lx\n", dspadr);
	ret = ioctl(fd, OMAP_DSP_MEM_IOCTL_EXUNMAP, dspadr);
	close(fd);

	if (ret < 0) {
		perror("EXUNMAP");
		return 1;
	}
	printf("%d (0x%x) bytes unmapped.\n", ret, ret);
	return 0;
}

static int do_mapflush(char **argv)
{
	int fd;

	if ((fd = open(DSPMEMDEVNM, O_RDWR)) < 0) {
		perror("open dspmem device");
		return 1;
	}
	printf("flush DSP TLB.\n");
	ioctl(fd, OMAP_DSP_MEM_IOCTL_EXMAP_FLUSH);
	close(fd);
	return 0;
}

static int do_fbexport(char **argv)
{
	unsigned long dspadr = strtoul(argv[2], NULL, 16);
	int ret;
	int fd;

	if ((fd = open(DSPMEMDEVNM, O_RDWR)) < 0) {
		perror("open dspmem device");
		return 1;
	}
	printf("mapping frame buffer to DSP space:\n"
	       "  dspadr hint   = 0x%06lx\n", dspadr);
	ret = ioctl(fd, OMAP_DSP_MEM_IOCTL_FBEXPORT, &dspadr);
	close(fd);

	if (ret < 0) {
		perror("FBEXPORT");
		return 1;
	}
	printf("  dspadr actual = 0x%06lx\n", dspadr);
	printf("%d (0x%x) bytes mapped.\n", ret, ret);
	return 0;
}

static int do_mmuinit(char **argv)
{
	int fd;

	if ((fd = open(DSPMEMDEVNM, O_RDWR)) < 0) {
		perror("open dspmem device");
		return 1;
	}
	printf("initialize DSP MMU.\n");
	ioctl(fd, OMAP_DSP_MEM_IOCTL_MMUINIT);
	close(fd);
	return 0;
}

static int do_mmuitack(char **argv)
{
	int fd;

	if ((fd = open(DSPMEMDEVNM, O_RDWR)) < 0) {
		perror("open dspmem device");
		return 1;
	}
	printf("sending DSP MMU interrupt acknowledge.\n");
	ioctl(fd, OMAP_DSP_MEM_IOCTL_MMUITACK);
	close(fd);
	return 0;
}

static int do_mkdev(char **argv)
{
	int ret;
	int fd;

	if ((fd = open(TWCHDEVNM, O_RDWR)) < 0) {
		perror("open task watch device");
		return 1;
	}
	printf("create DSP task device.\n");
	ret = ioctl(fd, OMAP_DSP_TWCH_IOCTL_MKDEV, argv[2]);
	close(fd);

	if (ret < 0) {
		perror("MKDEV");
		return 1;
	}
	return 0;
}

static int do_rmdev(char **argv)
{
	int ret;
	int fd;

	if ((fd = open(TWCHDEVNM, O_RDWR)) < 0) {
		perror("open task watch device");
		return 1;
	}
	printf("remove DSP task device.\n");
	ret = ioctl(fd, OMAP_DSP_TWCH_IOCTL_RMDEV, argv[2]);
	close(fd);

	if (ret < 0) {
		perror("RMDEV");
		return 1;
	}
	return 0;
}

static int do_tadd(char **argv)
{
	struct omap_dsp_taddinfo taddinfo;
	int ret;
	int fd;

	taddinfo.minor   = strtoul(argv[2], NULL, 10);
	taddinfo.taskadr = strtoul(argv[3], NULL, 16);

	if ((fd = open(TWCHDEVNM, O_RDWR)) < 0) {
		perror("open task watch device");
		return 1;
	}
	printf("add DSP task.\n");
	ret = ioctl(fd, OMAP_DSP_TWCH_IOCTL_TADD, &taddinfo);
	close(fd);

	if (ret < 0) {
		perror("TADD");
		return 1;
	}
	return 0;
}

static int do_tdel(char **argv)
{
	unsigned char minor = strtoul(argv[2], NULL, 10);
	int ret;
	int fd;

	if ((fd = open(TWCHDEVNM, O_RDWR)) < 0) {
		perror("open task watch device");
		return 1;
	}
	printf("delete DSP task.\n");
	ret = ioctl(fd, OMAP_DSP_TWCH_IOCTL_TDEL, minor);
	close(fd);

	if (ret < 0) {
		perror("TDEL");
		return 1;
	}
	return 0;
}

static int do_getvar(char **argv)
{
	char buf[32]; /* maximum size for omap_dsp_varinfo */
	struct omap_dsp_varinfo *var = (void *)buf;
	int valc;
	int ret;
	int fd;
	int i;

	if ((fd = open(CTLDEVNM, O_RDWR)) < 0) {
		perror("open control device");
		return 1;
	}
	var->varid = strtoul(argv[2], NULL, 16);
	printf("DSP variable read: varid=0x%02x\n", var->varid);
	switch (var->varid) {
	case OMAP_DSP_MBCMD_VARID_ICRMASK:
		valc = 1;
		break;
	case OMAP_DSP_MBCMD_VARID_LOADINFO:
		valc = 5;
		break;
	default:
		fprintf(stderr, "illegal varid\n");
		return 1;
	}
	ret = ioctl(fd, OMAP_DSP_IOCTL_GETVAR, var);
	close(fd);

	if (ret < 0) {
		printf("\n");
		perror("GETVAR");
		return 1;
	}
	for (i = 0; i < valc; i++) {
		printf("  val[%d]=0x%04x\n", i, var->val[i]);
	}
	return 0;
}

static int do_setvar_1(char **argv)
{
	char buf[32]; /* maximum size for omap_dsp_varinfo */
	struct omap_dsp_varinfo *var = (void *)buf;
	int valc = 1;
	int ret;
	int fd;
	int i;

	if ((fd = open(CTLDEVNM, O_RDWR)) < 0) {
		perror("open control device");
		return 1;
	}
	var->varid = strtoul(argv[2], NULL, 16);
	printf("DSP variable write: varid=0x%02x", var->varid);
	switch (var->varid) {
	case OMAP_DSP_MBCMD_VARID_ICRMASK:
		/* valc = 1; */
		break;
	default:
		fprintf(stderr, "illegal varid\n");
		return 1;
	}
	for (i = 0; i < valc; i++) {
		var->val[i] = strtoul(argv[3+i], NULL, 16);
		printf("    val[%d]=0x%04x\n", i, var->val[i]);
	}
	ret = ioctl(fd, OMAP_DSP_IOCTL_SETVAR, var);
	close(fd);

	if (ret < 0) {
		perror("SETVAR");
		return 1;
	}
	return 0;
}

static int do_regread(char **argv)
{
	return __regread(SPACE_MEM, strtoul(argv[2], NULL, 16));
}

static int do_regread_io(char **argv)
{
	if (strcmp(argv[2], "-io")) {
		usage(argv[0]);
		return 1;
	}

	return __regread(SPACE_IO, strtoul(argv[3], NULL, 16));
}

static int __regread(enum regspace space, unsigned short adr)
{
	struct omap_dsp_reginfo reg;
	unsigned int ctlcmd;
	int ret;
	int fd;

	if ((fd = open(CTLDEVNM, O_RDWR)) < 0) {
		perror("open control device");
		return 1;
	}
	printf("DSP register read: adr=%s:0x%04x",
	       (space == SPACE_MEM) ? "MEM" : "IO",  adr);
	ctlcmd = (space == SPACE_MEM) ? OMAP_DSP_IOCTL_REGMEMR :
					OMAP_DSP_IOCTL_REGIOR;
	reg.adr = adr;
	ret = ioctl(fd, ctlcmd, &reg);
	close(fd);

	if (ret < 0) {
		printf("\n");
		perror("REGREAD");
		return 1;
	}
	printf(", val=0x%04x\n", reg.val);
	return 0;
}

static int do_regwrite_io(char **argv)
{
	if (strcmp(argv[2], "-io")) {
		usage(argv[0]);
		return 1;
	}

	return __regwrite(SPACE_IO,
			  strtoul(argv[3], NULL, 16),
			  strtoul(argv[4], NULL, 16));
}

static int do_regwrite(char **argv)
{
	return __regwrite(SPACE_MEM,
			  strtoul(argv[2], NULL, 16),
			  strtoul(argv[3], NULL, 16));
}

static int __regwrite(enum regspace space, unsigned short adr,
		      unsigned short val)
{
	struct omap_dsp_reginfo reg;
	unsigned int ctlcmd;
	int ret;
	int fd;

	if ((fd = open(CTLDEVNM, O_RDWR)) < 0) {
		perror("open control device");
		return 1;
	}
	printf("DSP register write: adr=%s:0x%04x, val=0x%04x\n",
	       (space == SPACE_MEM) ? "MEM" : "IO",  adr, val);
	ctlcmd = (space == SPACE_MEM) ? OMAP_DSP_IOCTL_REGMEMW :
					OMAP_DSP_IOCTL_REGIOW;
	reg.adr = adr;
	reg.val = val;
	ret = ioctl(fd, ctlcmd, &reg);
	close(fd);

	if (ret < 0) {
		perror("REGWRITE");
		return 1;
	}
	return 0;
}

static int do_runlevel(char **argv)
{
	char *level_name = argv[2];
	unsigned char level;
	int fd;

	if (!strcmp(level_name, "user")) {
		level = OMAP_DSP_MBCMD_RUNLEVEL_USER;
	} else if (!strcmp(level_name, "super")) {
		level = OMAP_DSP_MBCMD_RUNLEVEL_SUPER;
	} else {
		printf("unknown runlevel %s\n", level_name);
		return 1;
	}

	if ((fd = open(CTLDEVNM, O_RDWR)) < 0) {
		perror("open control device");
		return 1;
	}
	printf("setting DSP runlevel to %s\n", level_name);
	ioctl(fd, OMAP_DSP_IOCTL_RUNLEVEL, level);
	close(fd);
	return 0;
}

static int do_mbsend(char **argv)
{
	struct omap_dsp_mailbox_cmd mbcmd;
	int fd;

	mbcmd.cmd  = strtoul(argv[2], NULL, 16);
	mbcmd.data = strtoul(argv[3], NULL, 16);

	if ((fd = open(CTLDEVNM, O_RDWR)) < 0) {
		perror("open control device");
		return 1;
	}
	printf("sending mailbox command to DSP: cmd = 0x%02x, data = 0x%02x\n",
	       mbcmd.cmd, mbcmd.data);
	ioctl(fd, OMAP_DSP_IOCTL_MBSEND, &mbcmd);
	close(fd);
	return 0;
}
