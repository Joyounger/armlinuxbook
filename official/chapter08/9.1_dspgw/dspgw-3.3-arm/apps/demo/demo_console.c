/*
 * dspapps/apps/demo/arm/demo_console.c
 *
 * ARM-side frontent program for console demo
 *
 * Copyright (C) 2004,2005 Nokia Corporation
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
 * 2004/09/28:  DSP Gateway version 3.3
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

int main(int argc, char **argv)
{
	int fd;
	union {
		short w;
		char c;
	} d;

	fd = open("/dev/dsptask/demo_console", O_RDWR);
	if (fd < 0) {
		perror("open");
		exit(1);
	}

	d.w = 0;
	write(fd, &d.w, 0x2);

	do {
		read(fd, &d.w, 2);
		write(1, &d.c, 1);
	} while (d.c != '\n');

	close(fd);

	return 0;
}
