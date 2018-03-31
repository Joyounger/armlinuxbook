/*
 * coff_unresolve.c
 *
 * Copyright (C) 2004,2005 Nokia Corporation
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
 * 2004/07/23:  DSP Gateway version 3.2
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <malloc.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/ioctl.h>
#include "list.h"
#include "coff-c55x.h"
#include "cofflib.h"
#include "coffobj.h"
#include "section.h"
#include "symbol.h"

/*
 * function declarations
 */
static int read_opt(int argc, char **argv);

/*
 * global variables
 */
static char *g_fn;
static char *g_dummy_scnnm;

int main(int argc, char **argv)
{
	struct coffobj *cobj;
	struct section *scn, *dummy_scn;
	struct list_head *ptr, *next_ptr;
	int ret = 0;

	/* parse command line option */
	if (read_opt(argc, argv) < 0) {
		return 1;
	}

	cobj = coff_new();
	if (coff_read(cobj, g_fn) < 0) {
		ret = 1;
		goto cleanup;
	}

	dummy_scn = NULL;
	section_for_each(scn, &cobj->scnlist) {
		int i;
		/* find dummy section */
		if (!strcmp(scn->name, g_dummy_scnnm)) {
			dummy_scn = scn;
			break;
		}
		/* symbol reference count check */
		for (i = 0; i < scn->nreloc; i++) {
			struct reloc *rel = &scn->reloc[i];
			if (rel_sym_field_type(rel->type) ==
			    RELOC_SYMBOL_FIELD_TYPE_SYMBOL) {
				rel->sym.sym->refcnt++;
			}
		}
	}
	if (dummy_scn == NULL) {
		fprintf(stderr, "section %s not found in object.\n",
			g_dummy_scnnm);
		ret = 1;
		goto cleanup;
	}

	for (ptr = cobj->symlist.next;
	     ptr != &cobj->symlist;
	     ptr = next_ptr) {
		struct symbol *sym = (struct symbol *)ptr;
		next_ptr = ptr->next;
		if (sym->scn == dummy_scn) {
			if (sym->refcnt == 0) {
				/* delete symbols which are not referred */
				list_del(ptr);
				symbol_free(sym);
			} else
				/* move to unresolved */
				sym->scn = NULL;
		} else if (symbol_is_absolute(sym) ||
			   symbol_is_debug(sym)) {
			/* delete absolute or debug symbols */
			list_del(ptr);
			symbol_free(sym);
		}
	}

	list_del((struct list_head *)dummy_scn);
	section_free(dummy_scn);

	section_set_out_data_align(2);
	coff_write(cobj, g_fn);

cleanup:
	section_freelist(&cobj->scnlist);
	symbol_freelist(&cobj->symlist);
	coff_free(cobj);

	return ret;
}

static int read_opt(int argc, char **argv)
{
	int i;

	g_fn = NULL;

	for (i = 1; i < argc; i++) {
		if (!strcmp(argv[i], "-s")) {
			if (++i >= argc)
				goto err;
			g_dummy_scnnm = argv[i];
		} else if (g_fn == NULL)
			g_fn = argv[i];
		else
			goto err;
	}

	if (g_fn == NULL)
		goto err;
	if (g_dummy_scnnm == NULL)
		goto err;

	return 0;

err:
	fprintf(stderr,
		"usage: %s <filename> -s <dummy_section>\n", argv[0]);
	return -1;
}
