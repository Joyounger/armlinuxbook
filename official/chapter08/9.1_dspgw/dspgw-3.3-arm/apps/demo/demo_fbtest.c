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
	int d = 0xffff;

	fd = open("/dev/dsptask/fbtest", O_RDWR);
	if (fd < 0) {
		perror("open");
		exit(1);
	}

	write(fd, &d , 0x2);
	read(fd, &d, 2);
	if( d != 0x0){
	        printf("start application on dsp first!");
		exit(1);
	}
	
	printf("demo_fbtest , please look at the lcd\n");
	write(fd, &d, 0x2);

	close(fd);

	return 0;
}
