/*
 * dspapps/apps/ex_task/arm/test8.c
 *
 * ARM-side frontent program for DSP task test8
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
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/time.h>

#define BUFSZ	256

void usage(char *cmd) {
	fprintf(stderr, "%s <repeat_count>\n", cmd);
}

int dif_str(char *s, struct timeval *tm1, struct timeval *tm2) {
	int scnt;
	int timdiff;

	if (tm2->tv_sec || tm2->tv_usec) {	/* non-zero */
		timdiff = (tm1->tv_sec  - tm2->tv_sec) * 100 +
			  (tm1->tv_usec - tm2->tv_usec) / 10000;
		scnt = sprintf(s, "time : %ld.%02ld (+%2d.%02d)  ",
			       tm1->tv_sec, tm1->tv_usec / 10000,
			       timdiff / 100, timdiff % 100);
	} else {
		scnt = sprintf(s, "time : %ld.%02ld           ",
			       tm1->tv_sec, tm1->tv_usec / 10000);
	}
	return scnt;
}

int main(int argc, char **argv) {
	int fd1, fd2, fd3;
	unsigned short repeat;
	char *devfn1 = "/dev/dsptask/test8a";
	char *devfn2 = "/dev/dsptask/test8b";
	char *devfn3 = "/dev/dsptask/test8c";
	fd_set fdset;
	struct timeval tm1a, tm2a, tm1b, tm2b, tm1c, tm2c;

	if (argc != 2) {
		usage(argv[0]);
		return 1;
	}

	repeat = atoi(argv[1]);

	fd1 = open(devfn1, O_RDWR);
	if (fd1 < 0) {
		fprintf(stderr, "cannot open %s\n", devfn1);
		return 1;
	}
	fd2 = open(devfn2, O_RDWR);
	if (fd2 < 0) {
		fprintf(stderr, "cannot open %s\n", devfn2);
		return 1;
	}
	fd3 = open(devfn3, O_RDWR);
	if (fd3 < 0) {
		fprintf(stderr, "cannot open %s\n", devfn3);
		return 1;
	}

	memset(&tm2a, 0, sizeof(struct timeval));
	memset(&tm2b, 0, sizeof(struct timeval));
	memset(&tm2c, 0, sizeof(struct timeval));

	FD_ZERO(&fdset);
	FD_SET(fd1, &fdset);
	FD_SET(fd2, &fdset);
	FD_SET(fd3, &fdset);

	write(fd1, &repeat, 2);
	write(fd2, &repeat, 2);
	write(fd3, &repeat, 2);

	for (;;) {
		fd_set testfds;
		int result, cnt;
		char buf[BUFSZ];
		char s[64];
		int scnt;

		testfds = fdset;
		result = select(fd3+1, &testfds,
				(fd_set *)0, (fd_set *)0, NULL);
		if (result < 0) {
			perror("select failed");
			break;
		}

		if (FD_ISSET(fd1, &testfds)) {
			cnt = read(fd1, buf, BUFSZ);
			if (cnt < 0) {
				perror("read failed");
				break;
			}
			gettimeofday(&tm1a, NULL);
			scnt = dif_str(s, &tm1a, &tm2a);
			write(1, s, scnt);
			write(1, buf, cnt);
			memcpy(&tm2a, &tm1a, sizeof(struct timeval));
		}

		if (FD_ISSET(fd2, &testfds)) {
			cnt = read(fd2, buf, BUFSZ);
			if (cnt < 0) {
				perror("read failed");
				break;
			}
			gettimeofday(&tm1b, NULL);
			scnt = dif_str(s, &tm1b, &tm2b);
			write(1, s, scnt);
			write(1, buf, cnt);
			memcpy(&tm2b, &tm1b, sizeof(struct timeval));
		}

		if (FD_ISSET(fd3, &testfds)) {
			cnt = read(fd3, buf, BUFSZ);
			if (cnt < 0) {
				perror("read failed");
				break;
			}
			gettimeofday(&tm1c, NULL);
			scnt = dif_str(s, &tm1c, &tm2c);
			write(1, s, scnt);
			write(1, buf, cnt);
			memcpy(&tm2c, &tm1c, sizeof(struct timeval));
		}
	}
	close(fd1);
	close(fd2);
	close(fd3);

	return 0;
}
