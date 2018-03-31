/*
 * dspapps/apps/ex_task/arm/test2.c
 *
 * ARM-side frontent program for DSP task test2
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
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#define BUFSZ	256

void t1(int fd);
void t2(int fd);
void t3(int fd);

void usage(char *cmd)
{
	fprintf(stderr, "%s\n", cmd);
}

char *data01 = "data01data01data01";
char *data02 = "data02data02";
char *data03 = "data03data03data03";
char *data04 = "data04data04";
char *data05 = "data05data05data05";

int main(int argc, char **argv)
{
	char *devfn = "/dev/dsptask/test2";
	int fd;
	int i;

	fd = open(devfn, O_RDWR);
	if (fd < 0) {
		fprintf(stderr, "cannot open %s\n", devfn);
		return 1;
	}

	t1(fd);
	t2(fd);
	t3(fd);

	for (i = 0; i < 20; i++) {
		t1(fd);
		usleep(1000);
		t2(fd);
		usleep(1000);
		t3(fd);
		usleep(1000);
	}

	close(fd);

	return 0;
}

void t1(int fd)
{
	int len;
	int cnt;
	char buf[BUFSZ];

	printf("test1: write and read\n");

	len = strlen(data01);
	cnt = write(fd, data01, len);
	printf("written \"%s\", %d bytes\n", data01, cnt);
	cnt = read(fd, buf, len);
	buf[cnt] = '\0';
	printf("read    \"%s\", %d bytes\n", buf, cnt);

	len = strlen(data02);
	cnt = write(fd, data02, len);
	printf("written \"%s\", %d bytes\n", data02, cnt);
	cnt = read(fd, buf, len);
	buf[cnt] = '\0';
	printf("read    \"%s\", %d bytes\n", buf, cnt);

	putchar('\n');
}

void t2(int fd)
{
	int cnt;

	printf("test2: write flood\n");

	cnt = write(fd, data01, strlen(data01));
	printf("written \"%s\", %d bytes\n", data01, cnt);
	cnt = write(fd, data02, strlen(data02));
	printf("written \"%s\", %d bytes\n", data02, cnt);
	cnt = write(fd, data03, strlen(data03));
	printf("written \"%s\", %d bytes\n", data03, cnt);
	cnt = write(fd, data04, strlen(data04));
	printf("written \"%s\", %d bytes\n", data04, cnt);
	cnt = write(fd, data05, strlen(data05));
	printf("written \"%s\", %d bytes\n", data05, cnt);

	putchar('\n');
}

void t3(int fd)
{
	int len;
	int cnt;
	char buf[BUFSZ];

	printf("test3: read flood\n");

	len = strlen(data01);
	cnt = write(fd, data01, len);
	printf("written \"%s\", %d bytes\n", data01, cnt);
	cnt = read(fd, buf, len);
	buf[cnt] = '\0';
	printf("read    \"%s\", %d bytes\n", buf, cnt);
	cnt = read(fd, buf, len);
	buf[cnt] = '\0';
	printf("read    \"%s\", %d bytes\n", buf, cnt);
	cnt = read(fd, buf, len);
	buf[cnt] = '\0';
	printf("read    \"%s\", %d bytes\n", buf, cnt);
	cnt = read(fd, buf, len);
	buf[cnt] = '\0';
	printf("read    \"%s\", %d bytes\n", buf, cnt);
	cnt = read(fd, buf, len);
	buf[cnt] = '\0';
	printf("read    \"%s\", %d bytes\n", buf, cnt);

	putchar('\n');
}
