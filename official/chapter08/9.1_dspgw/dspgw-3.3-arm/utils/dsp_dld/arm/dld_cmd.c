/*
 * dsp_dld/arm/dld_cmd.c
 *
 * DSP Dynamic Loader Daemon: dld_cmd.c
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
 * 2005/07/07:  DSP Gateway version 3.3
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "list.h"
#include "dsp_dld.h"
#include "dld_malloc.h"
#include "dld_daemon.h"
#include "dld_memmgr.h"
#include "dld_cmd.h"

#ifdef STICKY_LIST
static LIST_HEAD(g_dirlist);
static LIST_HEAD(g_exprlist);
static LIST_HEAD(g_loptlist);
#endif

static void dir_entry_print(struct directive *d);

void dir_scn_init(struct dir_scn *dscn);
static void dir_scn_print(struct dir_scn *p);

static void dir_iscn_freelist(struct list_head *list);
static void dir_iscn_print(struct dir_iscn *p);
static inline char *hole_name(enum hole_operator op);

static void expr_tree_print(struct list_head *p);

/**********************************************************************
 * lkcmd
 **********************************************************************/

struct lkcmd *lkcmd_new(char *fn)
{
	struct lkcmd *lkcmd = dld_malloc(sizeof(struct lkcmd), "lkcmd");

	if (lkcmd == NULL)
		return NULL;
	lkcmd->fn = dld_strdup(fn, "lkcmd->fn");
	if (lkcmd->fn == NULL) {
		dld_free(lkcmd, "lkcmd");
		return NULL;
	}
	INIT_LIST_HEAD(&lkcmd->memlist);
	INIT_LIST_HEAD(&lkcmd->dirlist);
	INIT_LIST_HEAD(&lkcmd->exprlist);
	INIT_LIST_HEAD(&lkcmd->loptlist);
	return lkcmd;
}

void lkcmd_free(struct lkcmd *lkcmd)
{
	lkcmd_clear(lkcmd);
	dld_free(lkcmd->fn, "lkcmd->fn");
	dld_free(lkcmd, "lkcmd");
}

void lkcmd_clear(struct lkcmd *lkcmd)
{
#ifdef STICKY_LIST
	if (lkcmd == NULL) {
		/*
		 * handles global lists
		 */
		memmgr_freelist(NULL);
		directive_freelist(NULL);
		globexpr_freelist(NULL);
		lopt_freelist(NULL);
	} else {
#endif
		memmgr_freelist(&lkcmd->memlist);
		directive_freelist(&lkcmd->dirlist);
		globexpr_freelist(&lkcmd->exprlist);
		lopt_freelist(&lkcmd->loptlist);
#ifdef STICKY_LIST
	}
#endif
}

/**********************************************************************
 * directive
 **********************************************************************/

/**
 * directive_new - create an new section entry
 *
 * Create an new section entry.
 * Initialize 3 lists.
 */
struct directive *directive_new(void)
{
	struct directive *dir = dld_malloc(sizeof(struct directive), "directive");

	if (dir == NULL)
		return NULL;
	dir->scnnm = NULL;
	INIT_LIST_HEAD(&dir->iscnlist);
	dir_scn_init(&dir->load);
	dir_scn_init(&dir->run);

	return dir;
}

void directive_free(struct directive *dir)
{
	if (dir->scnnm)
		dld_free(dir->scnnm, "directive->scnnm");

	dir_iscn_freelist(&dir->iscnlist);
	dir_scn_clear(&dir->load);
	dir_scn_clear(&dir->run);
	dld_free(dir, "directive");
}

void directive_freelist(struct list_head *list)
{
	struct directive *dir, *tmp;

#ifdef STICKY_LIST
	if (list == NULL)
		list = &g_dirlist;
#endif
	directive_for_each_safe(dir, tmp, list) {
		list_del(&dir->list_head);
		directive_free(dir);
	}
}

void directive_add(struct list_head *list, struct directive *dir)
{
#ifdef STICKY_LIST
	if (list == NULL)
		list = &g_dirlist;
#endif
	list_add_tail(&dir->list_head, list);
}

#ifdef STICKY_LIST
void directive_register_global(struct list_head *list)
{
	/*
	 * All directives are moved to g_dirlist
	 */
	list_splice(list, &g_dirlist);
	INIT_LIST_HEAD(list);
}
#endif /* STICKY_LIST */

struct directive *directive_find_by_scnnm(struct list_head *list, char *name)
{
	struct directive *dir;

#ifdef STICKY_LIST
	if (list == NULL)
		list = &g_dirlist;
#endif
	directive_for_each(dir, list) {
		if (!strcmp(dir->scnnm, name))
			return dir;
	}

	// not found
	return NULL;
}

/*
 * debug stuff
 */
/**
 * directive_printstat - print SECTIONS directives
 * @list: pointer to link list
 *
 * Print SECTIONS directives in linked list pointed by list
 */
void directive_printstat(struct list_head *list)
{
	struct directive *dir;

#ifdef STICKY_LIST
	if (list == NULL)
		list = &g_dirlist;
#endif
	prmsg("*** struct directive linked list: ***\n");
	directive_for_each(dir, list) {
		dir_entry_print(dir);
	}
	prmsg("\n");
	return;
}

/**
 * dir_entry_print - print section entry
 * @d: pointer to SECTIONS directive
 *
 * Print section entry of SECTIONS directive
 */
static void dir_entry_print(struct directive *d)
{
	struct dir_iscn *iscn;

	prmsg("SECTION NAME: %s\n", d->scnnm);

	prmsg("\tLOAD PROPERTIES:\n");
	dir_scn_print(&d->load);

	prmsg("\tRUN PROPERTIES:\n");
	dir_scn_print(&d->run);

	if (!list_empty(&d->iscnlist)) {
		prmsg("\tINPUT SECTIONS:\n");
		iscn_for_each(iscn, &d->iscnlist) {
			dir_iscn_print(iscn);
		}
	}

	prmsg("\n");
	return;
}

/**********************************************************************
 * globexpr
 **********************************************************************/
struct globexpr *globexpr_new(struct expr_tree *expr)
{
	struct globexpr *ge = dld_malloc(sizeof(struct globexpr), "globexpr");

	if (ge == NULL)
		return NULL;
	INIT_LIST_HEAD(&ge->expr);
	/*
	 * At this moment all elements in expr link are
	 * struct expr_tree, and there is no start point.
	 * So adding the start point (ge->expr) into it with
	 * list_add_tail().
	 */
	list_add_tail(&ge->expr, &expr->list_head);
	return ge;
}

void globexpr_freelist(struct list_head *list)
{
	struct globexpr *exp, *tmp;

#ifdef STICKY_LIST
	if (list == NULL)
		list = &g_exprlist;
#endif
	globexpr_for_each_safe(exp, tmp, list) {
		list_del(&exp->list_head);
		expr_tree_freelist(&exp->expr);
		dld_free(exp, "globexpr");
	}
}

void globexpr_add(struct list_head *list, struct globexpr *expr)
{
#ifdef STICKY_LIST
	if (list == NULL)
		list = &g_exprlist;
#endif
	list_add_tail(&expr->list_head, list);
}

/**
 * expr_printstat - print global assignment statements
 * @list: pointer to link list
 *
 * Print global assignment statements in linked list pointed by list
 */
void globexpr_printstat(struct list_head *list)
{
	struct globexpr *ge;

#ifdef STICKY_LIST
	if (list == NULL)
		list = &g_exprlist;
#endif
	if (list_empty(list)) {
		prmsg("global expr list is empty.\n");
		return;
	}

	prmsg("*** struct global_expression linked list: ***\n");
	globexpr_for_each(ge, list) {
		prmsg("*** expression ***\n");
		expr_tree_print(&ge->expr);
	}
	prmsg("\n");
	return;
}

/**********************************************************************
 * lopt
 **********************************************************************/
struct lopt *lopt_new(size_t sz)
{
	struct lopt *lo = dld_malloc(sizeof(struct lopt), "lopt");

	if (lo == NULL)
		return NULL;
	lo->opt = dld_malloc(sz, "lopt->opt");
	if (lo->opt == NULL) {
		dld_free(lo, "lopt");
		return NULL;
	}

	return lo;
}

void lopt_freelist(struct list_head *list)
{
	struct lopt *lo, *tmp;

#ifdef STICKY_LIST
	if (list == NULL)
		list = &g_dirlist;
#endif
	lopt_for_each_safe(lo, tmp, list) {
		list_del(&lo->list_head);
		if (lo->opt)
			dld_free(lo->opt, "lopt->opt");
		dld_free(lo, "lopt");
	}
}

void lopt_add(struct list_head *list, struct lopt *lo)
{
#ifdef STICKY_LIST
	if (list == NULL)
		list = &g_loptlist;
#endif
	list_add_tail(&lo->list_head, list);
}

/**
 * lopt_printstat - print linkage options
 * @list: pointer to link list
 *
 * Print linkage options in linked list pointed by list
 */
void lopt_printstat(struct list_head *list)
{
	struct lopt *lo;

#ifdef STICKY_LIST
	if (list == NULL)
		list = &g_loptlist;
#endif
	if (list_empty(list)) {
		prmsg("link option list is empty.\n");
		return;
	}

	prmsg("*** struct link_opt_entry linked list: ***\n");
	lopt_for_each(lo, list) {
		prmsg("\t%s\n", lo->opt);
	}
	prmsg("\n");
	return;
}

/**********************************************************************
 * dir_scn
 **********************************************************************/
void dir_scn_init(struct dir_scn *dscn)
{
	dscn->stype = 0;
	dscn->align = 0;
	dscn->block = 0;
	dscn->fill  = 0;
	dscn->mem   = NULL;
	dscn->addr  = 0;
}

void dir_scn_clear(struct dir_scn *dscn)
{
	if (dscn->mem) {
		dld_free(dscn->mem, "dir_scn->mem");
		dscn->mem = NULL;
	}
}

void dir_scn_flush(struct dir_scn *dscn)
{
	dir_scn_clear(dscn);
	dir_scn_init(dscn);
}

void dir_scn_append(struct dir_scn *dst, struct dir_scn *src)
{
	dst->stype |= src->stype;
	if (src->align)
		dst->align = src->align;
	if (src->block)
		dst->block = src->block;
	if (src->fill)
		dst->fill  = src->fill;
	if (src->mem)
		dst->mem   = dld_strdup(src->mem, "dir_scn->mem");
	if (src->addr)
		dst->addr  = src->addr;
}

/**
 * dir_scn_print - print section property
 * @p: pointer to section property
 *
 * Print section property of SECTIONS directive
 */
static void dir_scn_print(struct dir_scn *p)
{
	if (p->stype) {
		u32 stype = p->stype;
		prmsg("\t\tPT_SPECIAL_TYPE: %s%s%s\n",
		      (stype & DIRSCN_STYPE_DSECT)  ? "DSECT "  : "",
		      (stype & DIRSCN_STYPE_COPY)   ? "COPY "   : "",
		      (stype & DIRSCN_STYPE_NOLOAD) ? "NOLOAD " : "");
	}
	if (p->align)
		prmsg("\t\tPT_ALIGN: 0x%lx\n", p->align);
	if (p->block)
		prmsg("\t\tPT_BLOCK: 0x%lx\n", p->block);
	if (p->fill)
		prmsg("\t\tPT_FILL: 0x%lx\n", p->fill);
	if (p->mem)
		prmsg("\t\tPT_MEM: %s\n", p->mem);
	if (p->addr)
		prmsg("\t\tPT_ADDR: 0x%lx\n", p->addr);
	prmsg("\n");
	return;
}

/**********************************************************************
 * dir_iscn
 **********************************************************************/
struct dir_iscn *dir_iscn_new_iscn(char *fn, char *sn)
{
	struct dir_iscn *di = dld_malloc(sizeof(struct dir_iscn), "dir_iscn");

	if (di == NULL)
		return NULL;
	di->type = IT_ISCN;
	di->u.iscn.fn = dld_strdup(fn, "dir_iscn(iscn)->fn");
	if (di->u.iscn.fn == NULL)
		goto fail;
	di->u.iscn.scnnm = dld_strdup(sn, "dir_iscn(iscn)->scnnm");
	if (di->u.iscn.scnnm == NULL)
		goto fail;

	return di;

fail:
	if (di->u.iscn.fn)
		dld_free(di->u.iscn.fn, "dir_iscn(iscn)->fn");
	if (di->u.iscn.scnnm)
		dld_free(di->u.iscn.scnnm, "dir_iscn(iscn)->scnnm");
	if (di)
		dld_free(di, "dir_iscn");
	return NULL;
}

struct dir_iscn *dir_iscn_new_hole(enum hole_operator op, u32 val)
{
	struct dir_iscn *di = dld_malloc(sizeof(struct dir_iscn), "dir_iscn");

	if (di == NULL)
		return NULL;
	di->type = IT_HOLE;
	di->u.hole.op = op;
	di->u.hole.val = val;

	return di;
}

struct dir_iscn *dir_iscn_new_expr(struct expr_tree *expr)
{
	struct dir_iscn *di = dld_malloc(sizeof(struct dir_iscn), "dir_iscn");

	if (di == NULL)
		return NULL;
	di->type = IT_EXPR;
	INIT_LIST_HEAD(&di->u.expr);
	/*
	 * At this moment all elements in expr link are
	 * struct expr_tree, and there is no start point.
	 * So adding the start point (gexpr->expr) into it with
	 * list_add_tail().
	 */
	list_add_tail(&di->u.expr, &expr->list_head);

	return di;
}

static void dir_iscn_freelist(struct list_head *list)
{
	struct dir_iscn *di, *tmp;

	iscn_for_each_safe(di, tmp, list) {
		list_del(&di->list_head);
		switch (di->type) {
			case IT_EXPR:
				expr_tree_freelist(&di->u.expr);
				break;
			case IT_ISCN:
				dld_free(di->u.iscn.fn, "dir_iscn(iscn)->fn");
				dld_free(di->u.iscn.scnnm, "dir_iscn(iscn)->scnnm");
				break;
			case IT_HOLE:
				break;
		}
		dld_free(di, "dir_iscn");
	}
}

/**
 * dir_iscn_print - print input section
 * @p: pointer to linked list
 *
 * Print input section pointed by p
 */
static void dir_iscn_print(struct dir_iscn *p)
{
	switch (p->type) {
		case IT_ISCN:
			prmsg("\t\tIT_ISCN: %s ( %s )\n",
			      p->u.iscn.fn,
			      p->u.iscn.scnnm ?  p->u.iscn.scnnm : "nil");
			break;
		case IT_HOLE:
			prmsg("\t\tIT_HOLE: %s: %lx\n",
			      hole_name(p->u.hole.op), p->u.hole.val);
			break;
		case IT_EXPR:
			prmsg("\n\t\tIT_EXPR:\n");
			expr_tree_print(&p->u.expr);
			prmsg("\n");
			break;
		default:
			prmsg("Invalid inputsection type\n");
			break;
	}

	return;
}

static inline char *hole_name(enum hole_operator op)
{
	return (op == HL_EQ)           ? "HL_EQ" :
	       (op == HL_PLUSEQ)       ? "HL_PLUSEQ" :
	       (op == HL_EQ_ALIGN)     ? "HL_EQ_ALIGN" :
	       (op == HL_EQPLUS_ALIGN) ? "HL_EQPLUS_ALIGN" :
					 "Invalid type";
}


/**********************************************************************
 * expr_tree
 **********************************************************************/

/**
 * expr_tree_new - create a new entry
 * @t: expression type: CONSTANT or VAR
 * @val: valid value if t is CONSTANT
 * @s: point to string as variable name if t is VAR
 *
 * Create a new expr entry.
 */
struct expr_tree *expr_tree_new(enum expr_type t, u32 val, char *s)
{
	struct expr_tree *et = dld_malloc(sizeof(struct expr_tree), "expr_tree");

	if (et == NULL)
		return NULL;
	INIT_LIST_HEAD(&et->list_head);
	et->type = t;

	switch (t) {
		case ET_CONSTANT:
			et->val = val;
			break;
		case ET_VAR:
			et->name = dld_strdup(s, "expr_tree->name");
			if (et->name == NULL) {
				dld_free(et, "expr_tree");
				return NULL;
			}
			break;
		default:
			break;
	}

	return et;
}

/**
 * expr_tree_free - free an entry
 * @et: point to struct expr_tree entry
 *
 * Free an expr entry.
 */
void expr_tree_free(struct expr_tree *et)
{
	if (et->type == ET_VAR)
		dld_free(et->name, "expr_tree->name");
	list_del(&et->list_head);
	dld_free(et, "expr_tree");
	return;
}

void expr_tree_freelist(struct list_head *list)
{
	struct expr_tree *et, *tmp;

	exprtree_for_each_safe(et, tmp, list) {
		expr_tree_free(et);
	}
	return;
}

/**
 * expr_tree_print - print expr tree
 * @p: pointer to expr entry in a tree
 *
 * Print all expr entries in a tree
 */
static void expr_tree_print(struct list_head *list)
{
	struct expr_tree *et;
	enum expr_type type;

	if (list == NULL)
		return;

	exprtree_for_each(et, list) {
		type = et->type;

		if (type == ET_CONSTANT)
			prmsg("\t\t 0x%lx\n", et->val);
		else
			prmsg("\t\t %s\n",
			      (type == ET_VAR)     ? et->name :
			      (type == ET_EQ)      ? "=" :
			      (type == ET_PLUS)    ? "+" :
			      (type == ET_MINUS)   ? "-" :
			      (type == ET_MULT)    ? "*" :
			      (type == ET_DIV)     ? "/" :
			      (type == ET_PLUSEQ)  ? "+=" :
			      (type == ET_MINUSEQ) ? "-=" :
			      (type == ET_MULTEQ)  ? "*=" :
			      (type == ET_DIVEQ)   ? "/=" :
						     "unknown expr_tree type!");
	}
	return;
}
