/*
 * dspapps/apps/mmap/arm/mmaptest.c
 *
 * ARM-side frontent program for mmap test
 *
 * Copyright (C) 2004-2005 Nokia Corporation
 *
 * Written by Hiroo Ishikawa <ext-hiroo.ishikawa@nokia.com>
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

#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>

/*
 * This test writes numbers at randam position in the mapped area.
 */

#define PROG_NAME	"mmaptest"

int g_sticky;

void usage(void)
{
	fprintf(stderr, "usage: %s [-h] [-sticky]\n", PROG_NAME);
}

void read_arg(int argc, char *argv[])
{
	int i;

	g_sticky = 0;

	for (i = 1; i < argc; i++) {
		if (!strcmp(argv[i], "-h")) {
			usage();
			exit(0);
		} else if (!strcmp(argv[i], "-sticky")) {
			g_sticky = 1;
		} else {
			usage();
			exit(1);
		}
	}
}

int main(int argc, char *argv[])
{
	unsigned short *addr;
	#define ALEN	8
	unsigned short array[ALEN] = { 1234, 1, 2, 3, 4, 5, 6, 7 };
	unsigned short cmdbuf[2];
	#define OFF	702	/* arbitrary */
	unsigned short len;
	unsigned short off;
	int dsp_fd;
	int i;

	read_arg(argc, argv);

	if ((dsp_fd = open("/dev/dsptask/mmap", O_RDWR)) < 0) {
		perror("open");
		return -1;
	}

	printf("[ARM] mmapping...\n");
	addr = (unsigned short *)mmap(NULL, 4096, PROT_READ | PROT_WRITE, 
				      MAP_SHARED, dsp_fd, 0);
	if ((int)addr < 0) {
		perror("mmap");
		return -1;
	}
	printf("[ARM] mmapped address in user space = %p\n", addr);
	
	printf("[ARM] copying data to the mmapped area:\n"
	       "      offset = %d, len = %d\n", OFF, ALEN);
	memcpy(&addr[OFF], array, ALEN * sizeof(short));
	for (i = 0; i < ALEN; i++) {
		printf("[ARM] addr[%d] = %d\n", OFF+i, addr[OFF+i]);
	}

	printf("\n"
	       "[ARM] inform DSP that we have written the data "
	       "to the shared buffer...\n");
	cmdbuf[0] = OFF;	/* offset within the mmapped space */
	cmdbuf[1] = ALEN;	/* len (in word count) */
	if (write(dsp_fd, cmdbuf, 4) < 0) {
		perror("write");
		return -1;
	}

	printf("\n"
	       "[ARM] Then, wait for DSP until it modifies the data.\n");
	if (read(dsp_fd, cmdbuf, 4) < 0) {
		perror("read");
		return -1;
	}
	off = cmdbuf[0];
	len = cmdbuf[1];
	printf("\n"
	       "[ARM] received: offset = %d, len = %d\n", off, len);

	for (i = 0; i < len; i++) {
		printf("[ARM] addr[%d] = %d\n", off+i, addr[off+i]);
	}

	if (g_sticky)
		getchar();

	munmap(addr, 4096);
	close(dsp_fd);
	return 0;
}
