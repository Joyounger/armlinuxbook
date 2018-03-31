/*
 * dsp_dld/arm/dld_symbol.h
 *
 * DSP Dynamic Loader Daemon: dld_symbol.h
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
 * 2005/03/22:  DSP Gateway version 3.3
 */

#ifndef __DLD_SYMBOL_H
#define __DLD_SYMBOL_H

#include "list.h"
#include "dsp_dld.h"

extern void symbol_createlist(struct list_head *list, u16 n);
extern void symbol_freelist(struct list_head *list);
extern void symbol_filllist(struct coff *coff, u8 *src,
			    struct coffobj *cobj, u16 n);
#ifdef STICKY_LIST
extern int symbol_register_global(struct list_head *list);
#endif
extern struct symbol *symbol_find_by_name(struct list_head *list, char *name);
extern struct symbol *symbol_find_scnsym(struct list_head *list,
					 struct section *scn);
extern void symbol_sendstat(struct list_head *list, int fd);
extern int symbol_resolve(struct list_head *list_knl, struct list_head *list);

/*
 * debug stuff
 */
extern void symbol_printstat(struct list_head *list);

#endif /* __DLD_SYMBOL_H */
