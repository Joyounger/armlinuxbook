/*
 * dsp_dld/arm/dld_coff.c
 *
 * DSP Dynamic Loader Daemon: dld_coff.c
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
 * 2005/07/07:  DSP Gateway version 3.3
 */

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <malloc.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <asm/arch/dsp.h>
#include "coff-c55x.h"
#include "dsp_dld.h"
#include "dld_malloc.h"
#include "dld_daemon.h"
#include "dld_section.h"
#include "dld_symbol.h"
#include "dld_cmd.h"

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

struct coffobj *coff_new(char *fn)
{
	struct coffobj *cobj = dld_malloc(sizeof(struct coffobj), "coffobj");

	if (cobj == NULL)
		return NULL;
	cobj->fn = dld_strdup(fn, "coffobj->fn");
	if (cobj->fn == NULL) {
		dld_free(cobj, "coffobj");
		return NULL;
	}
	cobj->taskcount = 0;
	cobj->usecount = 0;
#ifdef USE_FORK
	cobj->request_lock = 0;
#endif
	coff_init(cobj);
	return cobj;
}

void coff_free(struct coffobj *cobj)
{
	coff_clear(cobj);
	if (cobj->fn)
		dld_free(cobj->fn, "coffobj->fn");
	dld_free(cobj, "coffobj");
}

void coff_init(struct coffobj *cobj)
{
	INIT_LIST_HEAD(&cobj->scnlist);
	INIT_LIST_HEAD(&cobj->symlist);
}

void coff_clear(struct coffobj *cobj)
{
	section_freelist(&cobj->scnlist);
	symbol_freelist(&cobj->symlist);
}

int coff_read(struct coffobj *cobj, int type)
{
	struct coff coff;
	COFF_AOUTHDR *aouthdr;
	int fd;
	struct stat stat;
	size_t size;
	u8 *src;
	u16 nscns;
	u32 symptr;
	u32 nsyms;
	u16 opthdr;
	u16 flags;
	u16 tgtid;
	u32 strtblsz;

	if ((fd = open(cobj->fn, O_RDONLY)) < 0) {
		prmsg("Can't open %s\n", cobj->fn);
		return -1;
	}

	/* acquiring file size */
	if (fstat(fd, &stat) < 0) {
		prmsg("%s: fstat() faild\n", cobj->fn);
		return -1;
	}
	size = stat.st_size;

	/* mmap */
	if ((src = mmap(0, size, PROT_READ, MAP_SHARED, fd, 0)) < 0) {
		prmsg("%s: mmap() failed\n", cobj->fn);
		return -1;
	}

	/* COFF file header */
	coff.coffhdr = (COFF_FILHDR*)&src[0];
	nscns  = COFF_SHORT(coff.coffhdr->f_nscns);
	symptr = COFF_LONG(coff.coffhdr->f_symptr);
	nsyms  = COFF_LONG(coff.coffhdr->f_nsyms);
	opthdr = COFF_SHORT(coff.coffhdr->f_opthdr);
	flags  = COFF_SHORT(coff.coffhdr->f_flags);
	tgtid  = COFF_SHORT(coff.coffhdr->f_tgtid);

	/*
	 * some checks are done here.
	 */
	if (tgtid != COFF_TGTID_C55X) {
		prmsg("%s: Unknown COFF target ID [0x%04x]!\n"
		      "We are not sure if this file can be executed "
		      "in this system.\n", cobj->fn, tgtid);
	}
	if (type == COFFTYP_KERNEL) {
		if ((!(flags & COFF_F_EXEC)) || (!opthdr)) {
			prmsg("%s: Error! not executable.\n", cobj->fn);
			return -1;
		}
	}
	if (type == COFFTYP_TASK) {
		if (flags & COFF_F_RELFLG) {
			prmsg("%s: Error! not relocatable.\n", cobj->fn);
			return -1;
		}
	}

	/* a.out header */
	aouthdr = opthdr ? (COFF_AOUTHDR*)&src[COFF_FILHSZ] : NULL;

	/* section header */
	coff.scnhdr = (COFF_SCNHDR*)&src[COFF_FILHSZ + opthdr];

	/* symbol table */
	coff.symtbl = (COFF_SYMENT*)&src[symptr];

	/* string table */
	coff.strtbl = &src[symptr + COFF_SYMESZ * nsyms];
	strtblsz = COFF_LONG(coff.strtbl);

	cobj->entry = aouthdr ? COFF_LONG(aouthdr->entry) : 0;
	section_createlist(&cobj->scnlist, nscns);
	symbol_createlist(&cobj->symlist, nsyms);
	section_filllist(&coff, src, cobj, nscns, type);
	symbol_filllist(&coff, src, cobj, nsyms);
	munmap(src, size);
	close(fd);

	return 0;
}
