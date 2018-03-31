/*
 * dspapps/apps/ex_task/arm/test7.c
 *
 * ARM-side frontent program for DSP task test7
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
 * 2004/06/30:  DSP Gateway version 3.3
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

#define BUFSZ	50

void usage(char *cmd) {
	fprintf(stderr, "%s <repeat_count>\n", cmd);
}

int main(int argc, char **argv) {
	int fd;
	char buf[BUFSZ];
	int cnt1, cnt2;
	char *wdata = "aaaaabbbbbcccccdddddeeeeefffffggggghhhhhiiiiijjjjj";
	char *devfn = "/dev/dsptask/test7";

	fd = open(devfn, O_RDWR);
	if (fd < 0) {
		fprintf(stderr, "cannot open %s\n", devfn);
		return 1;
	}

	cnt1 =  write(fd, wdata, BUFSZ);
	if (cnt1 < 0) {
		perror("write failed");
		return 1;
	}
	fprintf(stderr, "%d bytes written\n", cnt1);
	cnt2 = read(fd, buf, BUFSZ);
	if (cnt2 < 0) {
		perror("read failed");
		return 1;
	}
	fprintf(stderr, "%d bytes read\n", cnt2);
	write(1, buf, cnt2);
	putchar('\n');
	close(fd);

	return 0;
}
