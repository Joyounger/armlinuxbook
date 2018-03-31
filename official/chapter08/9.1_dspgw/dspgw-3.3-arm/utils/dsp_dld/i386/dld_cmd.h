/*
 * dsp_dld/arm/dld_cmd.h
 *
 * DSP Dynamic Loader Daemon: dld_cmd.h
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
 * 2005/03/23:  DSP Gateway version 3.3
 */

#ifndef __DLD_DIRECTIVE_H
#define __DLD_DIRECTIVE_H

#include "list.h"
#include "dsp_dld.h"

extern int lkcmd_read(struct lkcmd *lkcmd);
extern struct lkcmd *lkcmd_new(char *fn);
extern void lkcmd_free(struct lkcmd *lkcmd);
extern void lkcmd_clear(struct lkcmd *lkcmd);

extern struct directive *directive_new(void);
extern void directive_free(struct directive *dir);
extern void directive_freelist(struct list_head *list);
extern void directive_add(struct list_head *list, struct directive *dir);
extern void directive_register_global(struct list_head *list);
extern struct directive *directive_find_by_scnnm(struct list_head *list,
						 char *name);

/*
 * debug stuff
 */
extern void directive_printstat(struct list_head *list);

/**********************************************************************
 * globexpr
 **********************************************************************/
extern struct globexpr *globexpr_new(struct expr_tree *expr);
extern void globexpr_freelist(struct list_head *list);
extern void globexpr_add(struct list_head *list, struct globexpr *expr);

/*
 * debug stuff
 */
extern void globexpr_printstat(struct list_head *list);

/**********************************************************************
 * lopt
 **********************************************************************/
extern struct lopt *lopt_new(size_t size);
extern void lopt_freelist(struct list_head *list);
extern void lopt_add(struct list_head *list, struct lopt *opt);

/*
 * debug stuff
 */
extern void lopt_printstat(struct list_head *list);

/**********************************************************************
 * dir_scn
 **********************************************************************/
extern void dir_scn_init(struct dir_scn *dscn);
extern void dir_scn_clear(struct dir_scn *dscn);
extern void dir_scn_flush(struct dir_scn *dscn);
extern void dir_scn_append(struct dir_scn *dst, struct dir_scn *src);

/**********************************************************************
 * dir_iscn
 **********************************************************************/
extern struct dir_iscn *dir_iscn_new_iscn(char *fn, char *sn);
extern struct dir_iscn *dir_iscn_new_hole(enum hole_operator op, u32 val);
extern struct dir_iscn *dir_iscn_new_expr(struct expr_tree *expr);

/**********************************************************************
 * expr_tree
 **********************************************************************/
extern struct expr_tree *expr_tree_new(enum expr_type t, u32 val, char *s);
extern void expr_tree_free(struct expr_tree *expr);
extern void expr_tree_freelist(struct list_head *list);

#endif /* __DLD_DIRECTIVE_H */
