/*
 * dsp_dld/arm/dld_coff.h
 *
 * DSP Dynamic Loader Daemon: dld_coff.h
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
 * 2004/09/30:  DSP Gateway version 3.3
 */

#ifndef __DLD_COFF_H
#define __DLD_COFF_H

#include "coff-c55x.h"
#include "dsp_dld.h"

extern char *scnname(struct coff *coff, char *name);
extern char *symname(struct coff *coff, COFF_SYMENT *sym);
extern struct coffobj *coff_new(char *fn);
extern void coff_free(struct coffobj *cobj);
extern void coff_init(struct coffobj *cobj);
extern void coff_clear(struct coffobj *cobj);
#define coff_read_kernel(cobj)	coff_read(cobj,COFFTYP_KERNEL)
#define coff_read_task(cobj)	coff_read(cobj,COFFTYP_TASK)
extern int coff_read(struct coffobj *cobj, int type);

#endif /* __DLD_COFF_H */
