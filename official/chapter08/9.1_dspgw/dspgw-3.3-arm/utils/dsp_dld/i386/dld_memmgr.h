/*
 * dsp_dld/arm/dld_memmgr.h
 *
 * DSP Dynamic Loader Daemon: dld_memmgr.h
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
 * 2005/03/23:  DSP Gateway version 3.3
 */

#ifndef __DLD_MEMMGR_H
#define __DLD_MEMMGR_H

#include "list.h"
#include "dsp_dld.h"

extern struct memmgr *memmgr_new(char *name, u32 base, u32 size);
extern int memmgr_validate(struct memmgr *mem);
extern void memmgr_freelist(struct list_head *list);
extern void memmgr_add(struct list_head *list, struct memmgr *mem);
extern int memmgr_register_global(struct list_head *list);
#ifndef DSP_EMULATION
extern int memmgr_exmap(struct list_head *list);
#endif
extern struct memmgr *memmgr_range_user(struct list_head *list,
					u32 base, u32 size);
extern struct memmgr *memmgr_find_by_addr(struct list_head *list,
					  u32 addr, u32 size);
extern struct memmgr *memmgr_find_by_name(struct list_head *list, char *name);
extern int space_find_by_addr(u32 addr, u32 size);
extern void memmgr_occupy_kernel(struct list_head *scnlist);
extern int memmgr_placetask(struct taskent *te, struct coffobj *cobj,
			    struct lkcmd *gbl_lkcmd);
extern void memmgr_cleartask(struct taskent *te);
extern void memmgr_sendstat(struct list_head *list, int fd);

/*
 * debug stuff
 */
extern void memmgr_printstat(struct list_head *list);

#endif /* __DLD_MEMMGR_H */
