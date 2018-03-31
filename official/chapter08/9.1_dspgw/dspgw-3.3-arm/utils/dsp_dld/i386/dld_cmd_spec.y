/*
 * dsp_dld/arm/dld_cmd_spec.y
 *
 * DSP Dynamic Loader Daemon: dld_cmd_spec.y
 *
 * Copyright (C) 2003-2005 Nokia Corporation
 *
 * Written by Kiyotaka Takahashi <kiyotaka.takahashi@nokia.com>
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
 * 2005/07/07:  DSP Gateway version 3.3
 */

%{

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "dsp_dld.h"
#include "dld_malloc.h"
#include "dld_daemon.h"
#include "dld_cmd.h"
#include "dld_memmgr.h"

static inline void list_splice_tail_nostub(struct list_head *list,
					   struct list_head *head)
{
	struct list_head *oldlast = head->prev;
	struct list_head *last = list->prev;

	list->prev = oldlast;
	oldlast->next = list;

	last->next = head;
	head->prev = last;
}

#define DIRECTIVE_NEW(dir) \
do { \
	if (((dir) = directive_new()) == NULL) { \
		prmsg("memory allocation failed at %s line %d\n", \
		      __FILE__, __LINE__); \
		y_err = -1; \
		YYABORT; \
	} \
} while(0)

#define MEMMGR_NEW(mem,name,base,size) \
do { \
	if (((mem) = memmgr_new(name, base, size)) == NULL) { \
		prmsg("memory allocation failed at %s line %d\n", \
		       __FILE__, __LINE__); \
		y_err = -1; \
		YYABORT; \
	} \
} while(0)

#define LOPT_NEW(lo,sz) \
do { \
	if (((lo) = lopt_new(sz)) == NULL) { \
		prmsg("memory allocation failed at %s line %d\n", \
		      __FILE__, __LINE__); \
		y_err = -1; \
		YYABORT; \
	} \
} while(0)

typedef struct Codeval {
int val;
char* name;
} codeval;

struct dirgrp {
	struct list_head list_head;
	struct directive *dir;
};

#define dirgrp_for_each(pos, head) \
	list_for_each_entry(pos, head, list_head)
#define dirgrp_for_each_safe(pos, n, head) \
	list_for_each_entry_safe(pos, n, head, list_head)


static LIST_HEAD(g_dirgrp);

static struct dirgrp *dirgrp_new(struct directive *dir)
{
	struct dirgrp *dg = dld_malloc(sizeof(struct dirgrp), "dirgrp");

	if (dg == NULL)
		return NULL;
	dg->dir = dir;
	INIT_LIST_HEAD(&dg->list_head);

	return dg;
}

#define DIRGRP_NEW(dg,dir) \
do { \
	if (((dg) = dirgrp_new(dir)) == NULL) { \
		prmsg("memory allocation failed at %s line %d\n", \
		      __FILE__, __LINE__); \
		y_err = -1; \
		YYABORT; \
	} \
} while(0)

static void dirgrp_add(struct dirgrp *dg)
{
	list_add_tail(&dg->list_head, &g_dirgrp);
}

static void dirgrp_freelist(void)
{
	struct dirgrp *dg, *tmp;

	dirgrp_for_each_safe(dg, tmp, &g_dirgrp) {
		list_del(&dg->list_head);
		/* do not free dir */
		dld_free(dg, "dirgrp");
	}
}

static void dirgrp_setinfo(struct directive *src)
{
	struct dirgrp *dg;

	/*
	 * mem is MEMORY name string at this moment
	 */
	list_for_each_entry(dg, &g_dirgrp, list_head) {
		dir_scn_append(&dg->dir->load, &src->load);
		dir_scn_append(&dg->dir->run, &src->run);
	}
	dir_scn_clear(&src->load);
	dir_scn_clear(&src->run);
}

static struct dir_scn tmp_dir_scn;
static struct directive *cur_directive = NULL;
static struct lkcmd *user_lkcmd;
static int y_err;

int yylex(void);
int yyerror(char *);
void dir_convert_memptr(struct list_head *dirlist);

%}

%union {
	struct expr_tree *expr;
}

%token MEMORY
%token ORIGIN
%token <expr> ID
%token LENGTH
%token COMMA

%token IDENTIFIER
%token <expr> CONSTANT
%token RIGHT_OP
%token SECTIONS
%token ALIGN
%token ATTR
%token BLOCK
%token COPY
%token DSECT
%token FILL
%token GROUP
%token LOAD
%token NOLOAD
%token PAGE
%token RANGE
%token RUN
%token SPARE
%token TYPE
%token UNION

%token '.'
%token ':'
%token '['
%token ']'
%token '>'
%token '|'

%token '('
%token ')'
%token ','
%token '{'
%token '}'
%token ';'
%token '='
%left '+' '-'
%left '*' '/'

%token ADD_ASSIGN
%token SUB_ASSIGN
%token MUL_ASSIGN
%token DIV_ASSIGN
%token MOD_ASSIGN
%token AND_ASSIGN
%token XOR_ASSIGN
%token OR_ASSIGN
%token LEFT_OP
%token LE_OP
%token GE_OP
%token EQ_OP
%token NE_OP

%type <expr> assignment
%type <expr> E
%type <expr> T
%type <expr> F
%%

commands	: directives
{
}
		;

directives	: directives directive
{
}
		|
{
}
		;
directive 	: MEMORY '{' mementries '}'
{
/* MEMORY directive */
}
		| SECTIONS
{
/* SECTIONS directive */
	DIRECTIVE_NEW(cur_directive);
	dir_scn_init(&tmp_dir_scn);
}
'{' sctentries '}'
{
	directive_free(cur_directive);
}
		| assignment
{
/* Global assignment */
	struct globexpr *e;
	e = globexpr_new($1);
	globexpr_add(&user_lkcmd->exprlist, e);
}
		| '-' ID
{
/* Linkage option */
	struct lopt *o;
	LOPT_NEW(o, 1 + strlen($2->name) + 1);
	sprintf(o->opt, "-%s", $2->name);
	lopt_add(&user_lkcmd->loptlist, o);
	expr_tree_free($2);
}
		| '-' ID ID
{
/* Linkage option */
	struct lopt *o;
	LOPT_NEW(o, 1 + strlen($2->name) + 1 + strlen($3->name) + 1);
	sprintf(o->opt, "-%s %s", $2->name, $3->name);
	lopt_add(&user_lkcmd->loptlist, o);
	expr_tree_free($2);
	expr_tree_free($3);
}
		| '-' ID CONSTANT
{
/* Linkage option */
	struct lopt *o;
	LOPT_NEW(o, 1 + strlen($2->name) + 1 + 2 + 8 + 1);
	sprintf(o->opt, "-%s 0x%08lx", $2->name, $3->val);
	lopt_add(&user_lkcmd->loptlist, o);
	expr_tree_free($2);
	expr_tree_free($3);
}
		;

mementries	: mementries pageopt mementry
{
}
		|
{
}
		;

sctentries	: sctentries sctentry
{
}
		|
{
}
		;

pageopt		: PAGE CONSTANT ':'
{
	if ($2->val != 0) {
		prmsg("PAGE %d\n", $2->val);
		prmsg("MEMORY directive: PAGE option other than 0 is not supported.\n");
		y_err = -1;
		YYABORT;
	}
	expr_tree_free($2);
}
		|
{
}
		;

mementry 	: ID ':' ORIGIN '=' CONSTANT ',' LENGTH '=' CONSTANT
{
	struct memmgr	*memmgr;
	MEMMGR_NEW(memmgr, $1->name, $5->val, $9->val);
	expr_tree_free($1);
	expr_tree_free($5);
	expr_tree_free($9);
	memmgr->lkcmd = user_lkcmd;
	memmgr_add(&user_lkcmd->memlist, memmgr);
}
		| ID '(' ID ')' ':' ORIGIN '=' CONSTANT ',' LENGTH '=' CONSTANT
{
	struct memmgr	*memmgr;
	MEMMGR_NEW(memmgr, $1->name, $8->val, $12->val);
	expr_tree_free($1);
	expr_tree_free($3);
	expr_tree_free($8);
	expr_tree_free($12);
	memmgr->lkcmd = user_lkcmd;
	memmgr_add(&user_lkcmd->memlist, memmgr);
}
		|
{
}
		;

sctentry	: secname secinfo
{
	dir_scn_append(&cur_directive->load, &tmp_dir_scn);
	dir_scn_flush(&tmp_dir_scn);
	directive_add(&user_lkcmd->dirlist, cur_directive);
	DIRECTIVE_NEW(cur_directive);
}
		| secname secoptions '{' inputsections '}' secinfo
{
	dir_scn_append(&cur_directive->load, &tmp_dir_scn);
	dir_scn_flush(&tmp_dir_scn);
	directive_add(&user_lkcmd->dirlist, cur_directive);
	DIRECTIVE_NEW(cur_directive);
}
		| secname secinfo '{' inputsections '}' secinfo
{
	dir_scn_append(&cur_directive->load, &tmp_dir_scn);
	dir_scn_flush(&tmp_dir_scn);
	directive_add(&user_lkcmd->dirlist, cur_directive);
	DIRECTIVE_NEW(cur_directive);
}
		| GROUP
{
	prmsg("Warning: GROUP is not supported.\n");
}
'{' groupentries '}' secinfo
{
	/*
	 * cur_directive has only secinfo.
	 * copy this to all directives in dirgrp, then free.
	 */
	dirgrp_setinfo(cur_directive);
	directive_free(cur_directive);
	DIRECTIVE_NEW(cur_directive);
	dirgrp_freelist();
}
		;

groupentries	: groupentries groupentry
{
}
		|
{
}
		;

groupentry	: secname
{
	struct dirgrp *dg;
	DIRGRP_NEW(dg, cur_directive);
	dirgrp_add(dg);
	directive_add(&user_lkcmd->dirlist, cur_directive);
	DIRECTIVE_NEW(cur_directive);
}
		| secname secoptions '{' inputsections '}'
{
	struct dirgrp *dg;
	DIRGRP_NEW(dg, cur_directive);
	dirgrp_add(dg);
	directive_add(&user_lkcmd->dirlist, cur_directive);
	DIRECTIVE_NEW(cur_directive);
}
		| secname '{' inputsections '}'
{
	struct dirgrp *dg;
	DIRGRP_NEW(dg, cur_directive);
	dirgrp_add(dg);
	directive_add(&user_lkcmd->dirlist, cur_directive);
	DIRECTIVE_NEW(cur_directive);
}
		|
{
}
		;

secinfo		: loadsec runsec
{
}
		| runsec loadsec
{
}
		;

loadsec		: '>' ID secoptions
{
	/* LOAD allocation rule */
	tmp_dir_scn.mem = dld_strdup($2->name, "dir_scn->mem");

	/* Add properties related to this load allocation */
	dir_scn_append(&cur_directive->load, &tmp_dir_scn);
	dir_scn_flush(&tmp_dir_scn);
	expr_tree_free($2);
}
		| LOAD '=' ID secoptions
{
	/* LOAD allocation rule */
	tmp_dir_scn.mem = dld_strdup($3->name, "dir_scn->mem");

	/* Add properties related to this load allocation */
	dir_scn_append(&cur_directive->load, &tmp_dir_scn);
	dir_scn_flush(&tmp_dir_scn);
	expr_tree_free($3);
}
		| LOAD '=' CONSTANT secoptions
{
	/* LOAD allocation rule */
	tmp_dir_scn.addr = $3->val;

	/* Add properties related to this load allocation */
	dir_scn_append(&cur_directive->load, &tmp_dir_scn);
	dir_scn_flush(&tmp_dir_scn);
	expr_tree_free($3);
}
		|
{
	dir_scn_append(&cur_directive->load, &tmp_dir_scn);
	dir_scn_flush(&tmp_dir_scn);
}
		;

runsec		: RUN '=' ID secoptions
{
	/* RUN allocation rule */
	tmp_dir_scn.mem = dld_strdup($3->name, "dir_scn->mem");

	/* Add a property for MEMORY directive */
	dir_scn_append(&cur_directive->run, &tmp_dir_scn);
	dir_scn_flush(&tmp_dir_scn);
	expr_tree_free($3);
}
		| RUN '>' ID secoptions
{
	/* RUN allocation rule */
	tmp_dir_scn.mem = dld_strdup($3->name, "dir_scn->mem");

	/* Add properties related to this run allocation */
	dir_scn_append(&cur_directive->run, &tmp_dir_scn);
	dir_scn_flush(&tmp_dir_scn);
	expr_tree_free($3);
}
		|
{
}
		;

secname		: ID
{
	/* Allocate section name */
	if (cur_directive->scnnm == NULL) {
		cur_directive->scnnm = dld_strdup($1->name, "directive->scnnm");
	} else {
		prmsg("*** ID ***\n");
	}
	expr_tree_free($1);
}
		| ID secoptions ':'
{
	/* Allocate section name */
	if (cur_directive->scnnm == NULL) {
		cur_directive->scnnm = dld_strdup($1->name, "directive->scnnm");
	} else {
		prmsg("*** ID secoptions ***\n");
	}
	expr_tree_free($1);
}
		;

secoptions	: secoption secoptions
{
}
		| secoption ',' secoptions
{
}
		| 
{
}
		;

/*
 * Following allocation parameters are added to a temporary list.
 * Because it doesn't know whether this parameter is for load or run.
 * Parameters added to the temporary list splice an appropriate list,
 * when this section is parsed.
 */
secoption	: ALIGN '(' CONSTANT ')'
{
	tmp_dir_scn.align = $3->val;
	expr_tree_free($3);
}
		| ALIGN '=' CONSTANT
{
	tmp_dir_scn.align = $3->val;
	expr_tree_free($3);
}
		| BLOCK '(' CONSTANT ')'
{
	tmp_dir_scn.block = $3->val;
	expr_tree_free($3);
}
		| BLOCK '=' CONSTANT
{
	tmp_dir_scn.block = $3->val;
	expr_tree_free($3);
}
		| FILL '(' CONSTANT ')'
{
	tmp_dir_scn.fill = $3->val;
	expr_tree_free($3);
}
		| FILL '=' CONSTANT
{
	tmp_dir_scn.fill = $3->val;
	expr_tree_free($3);
}
		| '(' COPY ')'
{
	tmp_dir_scn.stype |= DIRSCN_STYPE_COPY;
}
		| '(' DSECT ')'
{
	tmp_dir_scn.stype |= DIRSCN_STYPE_DSECT;
}
		| '(' NOLOAD ')'
{
	tmp_dir_scn.stype |= DIRSCN_STYPE_NOLOAD;
}
		| PAGE CONSTANT
{
	if ($2->val != 0) {
		prmsg("PAGE %d\n", $2->val);
		prmsg("SECTIONS directive: PAGE option other than 0 is not supported.\n");
		y_err = -1;
		YYABORT;
	}
	expr_tree_free($2);
}
		| 
{
}
		;

inputsections	: inputsections inputsection
{
}
		|
{
}
		;

inputsection	: ID '(' ID ')'
{
	/* Specify an input section from an input file */
	struct dir_iscn *p;
	p = dir_iscn_new_iscn($1->name, $3->name);
	list_add_tail((struct list_head *)p, &cur_directive->iscnlist);
	expr_tree_free($1);
	expr_tree_free($3);
}
		| '*' '(' ID ')'
{
	/* Specify an input section from all input files */
	struct dir_iscn *p;
	p = dir_iscn_new_iscn("*", $3->name);
	list_add_tail((struct list_head *)p, &cur_directive->iscnlist);
	expr_tree_free($3);
}
		| inputassign
{
}
		;

inputassign	: '.' '=' ALIGN '(' CONSTANT ')' ';'
{
	/* Create a hole to align '.' */
	struct dir_iscn *p;
	p = dir_iscn_new_hole(HL_EQ_ALIGN, $5->val);
	list_add_tail((struct list_head *)p, &cur_directive->iscnlist);
	expr_tree_free($5);

}
		| '.' ADD_ASSIGN CONSTANT ';'
{
	/* Create a hole with specified size */
	struct dir_iscn *p;
	p = dir_iscn_new_hole(HL_EQPLUS_ALIGN, $3->val);
	list_add_tail((struct list_head *)p, &cur_directive->iscnlist);
	expr_tree_free($3);

}
		| '.' '=' CONSTANT ';'
{
	/* Create a hole with specified size */
	struct dir_iscn *p;
	p = dir_iscn_new_hole(HL_EQ, $3->val);
	list_add_tail((struct list_head *)p, &cur_directive->iscnlist);
	expr_tree_free($3);

}
		| assignment
{
	/*
	 * Define global symbols and assign values to them
	 * A statement has binary tree structure.
	 */
	struct dir_iscn *p;
	p = dir_iscn_new_expr($1);
	list_add_tail((struct list_head *)p, &cur_directive->iscnlist);
	/*
	 * so don't free.
	 */
}

		;

assignment	: ID '=' E ';'
{
	struct expr_tree *p;
	p = expr_tree_new(ET_EQ, 0, NULL);
	list_splice_tail_nostub((struct list_head*)p,  (struct list_head*)$1);
	list_splice_tail_nostub((struct list_head*)$3, (struct list_head*)$1);
	$$ = $1; // return first element
}
		;

E		: E '+' T
{
	struct expr_tree *p;
	p = expr_tree_new(ET_PLUS, 0, NULL);
	list_splice_tail_nostub((struct list_head*)p,  (struct list_head*)$1);
	list_splice_tail_nostub((struct list_head*)$3, (struct list_head*)$1);
	$$ = $1;
}
		| E '-' T
{
	struct expr_tree *p;
	p = expr_tree_new(ET_MINUS, 0, NULL);
	list_splice_tail_nostub((struct list_head*)p,  (struct list_head*)$1);
	list_splice_tail_nostub((struct list_head*)$3, (struct list_head*)$1);
	$$ = $1;
}
		| T
{
}
		;

T		: T '*' F
{
	struct expr_tree *p;
	p = expr_tree_new(ET_MULT, 0, NULL);
	list_splice_tail_nostub((struct list_head*)p,  (struct list_head*)$1);
	list_splice_tail_nostub((struct list_head*)$3, (struct list_head*)$1);
	$$ = $1;
}
		| T '/' F
{
	struct expr_tree *p;
	p = expr_tree_new(ET_DIV, 0, NULL);
	list_splice_tail_nostub((struct list_head*)p,  (struct list_head*)$1);
	list_splice_tail_nostub((struct list_head*)$3, (struct list_head*)$1);
	$$ = $1;
}
		| F
{
	$$ = $1;
}
		;

F		: ID
{
	$$ = $1;
}
		| CONSTANT
{
	$$ = $1;
}
		| '(' E ')'
{
	$$ = $2;
}
		| '.'
{
	$$ = expr_tree_new(ET_VAR, 0, ".");
}
		;



%%

#include "lex.yy.c"


int yyerror(char *s)
{
	prmsg("%s\n", s);
	return -1;
}

/**
 * lkcmd_read - parse a command file
 * @lkcmd: pointer to lkcmd struct to be stored
 *
 * Parse a command file and build each lists.
 */
int lkcmd_read(struct lkcmd *lkcmd)
{
	yyin = fopen(lkcmd->fn, "r");
	if (yyin == NULL) {
		prmsg("%s open failed\n", lkcmd->fn);
		return -1;
	}

	user_lkcmd = lkcmd;
	y_err = 0;
	dir_scn_init(&tmp_dir_scn);

	yyparse();

	dir_scn_clear(&tmp_dir_scn);
	fclose(yyin);

	if ((yynerrs > 0) || (y_err < 0))
		return -1;
	return 0;
}
