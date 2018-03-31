/*
 * dsp_dld/arm/dld_section.h
 *
 * DSP Dynamic Loader Daemon: dld_section.h
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
 * 2005/07/11:  DSP Gateway version 3.3
 */

#ifndef __DLD_SECTION_H
#define __DLD_SECTION_H

#include "list.h"
#include "dsp_dld.h"

extern void section_createlist(struct list_head *list, u16 n);
extern void section_freelist(struct list_head *list);
extern void section_filllist(struct coff *coff, u8 *src,
			     struct coffobj *cobj, u16 n, int type);
#ifdef STICKY_LIST
extern void section_register_global(struct list_head *list);
#endif
extern int section_load(struct list_head *list, struct taskent *te);
extern int section_relocate(struct list_head *list);
extern struct section *section_find_by_name(struct list_head *list, char *name);
extern void section_sendstat(struct list_head *list, int fd);

/*
 * debug stuff
 */
extern void section_printstat(struct list_head *list);

#endif /* __DLD_SECTION_H */
