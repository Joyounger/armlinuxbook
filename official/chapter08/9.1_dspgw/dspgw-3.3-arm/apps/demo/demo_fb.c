/*
 * dspapps/apps/demo/arm/demo_fb.c
 *
 * test for disabling FB update
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
 * 2004/12/15:  DSP Gateway version 3.3
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/ioctl.h>

struct fbinfo {
	unsigned short adrh;
	unsigned short adrl;
	unsigned short width;
	unsigned short height;
	unsigned short bpp;
} fbinfo = {
	.adrh = 0,	// init with illegal adr
	.adrl = 0,
#if defined(INNOVATOR)
	.width = 240,
	.height = 320,
	.bpp = 16,
#else
#error architecture is not specified!
#endif
};

enum mode { MODE_TIMER, MODE_BUSY } mode = MODE_TIMER;
int time = 10;

void usage(char *cmd)
{
	fprintf(stderr,
		"usage:\n"
		"    %s fbadr=<adr> [time=<sec>] [-busy]\n"
		"\n"
		"    or\n"
		"\n"
		"    %s [-h]\n",
	        cmd, cmd);
}

void chk_arg(int argc, char **argv)
{
	unsigned long fbadr = 0xffffffff;
	int i;

	for (i = 1; i < argc; i++) {
		if (!strncmp(argv[i], "time=", 5)) {
			time = atoi(&argv[i][5]);
		} else if (!strncmp(argv[i], "fbadr=", 6)) {
			fbadr = strtoul(&argv[i][6], NULL, 16);
			fbadr >>= 1;	 // word address
		} else if (!strcmp(argv[i], "-busy")) {
			mode = MODE_BUSY;
		} else if (!strcmp(argv[i], "-h")) {
			usage(argv[0]);
			exit(0);
		} else {
			usage(argv[0]);
			exit(1);
		}
	}

	if (fbadr > 0x1000000) {
		fprintf(stderr, "specify legal FB address.\n\n");
		usage(argv[0]);
		exit(1);
	}

	fbinfo.adrh = fbadr >> 16;
	fbinfo.adrl = fbadr & 0xffff;
}

int main(int argc, char **argv)
{
	int fd;

	chk_arg(argc, argv);

	fd = open("/dev/dsptask/demo_fb", O_RDWR);
	if (fd < 0) {
		perror("open");
		exit(1);
	}

	write(fd, &fbinfo, sizeof(struct fbinfo));
	if (mode == MODE_TIMER)
		ioctl(fd, 0x8000);	/* start with timer mode */
	else
		ioctl(fd, 0x8002);	/* start with busy mode */

	if (time < 0)
		select(0, NULL, NULL, NULL, 0);	/* sleep forever */
	else
		sleep(time);

	ioctl(fd, 0x8001);	/* stop */
	close(fd);

	return 0;
}
