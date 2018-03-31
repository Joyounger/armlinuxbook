/*
 * dsp_dld/arm/client.c
 *
 * DSP Dynamic Loader Daemon: client.c
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
 * 2005/06/06:  DSP Gateway version 3.3
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include "dsp_dld.h"

int receive_event(int fd);
int receive_string(int fd);

void usage(char *cmd)
{
	fprintf(stderr, "%s <command> ...\n", cmd);
	fprintf(stderr, "\n"
		"command:\n"
		"    tadd <minor>\n"
		"    tdel <minor>\n"
		"    tkill <minor>\n"
		"\n"
		"    run\n"
		"    reset\n"
		"    setrstvect <addr>\n"
		"    cpu_idle\n"
		"    dspcfg\n"
		"    dspuncfg\n"
		"    suspend\n"
		"    resume\n"
		"    exmap <dspadr> <request size>\n"
		"    mmuinit\n"
		"    mapflush\n"
		"    memdump <addr> <len>\n"
		"    mbsend <cmd> <data>\n"
		"    taskent\n"
		"    memmgr\n"
		"    symbol\n"
		"    section\n"
		"    terminate\n"
		"    restart\n");
	return;
}

int getoption(int argc, char *argv[], struct server_event *e)
{
	e->len = argc * 4;

	if (argc < 2) {
		usage(argv[0]);
		return -1;
	}

	/*
	 *  parse option and initiate command
	 */
	if (!strcmp(argv[1], "tadd") && argc == 3) {
		e->event = DLD_EVENT_TADD;
		e->data.task.minor = atoi(argv[2]);
	} else if (!strcmp(argv[1], "tdel") && argc == 3) {
		e->event = DLD_EVENT_TDEL;
		e->data.task.minor = atoi(argv[2]);
	} else if (!strcmp(argv[1], "tkill") && argc == 3) {
		e->event = DLD_EVENT_TKILL;
		e->data.task.minor = atoi(argv[2]);
	} else if (!strcmp(argv[1], "run")) {
		e->event = DLD_EVENT_DSP_RUN;
	} else if (!strcmp(argv[1], "reset")) {
		e->event = DLD_EVENT_DSP_RESET;
	} else if (!strcmp(argv[1], "setrstvect") && argc == 3) {
		e->event = DLD_EVENT_RSTVECT;
		e->data.rstvect.addr = strtoul(argv[2], NULL, 16);
	} else if (!strcmp(argv[1], "cpu_idle")) {
		e->event = DLD_EVENT_CPU_IDLE;
	} else if (!strcmp(argv[1], "dspcfg")) {
		e->event = DLD_EVENT_DSPCFG;
	} else if (!strcmp(argv[1], "dspuncfg")) {
		e->event = DLD_EVENT_DSPUNCFG;
	} else if (!strcmp(argv[1], "suspend")) {
		e->event = DLD_EVENT_SUSPEND;
	} else if (!strcmp(argv[1], "resume")) {
		e->event = DLD_EVENT_RESUME;
	} else if (!strcmp(argv[1], "exmap") && argc == 4) {
		e->event = DLD_EVENT_EXMAP;
		e->data.exmap.dspadr = strtol(argv[2], NULL, 16);
		e->data.exmap.size   = strtol(argv[3], NULL, 16);
	} else if (!strcmp(argv[1], "mmuinit")) {
		e->event = DLD_EVENT_MMUINIT;
	} else if (!strcmp(argv[1], "mapflush")) {
		e->event = DLD_EVENT_MAPFLUSH;
	} else if (!strcmp(argv[1], "mbsend") && argc == 4) {
		e->event = DLD_EVENT_MBSEND;
		e->data.mbsend.cmd  = strtol(argv[2], NULL, 16);
		e->data.mbsend.data = strtol(argv[3], NULL, 16);
	} else if (!strcmp(argv[1], "taskent") && argc == 2) {
		e->event = DLD_EVENT_GETSTAT_TASKENT;
	} else if (!strcmp(argv[1], "memmgr") && argc == 2) {
		e->event = DLD_EVENT_GETSTAT_MEMMGR;
	} else if (!strcmp(argv[1], "symbol") && argc == 2) {
		e->event = DLD_EVENT_GETSTAT_SYMBOL;
	} else if (!strcmp(argv[1], "section") && argc == 2) {
		e->event = DLD_EVENT_GETSTAT_SECTION;
	} else if (!strcmp(argv[1], "memdump") && argc == 4) {
		e->event = DLD_EVENT_MEMDUMP;
		e->data.memdump.addr = strtol(argv[2], NULL, 16);
		e->data.memdump.size = strtol(argv[3], NULL, 16);
	} else if (!strcmp(argv[1], "terminate") && argc == 2) {
		e->event = DLD_EVENT_TERMINATE;
	} else if (!strcmp(argv[1], "restart") && argc == 2) {
		e->event = DLD_EVENT_RESTART;
	} else {
		usage(argv[0]);
		return -1;
	}

	return 0;
}

int main(int argc, char **argv)
{
	int fd;
	struct sockaddr_un addr;
	u32 buf[256];
	struct server_event *e = (struct server_event *)buf;
	int result = 0;

	if (getoption(argc, argv, e))
		return 1;

	if ((fd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) {
		perror("socket");
		return 1;
	}

	addr.sun_family = AF_UNIX;
	strcpy(addr.sun_path, SOCK_NAME);

	if (connect(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
		perror("connect");
		return 1;
	}

	if (write(fd, buf, e->len) < 0) {
		perror("write");
		return 1;
	}

	switch (e->event) {
		case DLD_EVENT_GETSTAT_TASKENT:
		case DLD_EVENT_GETSTAT_MEMMGR:
		case DLD_EVENT_GETSTAT_SYMBOL:
		case DLD_EVENT_GETSTAT_SECTION:
		case DLD_EVENT_MEMDUMP:
			result = receive_string(fd);
			break;
		default:
			result = receive_event(fd);
	}

	close(fd);
	return (result < 0) ? 1 : 0;
}

int receive_event(int fd)
{
	struct server_event e;

	if (read(fd, &e, SERVER_EVENT_HDRSZ) < 0) {
		perror("read");
		return -1;
	}
	switch (e.event) {
		case DLD_EVENT_DONE:
			printf("success.\n");
			break;
		case DLD_EVENT_ERROR:
			printf("error!\n");
			break;
		default:
			printf("unknown packet! (%ld)\n", e.event);
			break;
	}
	return 0;
}

int receive_string(int fd)
{
	struct server_event e;

	do {
		char *buf = NULL;

		if (read(fd, &e, SERVER_EVENT_HDRSZ) <= 0) {
			perror("read");
			return -1;
		}
		if (e.len > SERVER_EVENT_HDRSZ) {
			size_t optsz = e.len - SERVER_EVENT_HDRSZ;
			buf = malloc(optsz+1);
			if (read(fd, buf, optsz) <= 0) {
				perror("read");
				return -1;
			}
			buf[optsz] = '\0';
			if (e.event == DLD_EVENT_STRING)
				printf("%s", buf);
			free(buf);
		}
	} while (e.event == DLD_EVENT_STRING);

	switch (e.event) {
		case DLD_EVENT_DONE:
			printf("success.\n");
			break;
		case DLD_EVENT_ERROR:
			printf("error!\n");
			break;
		default:
			printf("unknown packet! (%ld)\n", e.event);
			break;
	}

	return 0;
}
