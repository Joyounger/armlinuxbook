/*
 * dsp_dld/arm/dld_symbol.c
 *
 * DSP Dynamic Loader Daemon: dld_symbol.c
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
#include <unistd.h>
#include <string.h>
#include <malloc.h>
#include "list.h"
#include "coff-c55x.h"
#include "dsp_dld.h"
#include "dld_malloc.h"
#include "dld_daemon.h"
#include "dld_coff.h"

#ifdef STICKY_LIST
static LIST_HEAD(g_symlist);
#endif

struct symbol *symbol_find_by_name(struct list_head *list, char *name);

/*
 * dummy sections
 */
static struct section section_absolutesymbol = {
	.list_head  = { NULL, NULL },
	.name       = "Absolute Symbol",
	.vaddr_orig = 0,
	.vaddr      = 0,
	.size       = 0,
	.data       = NULL,
	.nreloc     = 0,
	.reloc      = NULL,
	.flags      = 0
};

static struct section section_debugsymbol = {
	.list_head  = { NULL, NULL },
	.name       = "Debug Symbol",
	.vaddr_orig = 0,
	.vaddr      = 0,
	.size       = 0,
	.data       = NULL,
	.nreloc     = 0,
	.reloc      = NULL,
	.flags      = 0
};

/*
 * create and fill functions should be separated since
 * section and symbol refere each otehr with their index number.
 * It means fill_scnlist() needs symbol instances as well as
 * fill_symlist() needs section instances.
 */

void symbol_createlist(struct list_head *list, u16 n)
{
	u16 i;

	for (i = 0; i < n; i++) {
		struct symbol *sym = dld_malloc(sizeof(struct symbol), "symbol");
		list_add_tail(&sym->list_head, list);
		sym->name = NULL;
	}
}

void symbol_freelist(struct list_head *list)
{
	struct symbol *sym, *tmp;

#ifdef STICKY_LIST
	if (list == NULL)
		list = &g_symlist;
#endif
	symbol_for_each_safe(sym, tmp, list) {
		list_del(&sym->list_head);
		if (sym->name)
			dld_free(sym->name, "symbol->name");
		dld_free(sym, "symbol");
	}
}

void symbol_filllist(struct coff *coff, u8 *src, struct coffobj *cobj, u16 n)
{
	struct list_head *ptr = &cobj->symlist;
	int i;

	for (i = 0; i < n; i++) {
#if 0
		/* arm-gcc doesn't like non-dword-sized structure array */
		COFF_SYMENT *symsrc = &coff->symtbl[i];
#else
		COFF_SYMENT *symsrc = (void *)coff->symtbl + (COFF_SYMESZ * i);
#endif
		struct symbol *sym;
		u16 scnum;

		ptr = ptr->next;
		sym = (struct symbol *)ptr;
		sym->name = dld_strdup(symname(coff, symsrc), "symbol->name");
		sym->value = COFF_LONG(symsrc->e_value);
		sym->value_orig = sym->value;
		scnum = COFF_SHORT(symsrc->e_scnum);
		sym->scn = (scnum ==      0) ? NULL :	/* unresolved */
			   (scnum == 0xffff) ? &section_absolutesymbol :	/* absolute */
			   (scnum == 0xfffe) ? &section_debugsymbol :	/* debug */
					       list_entry(list_find_by_idx(&cobj->scnlist, scnum-1),
							  struct section, list_head);
		sym->type   = COFF_SHORT(symsrc->e_type);
		sym->sclass = *symsrc->e_sclass;

		/* delete aux entry */
		if (*symsrc->e_numaux) {
			struct list_head *next = ptr->next;

			list_del(next);
			dld_free(list_entry(next, struct symbol, list_head), "symbol");
			i++;
		}
	}
}

#ifdef STICKY_LIST
int symbol_register_global(struct list_head *list)
{
	struct list_head *ptr, *next;
	int cnt = 0;

	for (ptr = list->next; ptr != list; ptr = next) {
		struct symbol *sym = (struct symbol *)ptr;

		next = ptr->next;	/* fix next pointer before deleted */
		if (sym->sclass == COFF_C_EXT) {
			list_del(ptr);
/*
			sym->scn = &section_absolutesymbol;
*/
			list_add_tail(ptr, &g_symlist);
			cnt++;
		}
	}
	return cnt;
}
#endif /* STICKY_LIST */

struct symbol *symbol_find_by_name(struct list_head *list, char *name)
{
	struct symbol *sym;

#ifdef STICKY_LIST
	if (list == NULL)
		list = &g_symlist;
#endif
	symbol_for_each(sym, list) {
		if (!strcmp(sym->name, name))
			return sym;
	}

	/* not found */
	return NULL;
}

struct symbol *symbol_find_scnsym(struct list_head *list, struct section *scn)
{
	struct symbol *sym;

	symbol_for_each(sym, list) {
		if ((sym->scn == scn) &&
		    (sym->type == COFF_C_STAT) &&
		    (!strcmp(sym->name, scn->name)))
			return sym;
	}

	/* not found */
	return NULL;
}

void symbol_sendstat(struct list_head *list, int fd)
{
	char buf[256];
	size_t strsz = 256 - SERVER_EVENT_HDRSZ;
	struct server_event *e = (struct server_event *)buf;
	struct symbol *sym;
	int cnt;

#ifdef STICKY_LIST
	if (list == NULL)
		list = &g_symlist;
#endif

	e->event = DLD_EVENT_STRING;

	cnt = snprintf(e->data.s, strsz, "%-30s %-10s %-20s %-6s %-4s\n",
		       "name", "value", "section", "type", "clas");
	e->len = SERVER_EVENT_HDRSZ + cnt;
	write(fd, e, e->len);

	symbol_for_each(sym, list) {
		cnt = snprintf(e->data.s, strsz,
			       "%-30s 0x%08lx %-20s 0x%04x 0x%02x\n",
			       sym->name,
			       sym->value,
			       sym->scn ? sym->scn->name : "NULL",
			       sym->type,
			       sym->sclass);
		e->len = SERVER_EVENT_HDRSZ + cnt;
		write(fd, e, e->len);
	}
}

/**
 * symbol_resolve
 *
 * @list_knl: kernel symbol list (can be NULL ifdef STICK_LIST)
 * @list: New symbol list, which will contain unresolved symbols
 *
 * Set appropriate parameters for unresolved symbol in list using list_knl
 *
 */
int symbol_resolve(struct list_head *list_knl, struct list_head *list)
{
	struct symbol *sym;
	struct symbol *s;

	symbol_for_each(sym, list) {
		/*
		 * FIXME: temporary fix for duplicated object
		 * library handling should be considered.
		 */
		if ((sym->sclass == COFF_C_EXT) && (sym->scn != NULL)) {
			s = symbol_find_by_name(list_knl, sym->name);
			if (s != NULL) {
				prmsg("Warning: symbol %s is found in "
				      "both kernel and task.\n"
				      "We use one in kernel.\n", sym->name);
				sym->scn = s->scn;
				sym->value = s->value;
			}
		}

		/* Find unresolved symbol in list */
		if ((sym->sclass == COFF_C_EXT) && (sym->scn == NULL)) {
			/* Then find it in global list */
			s = symbol_find_by_name(list_knl, sym->name);
			if (s != NULL) {
				sym->scn = s->scn;
				sym->value = s->value;
				sym->type = s->type;
				sym->sclass = s->sclass;
			} else {
				prmsg("symbol %s not found\n", sym->name);
				return -1;
			}
		}
	}
	return 0;
}

/*
 * debug stuff
 */
void symbol_printstat(struct list_head *list)
{
	struct symbol *sym;

#ifdef STICKY_LIST
	if (list == NULL)
		list = &g_symlist;
#endif
	prmsg("symbol list status ...\n");
	prmsg("  %-30s %-10s %-20s %-6s %-4s\n",
	      "name", "value", "section", "type", "clas");
	symbol_for_each(sym, list) {
		prmsg("  %-30s 0x%08lx %-20s 0x%04x 0x%02x\n",
		      sym->name,
		      sym->value,
		      sym->scn ? sym->scn->name : "NULL",
		      sym->type,
		      sym->sclass);
	}
}
