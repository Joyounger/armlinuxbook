/*
 * cofflib.h
 *
 * Copyright (C) 2003-2005 Nokia Corporation
 *
 * Written by Toshihiro Kobayashi <toshihiro.kobayashi@nokia.com>
 * Written by Kiyotaka Takahashi <kiyotaka.takahashi@nokia.com>
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
 * 2004/12/09:  DSP Gateway version 3.2
 */

#ifndef __COFFLIB_H
#define __COFFLIB_H

#include "coff-c55x.h"
#include "list.h"

typedef unsigned long u32;
typedef unsigned short u16;
typedef unsigned char u8;

/*
 *
 */
struct coff {
	COFF_FILHDR *coffhdr;
	COFF_AOUTHDR *aouthdr;
	COFF_SCNHDR *scnhdr;
	COFF_SYMENT *symtbl;
	char *strtbl;
};

struct coffobj {
//	char *fn;
	u16 verid;
	u16 flags;
	u16 tgtid;
	struct aouthdr *aouthdr;
	struct list_head scnlist;
	struct list_head symlist;
};

struct aouthdr {
	u16 magic;
	u16 vstamp;
	u32 entry;
	u32 text_start;
	u32 data_start;
};

/*
 *
 */
struct symbol {
	struct list_head list_head;
	char *name;
	u32 value;
	struct section *scn;
	u16 type;
	u8 sclass;
	u8 *aux;
	u32 refcnt;
};

#define symbol_for_each(pos, head) \
	list_for_each_entry(pos, head, list_head)

struct reloc {
	u32 vaddr;
	union {
		struct symbol *sym;
		u32 val;
	} sym;
	u16 exa;
	u16 type;
};

struct section {
	struct list_head list_head;
	char *name;
	u32 paddr;
	u32 vaddr;
	u32 size;
	u8 *data;
	u32 nreloc;
	struct reloc *reloc;
	u32 flags;
	struct coffobj *cobj;
};

#define section_for_each(pos, head) \
	list_for_each_entry(pos, head, list_head)

#endif /* __COFFLIB_H */
