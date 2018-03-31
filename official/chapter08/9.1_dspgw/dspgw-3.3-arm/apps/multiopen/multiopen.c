/*
 * dspapps/apps/multiopen/arm/multiopen.c
 *
 * ARM-side frontent program for DSP task device lock test
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
 * 2004/06/30:  DSP Gateway version 3.3
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <asm/arch/dsp.h>

void parent(void);
void child(void);

int main(int argc, char **argv)
{
	if (fork() == 0)
		child();
	else
		parent();
	return 0;
}

void parent(void)
{
	unsigned short data = 0x5abc, buf;
	int fd;
	int ret;
	pid_t pid = getpid();

	printf("[%d] open() req\n", pid);
	fd = open("/dev/dsptask/multiopen", O_RDWR);
	if (fd < 0) {
		perror("open");
		exit(1);
	}
	printf("[%d] open() done\n", pid);

	printf("[%d] ioctl(LOCK) req\n", pid);
	ret = ioctl(fd, OMAP_DSP_TASK_IOCTL_LOCK);
	if (ret < 0) {
		perror("ioctl");
		exit(1);
	}
	printf("[%d] ioctl(LOCK) done\n", pid);

	sleep(1);

	printf("[%d] write() req\n", pid);
	ret = write(fd, &data, 2);
	if (ret < 0) {
		perror("write");
	}
	printf("[%d] write() done\n", pid);

	printf("[%d] read() req\n", pid);
	ret = read(fd, &buf, 2);
	if (ret < 0) {
		perror("read");
	}
	printf("[%d] read() done\n", pid);

	sleep(3);

	printf("[%d] ioctl(UNLOCK) req\n", pid);
	ret = ioctl(fd, OMAP_DSP_TASK_IOCTL_UNLOCK);
	if (ret < 0) {
		perror("ioctl");
		exit(1);
	}
	printf("[%d] ioctl(UNLOCK) done\n", pid);

	sleep(1);

	printf("[%d] ioctl(LOCK) req\n", pid);
	ret = ioctl(fd, OMAP_DSP_TASK_IOCTL_LOCK);
	if (ret < 0) {
		perror("ioctl");
		exit(1);
	}
	printf("[%d] ioctl(LOCK) done\n", pid);

	printf("[%d] ioctl(UNLOCK) req\n", pid);
	ret = ioctl(fd, OMAP_DSP_TASK_IOCTL_UNLOCK);
	if (ret < 0) {
		perror("ioctl");
		exit(1);
	}
	printf("[%d] ioctl(UNLOCK) done\n", pid);

	printf("[%d] close() req\n", pid);
	close(fd);
	printf("[%d] close() done\n", pid);
}

void child(void)
{
	unsigned short data = 0x9999, buf;
	int fd;
	int ret;
	pid_t pid = getpid();

	sleep(1);

	printf("\t\t[%d] open() req\n", pid);
	fd = open("/dev/dsptask/multiopen", O_RDWR);
	if (fd < 0) {
		perror("open");
		exit(1);
	}
	printf("\t\t[%d] open() done\n", pid);

	printf("\t\t[%d] write() req\n", pid);
	ret = write(fd, &data, 2);
	if (ret < 0) {
		perror("write");
	}
	printf("\t\t[%d] write() done\n", pid);

	printf("\t\t[%d] read() req\n", pid);
	ret = read(fd, &buf, 2);
	if (ret < 0) {
		perror("read");
	}
	printf("\t\t[%d] read() done\n", pid);

	printf("\t\t[%d] ioctl(LOCK) req\n", pid);
	ret = ioctl(fd, OMAP_DSP_TASK_IOCTL_LOCK);
	if (ret < 0) {
		perror("ioctl");
		exit(1);
	}
	printf("\t\t[%d] ioctl(LOCK) done\n", pid);

	printf("\t\t[%d] write() req\n", pid);
	ret = write(fd, &data, 2);
	if (ret < 0) {
		perror("write");
	}
	printf("\t\t[%d] write() done\n", pid);

	printf("\t\t[%d] read() req\n", pid);
	ret = read(fd, &buf, 2);
	if (ret < 0) {
		perror("read");
	}
	printf("\t\t[%d] read() done\n", pid);

	sleep(3);

	printf("\t\t[%d] close() req\n", pid);
	close(fd);
	printf("\t\t[%d] close() done\n", pid);
}
