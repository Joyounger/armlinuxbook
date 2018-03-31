/*
 * dspapps/utils/dspload.c
 *
 * control utility for DSP Gateway
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
 * 2004/12/28:  DSP Gateway version 3.3
 */

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <malloc.h>
#include <fcntl.h>
#include <assert.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include "dspctl.h"

#define BAD_ENTRYPTR	0xffffffff

#define DARAM_BASE	0x000000
#define DARAM_END	0x010000
#define SARAM_BASE	0x010000
#define SARAM_END	0x028000

#define SPACE_DARAM	1
#define SPACE_SARAM	2
#define SPACE_EXTERN	3
#define SPACE_CROSSING	-2

enum scntyp {
	SCNTYP_LOAD,
	SCNTYP_NOLOAD,
	SCNTYP_NODATA,
	SCNTYP_ZEROSZ,
	SCNTYP_CINIT_RAM,
};

struct coff {
	unsigned char *src;
	off_t size;
	COFF_AOUTHDR *aouthdr;
	COFF_FILHDR *coffhdr;
	COFF_SCNHDR *scnhdr;
	COFF_SYMENT *symtbl;
	char *strtbl;
};

static struct coff coff;

static int open_coff(char *coffname);
static int close_coff(int fd);
static int load_scns(void);
static char *scnname(COFF_SCNHDR *hdr);
static COFF_SCNHDR *find_scn_by_name(char *name);
static enum scntyp get_scntyp(COFF_SCNHDR *hdr);
static int spaceof(unsigned long addr, size_t size);
static int load_to_dspmem(int fd, unsigned long addr, size_t size,
		 	  unsigned char *data);
static void copy_byteswap(void *dst, void *src, size_t size);
static int cinit_bss_init(int fd, unsigned char *p, size_t size);

/*
 * load_coff():
 * load the DSP binary into the DSP memory.
 */
unsigned long load_coff(char *coffname)
{
	int fd;
	int ret = BAD_ENTRYPTR;

	if ((fd = open_coff(coffname)) < 0)
		return BAD_ENTRYPTR;

	if (!coff.aouthdr)
		goto out;

	if (load_scns() < 0)
		goto out;

	ret = COFF_LONG(coff.aouthdr->entry);
out:
	close_coff(fd);
	return ret;
}

/*
 * read_dspgw_version():
 * read version info in the DSP binary.
 */
struct COFF_dspgw_version {
	char major[2];
	char minor[2];
	char extra1[2];
	char extra2[2];
};
#define COFF_DSPGW_VERSION struct COFF_dspgw_version
#define COFF_DSPGW_VERSION_SZ	8

int read_dspgw_version(struct dspgw_version *version, char *coffname)
{
	int fd;
	COFF_SCNHDR *version_hdr;
	unsigned long ptr;
	size_t size;
	COFF_DSPGW_VERSION *data;
	int ret = -1;

	if ((fd = open_coff(coffname)) < 0)
		return -1;

	if (!coff.aouthdr)
		goto out;

	/* read version info */
	if ((version_hdr = find_scn_by_name("dspgw_version")) == NULL)
		goto out;
	ptr   = COFF_LONG(version_hdr->s_scnptr);
	size  = COFF_LONG(version_hdr->s_size);
	if (!ptr || (size < COFF_DSPGW_VERSION_SZ))
		goto out;
	data = (COFF_DSPGW_VERSION *)&coff.src[ptr];
	version->major  = COFF_SHORT_H(data->major);
	version->minor  = COFF_SHORT_H(data->minor);
	version->extra1 = COFF_SHORT_H(data->extra1);
	version->extra2 = COFF_SHORT_H(data->extra2);
	ret = 0;
out:
	close_coff(fd);
	return ret;
}

static int open_coff(char *coffname)
{
	int fd;
	struct stat stat;
	unsigned short opthdrsz;
	unsigned long nsyms;
	off_t symptr;

	fd = open(coffname, O_RDONLY);
	if (fd < 0) {
		perror("open");
		return -1;
	}

	if (fstat(fd, &stat) < 0) {
		perror("fstat");
		return -1;
	}

	coff.size = stat.st_size;
	coff.src = mmap(0, coff.size, PROT_READ, MAP_SHARED, fd, 0);
	if ((int)coff.src < 0) {
		perror("mmap");
		return -1;
	}

	coff.coffhdr = (COFF_FILHDR*)&coff.src[0];
	opthdrsz = COFF_SHORT(coff.coffhdr->f_opthdr);
	nsyms = COFF_LONG(coff.coffhdr->f_nsyms);
	symptr = COFF_LONG(coff.coffhdr->f_symptr);

	/* a.out header */
	coff.aouthdr = opthdrsz ? (COFF_AOUTHDR*)&coff.src[COFF_FILHSZ] : NULL;

	/* section header */
	coff.scnhdr = (COFF_SCNHDR*)&coff.src[COFF_FILHSZ + opthdrsz];

	/* string table */
	coff.strtbl = &coff.src[symptr + COFF_SYMESZ * nsyms];

	return fd;
}

static int close_coff(int fd)
{
	munmap(coff.src, coff.size);
	close(fd);
	return 0;
}

static int load_scns(void)
{
	int fd;
	char *msg;
	int ret = 0;
	int i;
	unsigned short nscns = COFF_SHORT(coff.coffhdr->f_nscns);

	fd = open(DSPMEMDEVNM, O_RDWR);
	if (fd < 0) {
		perror("open dspmem device");
		return -1;
	}

	for (i = 0; i < nscns; i++) {
		COFF_SCNHDR *hdr = &coff.scnhdr[i];
		unsigned long vaddr, ptr;
		size_t size;

		vaddr = COFF_LONG(hdr->s_vaddr);
		ptr   = COFF_LONG(hdr->s_scnptr);
		size  = COFF_LONG(hdr->s_size);

		switch (get_scntyp(hdr)) {
			case SCNTYP_CINIT_RAM:
				/* .cinit will be treated later */
				continue;
			case SCNTYP_NOLOAD:
				msg = " ... ignored.";
				break;
			case SCNTYP_ZEROSZ:
			case SCNTYP_NODATA:
				msg = "";
				break;
			case SCNTYP_LOAD:
			{
				unsigned char *data;
				int size_loaded;

				if (ptr) {
					data = &coff.src[ptr];
				} else {
					/* .bss */
					data = malloc(size);
					memset(data, 0, size);
				}
				size_loaded = load_to_dspmem(fd, vaddr, size, data);
				if (!ptr)
					free(data);
				if (size_loaded == size) {
					msg = " ... initialized.";
				} else {
					msg = " ... initialization failed!!";
					ret = -1;
				}
			}
		}
		printf("%-15s : adr = 0x%06lx, size=%5d %s\n",
		       scnname(hdr), vaddr, size, msg);
		if (ret < 0)
			goto finish;
	}

	/* RAM Mode .cinit handling */
	for (i = 0; i < nscns; i++) {
		COFF_SCNHDR *hdr = &coff.scnhdr[i];
		unsigned long vaddr, ptr;
		size_t size;

		if (get_scntyp(hdr) != SCNTYP_CINIT_RAM)
			continue;

		vaddr = COFF_LONG(hdr->s_vaddr);
		ptr   = COFF_LONG(hdr->s_scnptr);
		size  = COFF_LONG(hdr->s_size);

		if (cinit_bss_init(fd, &coff.src[ptr], size) < 0) {
			msg = "  ... .bss variables initialization failed!!";
			ret = -1;
		} else {
			msg = "  ... .bss variables are initialized.";
		}
		printf("%-15s : adr = 0x%06lx, size=%5d %s\n",
		       scnname(hdr), vaddr, size, msg);
		if (ret < 0)
			goto finish;
	}

finish:
	close(fd);
	return ret;
}

static char *scnname(COFF_SCNHDR *hdr)
{
        static char nm[9];  

        if (hdr->s_name[0] == 0) { 
                unsigned long idx = COFF_LONG((&hdr->s_name[4]));
                return &coff.strtbl[idx];
        } else {
                strncpy(nm, hdr->s_name, 8);
                nm[8] = '\0'; 
                return nm;
        }
}

static COFF_SCNHDR *find_scn_by_name(char *name)
{
	int i;
	unsigned short nscns = COFF_SHORT(coff.coffhdr->f_nscns);

	for (i = 0; i < nscns; i++) {
		if (!strcmp(scnname(&coff.scnhdr[i]), name))
			return &coff.scnhdr[i];
	}
	return NULL;
}

static enum scntyp get_scntyp(COFF_SCNHDR *hdr)
{
	unsigned long ptr;
	unsigned long flags;
	size_t size;

	ptr   = COFF_LONG(hdr->s_scnptr);
	size  = COFF_LONG(hdr->s_size);
	flags = COFF_LONG(hdr->s_flags);

	if (size == 0)
		return SCNTYP_ZEROSZ;

	if (ptr == 0) {
		if (flags & COFF_STYP_BSS)
			return SCNTYP_LOAD;
		else
			return SCNTYP_NODATA;
	}

	if (flags & (COFF_STYP_DSECT | COFF_STYP_NOLOAD | COFF_STYP_PAD))
		return SCNTYP_NOLOAD;

	if (flags & COFF_STYP_COPY) {
		if (!strcmp(scnname(hdr), ".cinit"))
			return SCNTYP_CINIT_RAM;
		else
			return SCNTYP_NOLOAD;
	}

	return SCNTYP_LOAD;
}

static int spaceof(unsigned long addr, size_t size)
{
	if ((addr >= DARAM_BASE) && (addr < DARAM_END)) {
		if (addr + size > DARAM_END)
			return SPACE_CROSSING;
		else
			return SPACE_DARAM;
	} else if ((addr >= SARAM_BASE) && (addr < SARAM_END)) {
		if (addr + size > SARAM_END)
			return SPACE_CROSSING;
		else
			return SPACE_SARAM;
	} else {
		return SPACE_EXTERN;
	}
}

static int load_to_dspmem(int fd, unsigned long addr, size_t size,
			  unsigned char *data)
{
	/*
	 * We assume MPUI byteswap off for DARAM/SARAM.
	 * You should be careful that the copy_from_user routine
	 * for 1,2 and 3-bytes is tricky through MPUI.
	 */
	size_t size1, size2, size3;
	unsigned char data_bs[size+8], *dst;
	size_t writesize;
	unsigned long writeaddr;
	unsigned char *writesrc;

	if (spaceof(addr, size) == SPACE_CROSSING) {
		fprintf(stderr, "section crossing memory boundary!\n");
		return -1;
	}

	size1 = (addr & 0x3) ? 4 - (addr & 0x3) : 0;	/* top odd bytes */
	size3 = (addr + size) & 0x3;			/* bottom odd bytes */
	size2 = size - size1 - size3;			/* multiple of 4 */
#ifdef DEBUG
	printf("\n");
	printf("addr = %08lx\n", addr);
	printf("size = %08lx, %x:%x:%x\n", size, size1, size2, size3);
#endif

	writesize = 0;

	if (size1) {
#ifdef DEBUG
		printf("top: seek to %08lx\n", addr & ~0x3);
#endif
		lseek(fd, addr & ~0x3, SEEK_SET);
		dst = data_bs;
		if (read(fd, dst, 4) < 4)
			return -1;

#ifdef DEBUG
		printf("%02x %02x %02x %02x -> ",
			dst[0], dst[1], dst[2], dst[3]);
#endif
		if (size1 >= 3)
			dst[0] = *(data++);
		if (size1 >= 2)
			dst[3] = *(data++);
		/* if (size1 >= 1) */
			dst[2] = *(data++);
#ifdef DEBUG
		printf("%02x %02x %02x %02x\n",
			dst[0], dst[1], dst[2], dst[3]);
#endif
	}

	dst = data_bs + 4;
	copy_byteswap(dst, data, size2);
	data      += size2;
	dst       += size2;

	if (size3) {
#ifdef DEBUG
		printf("bottom: seek to %08lx\n", addr+size1+size2);
#endif
		lseek(fd, addr+size1+size2, SEEK_SET);
		if (read(fd, dst, 4) < 4)
			return -1;

#ifdef DEBUG
		printf("%02x %02x %02x %02x ->",
			dst[0], dst[1], dst[2], dst[3]);
#endif
		/* if (size3 >= 1) */
			dst[1] = *(data++);
		if (size3 >= 2)
			dst[0] = *(data++);
		if (size3 >= 3)
			dst[3] = *(data++);
#ifdef DEBUG
		printf("%02x %02x %02x %02x\n",
			dst[0], dst[1], dst[2], dst[3]);
#endif
	}

	writeaddr = size1 ? addr & ~0x3 : addr;
	writesize = (size1 ? 4 : 0) + size2 + (size3 ? 4 : 0);
	writesrc  = size1 ? data_bs : data_bs+4;
#ifdef DEBUG
	printf("writeaddr = %08lx, writesize = %08lx, src offset = %d\n",
		writeaddr, writesize, writesrc-data_bs);
#endif
	lseek(fd, writeaddr, SEEK_SET);
	if (write(fd, writesrc, writesize) != writesize)
		return -1;

	return size;
}

static void copy_byteswap(void *dst, void *src, size_t size)
{
	unsigned long *dst_l = dst;
	unsigned char *src_c = src;
	size_t size_l = size/4;
	int i;

	assert(((unsigned long)dst & 3) == 0);
	assert((size & 3) == 0);
	for (i = 0; i < size_l; i++, src_c+=4, dst_l++) {
		*dst_l = ((unsigned long)src_c[2] << 24) |
			 ((unsigned long)src_c[3] << 16) |
			 ((unsigned long)src_c[0] <<  8) |
			 src_c[1];
	}
}

static int cinit_bss_init(int fd, unsigned char *p, size_t size)
{
	int i = 0;

	while(i < size) {
		COFF_CINIT *cinit = (COFF_CINIT *)&p[i];
		/* size and addr are written in big-endian format */
		unsigned short size = COFF_SHORT_H(cinit->size) * 2; /* size in word */
		unsigned long addr = COFF_24BIT_H(cinit->addr) * 2; /* addr in word */
		unsigned char flags = *cinit->flags;
		int cnt;

#ifdef DEBUG
{
		int j;
		printf("\n  i=%d, size=%04x, addr=%06lx, flags=%02x\n",
			i, size, addr, flags);
		for (j = 0; j < size; j += 2) {
			printf("%02x: %02x %02x\n",
				j, cinit->data[j], cinit->data[j+1]);
		}
}
#endif
		if (size == 0) {
			i+= 2;
			if (i != size) {
				fprintf(stderr,
					"warning: found zero-sized "
					"cinit entry at 0x%x\n", i-2);
			}
			continue;
		}

		if ((flags & 0x01) == COFF_CINIT_FLAG_IO) {
			fprintf(stderr,
				"We can't handle cinit data for I/O space!\n");
			return -1;
		}
		cnt = load_to_dspmem(fd, addr, size, cinit->data);
		if (cnt < size)
			return -1;
		i += COFF_CINITSZ + size;
	}

	if (i > size) {
		fprintf(stderr, "cinit data size is inconsistent.\n");
		return -1;
	}

	return 0;
}
