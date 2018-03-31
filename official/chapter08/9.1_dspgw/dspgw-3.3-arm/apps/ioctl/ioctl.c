/*
 * dspapps/apps/ioctl/arm/ioctl.c
 *
 * ARM-side frontent program for DSP task ioctl
 *
 * Copyright (C) 2004-2005 Nokia Corporation
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
#include <sys/ioctl.h>

void busywait(int n)
{
	int i;
	for (i = 0; i < n; i++);
}

int issue_ioctl0(int fd, int cmd)
{
	int ret;

	printf("ioctl(0x%04x)\n", cmd);
	busywait(0x400000);
	ret = ioctl(fd, cmd);
	printf("return = %d (0x%04x)\n\n", ret, ret);
	return ret;
}

int issue_ioctl1(int fd, int cmd, int arg)
{
	int ret;

	printf("ioctl(0x%04x, 0x%04x)\n", cmd, arg);
	busywait(0x400000);
	ret = ioctl(fd, cmd, arg);
	printf("return = %d (0x%04x)\n\n", ret, ret);
	return ret;
}

int main(int argc, char **argv)
{
	int fd;

	fd = open("/dev/dsptask/ioctl_test", O_RDWR);
	if (fd < 0) {
		perror("open");
		exit(1);
	}

	issue_ioctl0(fd, 0xa4);
	busywait(0x400000);
	issue_ioctl0(fd, 0x8050);
	busywait(0x400000);
	issue_ioctl1(fd, 0x8112, 0xbeef);
	busywait(0x400000);
	issue_ioctl0(fd, 0x90bb);
	busywait(0x400000);
	issue_ioctl1(fd, 0x91ff, 0xbaca);
	busywait(0x400000);
	issue_ioctl0(fd, 0xdead);

	close(fd);

	return 0;
}
