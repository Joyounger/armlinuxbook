/*
 * dsp_dld/arm/dld_taskent.h
 *
 * DSP Dynamic Loader Daemon: dld_taskent.h
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
 * 2005/03/18:  DSP Gateway version 3.3
 */

#ifndef __DLD_TASKENT_H
#define __DLD_TASKENT_H

#include "list.h"
#include "dsp_dld.h"

extern struct taskent *taskent_next(struct taskent *te);
extern void taskent_freelist(void);
extern int taskent_readconfig(struct dld_conf *conf);
extern int taskent_mkdev(int fd);
extern int taskent_rmdev(int fd);
#ifndef DSP_EMULATION
extern int taskent_process_request_all(void);
#endif
extern struct taskent *taskent_find_by_minor(u8 minor);
extern int taskent_register_lkcmd(struct taskent *te, struct lkcmd *lkcmd);
extern struct memmgr *taskent_mem_range_user(u32 base, u32 size);
extern void taskent_mem_sendstat(int fd);
extern void taskent_sym_sendstat(int fd);
extern void taskent_scn_sendstat(int fd);
extern void taskent_sendstat(int fd);

/*
 * debug stuff
 */
extern void taskent_printstat(void);

#endif /* __DLD_TASKENT_H */
