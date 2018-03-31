/*
 * dspapps/apps/ex_task/arm/test6.c
 *
 * ARM-side frontent program for DSP task test6
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
 * Toshihiro Kobayashi <toshihiro.kobayashi@nokia.com>
 * 2004/06/30:  DSP Gateway version 3.3
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <asm/arch/dsp.h>

#define BUFSZ	20

void usage(char *cmd) {
	fprintf(stderr, "%s <repeat_count>\n", cmd);
}

int main(int argc, char **argv) {
	int fd;
	unsigned short repeat;
	char buf[BUFSZ];
	int i;
	fd_set fdset;
	char *wdata = ".A.B.C.D.E.F.G.H.I.J";
	char *devfn = "/dev/dsptask/test6";

	if (argc != 2) {
		usage(argv[0]);
		return 1;
	}

	repeat = atoi(argv[1]);

	fd = open(devfn, O_RDWR);
	if (fd < 0) {
		fprintf(stderr, "cannot open %s\n", devfn);
		return 1;
	}

	FD_ZERO(&fdset);
	FD_SET(fd, &fdset);

	ioctl(fd, OMAP_DSP_MBCMD_TCTL_TEN);
	for (i = 0; i < repeat;) {
		fd_set rfds = fdset, wfds = fdset;
		int cnt1, cnt2;
		int result;
		
		result = select(fd+1, &rfds, &wfds, (fd_set*)0, NULL);
		if (result < 0) {
			perror("select failed");
			break;
		}

		if (FD_ISSET(fd, &wfds)) {
			cnt1 = write(fd, wdata, BUFSZ);
			if (cnt1 < 0) {
				perror("write failed");
				break;
			}
		}
		if (FD_ISSET(fd, &rfds)) {
			cnt2 = read(fd, buf, BUFSZ);
			if (cnt2 < 0) {
				perror("read failed");
				break;
			}
			write(1, buf, cnt2);
			putchar('\n');
			i++;
		}
	}
	ioctl(fd, OMAP_DSP_MBCMD_TCTL_TDIS);
	close(fd);

	return 0;
}
