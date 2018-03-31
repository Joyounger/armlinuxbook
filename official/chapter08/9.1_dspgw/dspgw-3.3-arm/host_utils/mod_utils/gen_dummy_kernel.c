/*
 * gen_dummy_kernel.c
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
 * 2005/01/13:  DSP Gateway version 3.2
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
static char *g_outfn;

int main(int argc, char **argv)
{
	struct coffobj *cobj;
	struct section *scn_tk;
	struct list_head *ptr, *next_ptr;
	struct symbol *sym;
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

	/* header */
	cobj->flags = COFF_F_UPDATE | COFF_F_SWABD | COFF_F_AR16WR | COFF_F_AR32W;
	if (cobj->aouthdr) {
		free(cobj->aouthdr);
		cobj->aouthdr = NULL;
	}

	/* delete all sections and create one */
	for (ptr = cobj->scnlist.next;
	     ptr != &cobj->scnlist;
	     ptr = next_ptr) {
		struct section *scn = (struct section *)ptr;
		next_ptr = ptr->next;
		list_del(ptr);
		section_free(scn);
	}
	scn_tk = section_new();
	scn_tk->name = strdup(".tinkernel");
	scn_tk->flags = COFF_STYP_DATA | COFF_STYP_RDATA;
	list_add_tail((struct list_head *)scn_tk, &cobj->scnlist);

	/* all pointer symbols should point ".tinkernel" section */
	symbol_for_each(sym, &cobj->symlist) {
		if (!symbol_is_absolute(sym) && !symbol_is_debug(sym)) {
			sym->scn = scn_tk;
			sym->value = 0;
		}
	}

	coff_write(cobj, g_outfn);

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
	g_outfn = NULL;

	for (i = 1; i < argc; i++) {
		if (!strcmp(argv[i], "-o")) {
			if (++i >= argc)
				goto err;
			g_outfn = argv[i];
		} else if (g_fn == NULL)
			g_fn = argv[i];
		else
			goto err;
	}

	if (g_fn == NULL)
		goto err;
	if (g_outfn == NULL)
		goto err;

	return 0;

err:
	fprintf(stderr,
		"usage: %s <filename> -o <outfn>\n", argv[0]);
	return -1;
}
