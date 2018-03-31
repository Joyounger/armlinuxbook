/*
 * dspapps/apps/ex_task/arm/test4.c
 *
 * ARM-side frontent program for DSP task test4
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

#define DEFAULT_BUFSZ	1024

void usage(char *cmd) {
	fprintf(stderr, "%s <repeat_count> [buffer size]\n", cmd);
}

int main(int argc, char **argv) {
	int fd;
	unsigned short repeat;
	size_t bufsz = DEFAULT_BUFSZ;
	char *buf;
	char *devfn = "/dev/dsptask/test4";

	if ((argc != 2) && (argc != 3)) {
		usage(argv[0]);
		return 1;
	}

	repeat = atoi(argv[1]);
	if (argc == 3) {
		bufsz = atoi(argv[2]);
		if (bufsz == 0) {
			fprintf(stderr,
				"invalid buffer size (%d)\n", bufsz);
		}
	}
	buf = malloc(bufsz);

	fd = open(devfn, O_RDWR);
	if (fd < 0) {
		fprintf(stderr, "cannot open %s\n", devfn);
		return 1;
	}
	printf("reading with %d bytes buffer...\n", bufsz);
	write(fd, &repeat, 2);
	for (;;) {
		int cnt;
		cnt = read(fd, buf, bufsz);
		if(cnt < 0) {
			perror("read failed");
			break;
		}
		write(1, buf, cnt);
	}
	close(fd);

	free(buf);

	return 0;
}
