/*
 * symbol.h
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
 * 2004/07/22:  DSP Gateway version 3.2
 */

#ifndef __DLD_SYMBOL_H
#define __DLD_SYMBOL_H

#include "list.h"
#include "cofflib.h"

extern void symbol_createlist(struct list_head *list, u16 n);
extern void symbol_free(struct symbol *sym);
extern void symbol_freelist(struct list_head *list);
extern void symbol_filllist(struct coff *coff, u8 *src,
			    struct coffobj *cobj, u16 n);
extern void symbol_putlist(struct coff *coff, struct coffobj *cobj);
extern u32 symbol_countlist(struct list_head *list);
extern u32 symbol_idx(struct list_head *list, struct symbol *ent);
extern size_t symbol_total_strtblsz(struct list_head *list);
extern int symbol_is_absolute(struct symbol *sym);
extern int symbol_is_debug(struct symbol *sym);

#endif /* __DLD_SYMBOL_H */
