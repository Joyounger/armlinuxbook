/*
 * dsp_dld/arm/dld_daemon.c
 *
 * DSP Dynamic Loader Daemon: dld_daemon.c
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
 * 2005/03/23:  DSP Gateway version 3.3
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <syslog.h>
#include "dsp_dld.h"

static int g_daemon = 0;

void prmsg(const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	if (g_daemon) {
		char s[256];

		vsnprintf(s, 256, fmt, ap);
		if (s[254])
			s[254] = '\n';	/* force line feed */
		/* TODO: consider the priority */
		syslog(LOG_INFO, "%s\n", s);
	} else {
		vfprintf(stderr, fmt, ap);
	}
	va_end(ap);
}

int daemon_config_check(struct dld_conf *conf)
{
	if (conf->knlfn[0] != '/') {
		prmsg("kernel file name '%s' is not absolute path.\n",
		      conf->knlfn);
		return -1;
	}
	if (conf->cmdfn[0] != '/') {
		prmsg("global command file name '%s' is not absolute path.\n",
		      conf->cmdfn);
		return -1;
	}

	return 0;
}

int daemon_open(void)
{
	pid_t pid;

	if ((pid = fork()) < 0) {
		perror("fork");
		return pid;
	}
	if (pid != 0)
		/* parent exits here */
		exit(0);

	/* child */
	setsid();	/* session leader */
	chdir("/");
	umask(0);

	/* close stdios */
	close(0);
	close(1);
	close(2);

	/* send log message */
	openlog("dsp_dld", 0, LOG_DAEMON);

	g_daemon = 1;

	return 0;
}

int daemon_close(void)
{
	if (g_daemon)
		closelog();
	return 0;
}
