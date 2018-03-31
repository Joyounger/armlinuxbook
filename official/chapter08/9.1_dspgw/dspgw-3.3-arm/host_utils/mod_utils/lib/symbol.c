/*
 * symbol.c
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
 * 2004/07/22:  DSP Gateway version 3.2
 */

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <malloc.h>
#include "list.h"
#include "coff-c55x.h"
#include "cofflib.h"
#include "coffobj.h"

/*
 * dummy sections
 */
static struct section section_absolutesymbol = {
	.list_head  = { NULL, NULL },
	.name       = "Absolute Symbol",
	.paddr      = 0,
	.vaddr      = 0,
	.size       = 0,
	.data       = NULL,
	.nreloc     = 0,
	.reloc      = NULL,
	.flags      = 0,
	.cobj       = NULL
};

static struct section section_debugsymbol = {
	.list_head  = { NULL, NULL },
	.name       = "Debug Symbol",
	.vaddr      = 0,
	.paddr      = 0,
	.size       = 0,
	.data       = NULL,
	.nreloc     = 0,
	.reloc      = NULL,
	.flags      = 0,
	.cobj       = NULL
};

/*
 * create and fill functions should be separated since
 * section and symbol refere each otehr with their index number.
 * It means fill_scnlist() needs symbol instances as well as
 * fill_symlist() needs section instances.
 */

struct symbol *symbol_new(void)
{
	struct symbol *sym = malloc(sizeof(struct symbol));
	memset(sym, 0, sizeof(struct symbol));
	return sym;
}

void symbol_createlist(struct list_head *list, u16 n)
{
	u16 i;

	for (i = 0; i < n; i++) {
		struct symbol *sym = symbol_new();
		list_add_tail(&sym->list_head, list);
	}
}

void symbol_free(struct symbol *sym)
{
	if (sym->name)
		free(sym->name);
	if (sym->aux)
		free(sym->aux);
	free(sym);
}

void symbol_freelist(struct list_head *list)
{
	struct symbol *sym;
	struct symbol *tmp;

	list_for_each_entry_safe(sym, tmp, list, list_head) {
		list_del(&sym->list_head);
		symbol_free(sym);
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
		sym->name = strdup(symname(coff, symsrc));
		sym->value = COFF_LONG(symsrc->e_value);
		scnum = COFF_SHORT(symsrc->e_scnum);
		sym->scn = (scnum ==      0) ? NULL :	/* unresolved */
			   (scnum == 0xffff) ? &section_absolutesymbol :	/* absolute */
			   (scnum == 0xfffe) ? &section_debugsymbol :	/* debug */
					       list_entry(list_find_by_idx(&cobj->scnlist, scnum-1),
							  struct section, list_head);
		sym->type   = COFF_SHORT(symsrc->e_type);
		sym->sclass = *symsrc->e_sclass;

		if (*symsrc->e_numaux) {
			u8 *auxent = (void *)symsrc + COFF_SYMESZ;
			/* delete aux entry */
			list_del(ptr->next);
			sym->aux = malloc(COFF_SYMESZ);
			memcpy(sym->aux, auxent, COFF_SYMESZ);
			i++;
		} else
			sym->aux = NULL;
	}
}

void symbol_putlist(struct coff *coff, struct coffobj *cobj)
{
	struct symbol *sym;
	COFF_SYMENT *dst = coff->symtbl;

	symbol_for_each(sym, &cobj->symlist) {
		u16 scnum;

		symname_put(coff, sym->name, dst);
		COFF_PUT_LONG(sym->value, dst->e_value);
		scnum = (sym->scn == NULL) ? 0 :
			(sym->scn == &section_absolutesymbol) ? 0xffff :
			(sym->scn == &section_debugsymbol) ? 0xfffe :
			list_idx(&cobj->scnlist, &sym->scn->list_head) + 1;
		COFF_PUT_SHORT(scnum, dst->e_scnum);
		COFF_PUT_SHORT(sym->type, dst->e_type);
		*dst->e_sclass = sym->sclass;
		*dst->e_numaux = sym->aux ? 1 : 0;

		if (sym->aux) {
			dst = (void *)dst + COFF_SYMESZ;
			memcpy(dst, sym->aux, COFF_SYMESZ);
		}

		dst = (void *)dst + COFF_SYMESZ;
	}
}

u32 symbol_countlist(struct list_head *list)
{
	struct symbol *sym;
	u32 n = 0;

	symbol_for_each(sym, list) {
		n++;
		if (sym->aux)
			n++;
	}
	return n;
}

u32 symbol_idx(struct list_head *list, struct symbol *ent)
{
	struct symbol *sym;
	int n = 0;

	symbol_for_each(sym, list) {
		if (sym == ent)
			return n;
		n++;
		if (sym->aux)
			n++;
	}
	return -1;
}

size_t symbol_total_strtblsz(struct list_head *list)
{
	struct symbol *sym;
	size_t cnt = 0;

	symbol_for_each(sym, list) {
		u32 len = strlen(sym->name);
		if (len > E_SYMNMLEN)
			cnt += len + 1;
	}

	return cnt;
}

int symbol_is_absolute(struct symbol *sym)
{
	return (sym->scn == &section_absolutesymbol);
}

int symbol_is_debug(struct symbol *sym)
{
	return (sym->scn == &section_debugsymbol);
}
