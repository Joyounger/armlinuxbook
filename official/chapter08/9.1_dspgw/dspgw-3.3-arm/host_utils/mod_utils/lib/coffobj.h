/*
 * coffobj.h
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

#ifndef __COFFOBJ_H
#define __COFFOBJ_H

#include "coff-c55x.h"
#include "cofflib.h"

extern char *scnname(struct coff *coff, char *name);
extern void scnname_put(struct coff *coff, char *name, char *dst);
extern char *symname(struct coff *coff, COFF_SYMENT *sym);
extern void symname_put(struct coff *coff, char *name, COFF_SYMENT *dst);
extern struct coffobj *coff_new(void);
extern void coff_free(struct coffobj *cobj);
extern void coff_init(struct coffobj *cobj);
extern void coff_clear(struct coffobj *cobj);
extern int coff_read(struct coffobj *cobj, char *fn);
extern int coff_write(struct coffobj *cobj, char *fn);

#endif /* __COFFOBJ_H */
