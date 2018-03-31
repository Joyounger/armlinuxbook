/*
 * coffobj.c
 *
 * Copyright (C) 2003-2005 Nokia Corporation
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
 * 2004/07/22:  DSP Gateway version 3.2
 */

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <malloc.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/mman.h>
#include "coff-c55x.h"
#include "cofflib.h"
#include "section.h"
#include "symbol.h"

void coff_init(struct coffobj *cobj);
void coff_clear(struct coffobj *cobj);

char *scnname(struct coff *coff, char *name)
{
	static char nm[9];

	if (name[0] == 0) {
		u32 idx = COFF_LONG((&name[4]));
		return &coff->strtbl[idx];
	} else {
		strncpy(nm, name, 8);
		nm[8] = '\0';
		return nm;
	}
}

void scnname_put(struct coff *coff, char *name, char *dst)
{
	u32 len = strlen(name);

	memset(dst, 0, 8);

	if (len <= 8)
		strncpy(dst, name, 8);
	else {
		u32 strtblsz = COFF_LONG(coff->strtbl);

		strcpy(coff->strtbl + strtblsz, name);
		COFF_PUT_LONG(strtblsz, (&dst[4]));
		strtblsz += len + 1;
		COFF_PUT_LONG(strtblsz, coff->strtbl);
	}
}

char *symname(struct coff *coff, COFF_SYMENT *sym)
{
	static char nm[E_SYMNMLEN+1];

	if (COFF_LONG(sym->e.e.e_zeroes) == 0) {
		u32 idx = COFF_LONG(sym->e.e.e_offset);
		return &coff->strtbl[idx];
	} else {
		strncpy(nm, sym->e.e_name, E_SYMNMLEN);
		nm[E_SYMNMLEN] = '\0';
		return nm;
	}
}

void symname_put(struct coff *coff, char *name, COFF_SYMENT *dst)
{
	u32 len = strlen(name);

	memset(dst->e.e_name, 0, E_SYMNMLEN);

	if (len <= E_SYMNMLEN)
		strncpy(dst->e.e_name, name, E_SYMNMLEN);
	else {
		u32 strtblsz = COFF_LONG(coff->strtbl);

		strcpy(coff->strtbl + strtblsz, name);
		COFF_PUT_LONG(strtblsz, dst->e.e.e_offset);
		strtblsz += len + 1;
		COFF_PUT_LONG(strtblsz, coff->strtbl);
	}
}

struct coffobj *coff_new(void)
{
	struct coffobj *cobj = malloc(sizeof(struct coffobj));

	coff_init(cobj);
	return cobj;
}

void coff_free(struct coffobj *cobj)
{
	coff_clear(cobj);
	if (cobj->aouthdr)
		free(cobj->aouthdr);
	free(cobj);
}

void coff_init(struct coffobj *cobj)
{
	cobj->aouthdr = NULL;
	INIT_LIST_HEAD(&cobj->scnlist);
	INIT_LIST_HEAD(&cobj->symlist);
}

void coff_clear(struct coffobj *cobj)
{
	section_freelist(&cobj->scnlist);
	symbol_freelist(&cobj->symlist);
}

int coff_read(struct coffobj *cobj, char *fn)
{
	struct coff coff;
	int fd;
	struct stat stat;
	size_t size;
	u8 *src;
	u16 nscns;
	u32 symptr;
	u32 nsyms;
	u16 opthdr;
	u32 strtblsz;

	if ((fd = open(fn, O_RDONLY)) < 0) {
		fprintf(stderr, "Can't open %s\n", fn);
		return -1;
	}

	/* acquiring file size */
	if (fstat(fd, &stat) < 0) {
		fprintf(stderr, "%s: fstat() faild\n", fn);
		return -1;
	}
	size = stat.st_size;

	/* mmap */
	if ((src = mmap(0, size, PROT_READ, MAP_SHARED, fd, 0)) < 0) {
		fprintf(stderr, "%s: mmap() failed\n", fn);
		return -1;
	}

	/* COFF file header */
	coff.coffhdr = (COFF_FILHDR*)&src[0];
	cobj->verid = COFF_SHORT(coff.coffhdr->f_verid);
	nscns  = COFF_SHORT(coff.coffhdr->f_nscns);
	symptr = COFF_LONG(coff.coffhdr->f_symptr);
	nsyms  = COFF_LONG(coff.coffhdr->f_nsyms);
	opthdr = COFF_SHORT(coff.coffhdr->f_opthdr);
	cobj->flags = COFF_SHORT(coff.coffhdr->f_flags);
	cobj->tgtid = COFF_SHORT(coff.coffhdr->f_tgtid);

	/*
	 * some checks are done here.
	 */
	if (cobj->tgtid != COFF_TGTID_C55X) {
		fprintf(stderr,
			"%s: Unknown COFF target ID [0x%04x]!\n"
			"We are not sure if this file can be executed "
			"in this system.\n", fn, cobj->tgtid);
	}

	/* a.out header */
	if (opthdr) {
		coff.aouthdr = (COFF_AOUTHDR *)&src[COFF_FILHSZ];
		cobj->aouthdr = malloc(COFF_AOUTSZ);
		cobj->aouthdr->magic = COFF_SHORT(coff.aouthdr->magic);
		cobj->aouthdr->vstamp = COFF_SHORT(coff.aouthdr->vstamp);
		cobj->aouthdr->entry = COFF_LONG(coff.aouthdr->entry);
		cobj->aouthdr->text_start = COFF_LONG(coff.aouthdr->text_start);
		cobj->aouthdr->data_start = COFF_LONG(coff.aouthdr->data_start);
	}

	/* section header */
	coff.scnhdr = (COFF_SCNHDR*)&src[COFF_FILHSZ + opthdr];

	/* symbol table */
	coff.symtbl = (COFF_SYMENT*)&src[symptr];

	/* string table */
	coff.strtbl = &src[symptr + COFF_SYMESZ * nsyms];
	strtblsz = COFF_LONG(coff.strtbl);

	section_createlist(&cobj->scnlist, nscns);
	symbol_createlist(&cobj->symlist, nsyms);
	section_filllist(&coff, src, cobj, nscns);
	symbol_filllist(&coff, src, cobj, nsyms);
	munmap(src, size);
	close(fd);

	return 0;
}

int coff_write(struct coffobj *cobj, char *fn)
{
	struct coff coff;
	int fd;
	const char dummy_c = 0;
	struct timeval tm;
	size_t size;
	u8 *dst;
	u16 nscns;
	u32 scnptr, symptr, strptr;
	u32 nsyms;
	u16 opthdr;
	u32 strtblsz;

	unlink(fn);

	if ((fd = open(fn, O_RDWR | O_CREAT, 0644)) < 0) {
		fprintf(stderr, "Can't open %s for write.\n", fn);
		return -1;
	}

	/* size info */
	opthdr = cobj->aouthdr ? COFF_AOUTSZ : 0;
	nscns = list_count(&cobj->scnlist);
	nsyms = symbol_countlist(&cobj->symlist);

	scnptr = COFF_FILHSZ + opthdr;
	symptr = scnptr + nscns * COFF_SCNHSZ +
		 section_total_datasz(&cobj->scnlist);
	strptr = symptr + COFF_SYMESZ * nsyms;
	size = strptr + 4 + section_total_strtblsz(&cobj->scnlist) +
	       symbol_total_strtblsz(&cobj->symlist);

	lseek(fd, size-1, SEEK_SET);
	write(fd, &dummy_c, 1);

	/* mmap */
	if ((dst = mmap(0, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0)) < 0) {
		fprintf(stderr, "%s: mmap() failed\n", fn);
		return -1;
	}

	coff.coffhdr = (COFF_FILHDR*)&dst[0];
	if (opthdr)
		coff.aouthdr = (COFF_AOUTHDR *)&dst[COFF_FILHSZ];
	coff.scnhdr  = (COFF_SCNHDR*)&dst[scnptr];
	coff.symtbl  = (COFF_SYMENT*)&dst[symptr];
	coff.strtbl  = &dst[strptr];

	/* COFF file header */
	gettimeofday(&tm, NULL);
	COFF_PUT_SHORT(cobj->verid, coff.coffhdr->f_verid);
	COFF_PUT_SHORT(nscns,       coff.coffhdr->f_nscns);
	COFF_PUT_LONG(tm.tv_sec,   coff.coffhdr->f_timdat);
	COFF_PUT_LONG(symptr,       coff.coffhdr->f_symptr);
	COFF_PUT_LONG(nsyms,        coff.coffhdr->f_nsyms);
	COFF_PUT_SHORT(opthdr,      coff.coffhdr->f_opthdr);
	COFF_PUT_SHORT(cobj->flags, coff.coffhdr->f_flags);
	COFF_PUT_SHORT(cobj->tgtid, coff.coffhdr->f_tgtid);

	/* a.out header */
	if (cobj->aouthdr) {
		u32 tsize;
		u32 dsize;
		u32 bsize;
		struct section *scn;
		
		scn = section_find_by_name(&cobj->scnlist, ".text");
		tsize = scn ? scn->size : 0;
		scn = section_find_by_name(&cobj->scnlist, ".data");
		dsize = scn ? scn->size : 0;
		scn = section_find_by_name(&cobj->scnlist, ".bss");
		bsize = scn ? scn->size : 0;
		COFF_PUT_SHORT(cobj->aouthdr->magic,     coff.aouthdr->magic);
		COFF_PUT_SHORT(cobj->aouthdr->vstamp,    coff.aouthdr->vstamp);
		COFF_PUT_LONG(tsize,                     coff.aouthdr->tsize);
		COFF_PUT_LONG(dsize,                     coff.aouthdr->dsize);
		COFF_PUT_LONG(bsize,                     coff.aouthdr->bsize);
		COFF_PUT_LONG(cobj->aouthdr->entry,      coff.aouthdr->entry);
		COFF_PUT_LONG(cobj->aouthdr->text_start, coff.aouthdr->text_start);
		COFF_PUT_LONG(cobj->aouthdr->data_start, coff.aouthdr->data_start);
	}

	/* string table init */
	strtblsz = 4;
	COFF_PUT_LONG(strtblsz, coff.strtbl);

	section_putlist(&coff, dst, cobj, nscns);
	symbol_putlist(&coff, cobj);
	munmap(dst, size);
	close(fd);

	return 0;
}
