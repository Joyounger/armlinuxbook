/*
 * dspapps/apps/ex_task/arm/hellotimer.c
 *
 * ARM-side frontent program for DSP task hallotimer
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
 * 2005/02/25:  DSP Gateway version 3.3
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/time.h>

#define BUFSZ	256

void usage(char *cmd)
{
	fprintf(stderr, "%s <repeat_count>\n", cmd);
}

int main(int argc, char **argv)
{
	int fd;
	unsigned short repeat;
	unsigned short i;
	char *devfn = "/dev/dsptask/hellotimer";

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
	write(fd, &repeat, 2);
	for (i = 0; i < repeat; i++) {
		int cnt;
		char buf[BUFSZ];
		struct timeval tm;
		char s[64];
		int scnt;
		char lf = '\n';

		cnt = read(fd, buf, BUFSZ);
		if (cnt < 0) {
			perror("read failed");
			break;
		}
		gettimeofday(&tm, NULL);
		scnt = sprintf(s, "time: %ld.%02ld  ",
			       tm.tv_sec, tm.tv_usec/10000);
		write(1, s, scnt);
		write(1, buf, cnt);
		write(1, &lf, 1);
	}
	ioctl(fd, 0x8001); /* stop */
	close(fd);

	return 0;
}
