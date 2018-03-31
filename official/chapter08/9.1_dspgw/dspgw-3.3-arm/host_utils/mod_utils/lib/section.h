/*
 * section.h
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
 * 2004/07/23:  DSP Gateway version 3.2
 */

#ifndef __SECTION_H
#define __SECTION_H

#include "list.h"
#include "cofflib.h"

extern void section_set_out_data_align(int align);
extern struct section *section_new(void);
extern void section_createlist(struct list_head *list, u16 n);
extern void section_free(struct section *scn);
extern void section_freelist(struct list_head *list);
extern void section_filllist(struct coff *coff, u8 *src,
			     struct coffobj *cobj, u16 n);
extern void section_putlist(struct coff *coff, u8 *dst,
			    struct coffobj *cobj, u32 n);
extern struct section *section_find_by_name(struct list_head *list, char *name);
extern size_t section_total_datasz(struct list_head *list);
extern size_t section_total_strtblsz(struct list_head *list);

#define RELOC_SYMBOL_FIELD_TYPE_INVALID	0
#define RELOC_SYMBOL_FIELD_TYPE_SYMBOL	1
#define RELOC_SYMBOL_FIELD_TYPE_VALUE	2

int rel_sym_field_type(u16 type);

#endif /* __SECTION_H */
