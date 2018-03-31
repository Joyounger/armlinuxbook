/*
 * section.c
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
 * 2004/12/09:  DSP Gateway version 3.2
 */

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <malloc.h>
#include <fcntl.h>
#include "list.h"
#include "coff-c55x.h"
#include "cofflib.h"
#include "coffobj.h"
#include "symbol.h"
#include "section.h"

static u8 out_data_align = 1;
static u8 dummy_section_data[1];

/*
 * dummy symbols
 */
static struct symbol symbol_internalreloc = {
	.list_head  = { NULL, NULL },
	.name       = "Internal Relocation",
	.value      = 0,
	.scn        = NULL,
	.type       = COFF_T_NULL,
	.sclass     = COFF_C_NULL,
	.aux        = NULL,
	.refcnt     = 0
};

void section_set_out_data_align(int align)
{
	out_data_align = align;
}

/*
 * create and fill functions should be separated since
 * section and symbol refere each otehr with their index number.
 * It means fill_scnlist() needs symbol instances as well as
 * fill_symlist() needs section instances.
 */
struct section *section_new(void)
{
	struct section *scn = malloc(sizeof(struct section));
	memset(scn, 0, sizeof(struct section));
	return scn;
}

void section_createlist(struct list_head *list, u16 n)
{
	int i;

	for (i = 0; i < n; i++) {
		struct section *scn = section_new();
		list_add_tail(&scn->list_head, list);
	}
}

void section_free(struct section *scn)
{
	if (scn->name)
		free(scn->name);
	/* data might be pointing dummy data */
	if ((scn->data) && (scn->data != dummy_section_data))
		free(scn->data);
	if (scn->reloc)
		free(scn->reloc);
	free(scn);
}

void section_freelist(struct list_head *list)
{
	struct section *scn, *tmp;

	list_for_each_entry_safe(scn, tmp, list, list_head) {
		list_del(&scn->list_head);
		section_free(scn);
	}
}

int rel_sym_field_type(u16 type)
{
	switch (type) {
		case COFF_R_RELBYTE:
		case COFF_R_RELWORD:
		case COFF_R_LD3_REL16:
		case COFF_R_REL24:
		case COFF_R_LD3_REL23:
		case COFF_R_RELLONG:
		case COFF_R_LD3_DMA:
		case COFF_R_LD3_MDP:
		case COFF_R_LD3_PDP:
		case COFF_R_LD3_k8:
		case COFF_R_LD3_k16:
		case COFF_R_LD3_K8:
		case COFF_R_LD3_K16:
		case COFF_R_LD3_l8:
		case COFF_R_LD3_l16:
		case COFF_R_LD3_L8:
		case COFF_R_LD3_L16:
		case COFF_R_LD3_k4:
		case COFF_R_LD3_k5:
		case COFF_R_LD3_K5:
		case COFF_R_LD3_k6:
		case COFF_R_LD3_k12:
		case COFF_R_EX_PSHSYM:
			return RELOC_SYMBOL_FIELD_TYPE_SYMBOL;
		case COFF_R_EX_PSHVAL:
		case COFF_R_EX_PSHINTOFF:
		case COFF_R_EX_WRITE1:
		case COFF_R_EX_WRITE2:
		case COFF_R_EX_WRITE3:
			return RELOC_SYMBOL_FIELD_TYPE_VALUE;
		default:
			return RELOC_SYMBOL_FIELD_TYPE_INVALID;
	}
}

void section_filllist(struct coff *coff, u8 *src, struct coffobj *cobj, u16 n)
{
	struct list_head *ptr = &cobj->scnlist;
	int i, j;

	for (i = 0; i < n; i++) {
#if 0
		/* arm-gcc doesn't like non-dword-sized structure array */
		COFF_SCNHDR *scnsrc = &coff->scnhdr[i];
#else
		COFF_SCNHDR *scnsrc = (void *)coff->scnhdr + (COFF_SCNHSZ * i);
#endif
		struct section *scn;
		u32 scnptr;
		u32 relptr;

		ptr = ptr->next;
		scn = (struct section *)ptr;
		scn->name = strdup(scnname(coff, scnsrc->s_name));
		scn->paddr = COFF_LONG(scnsrc->s_paddr);
		scn->vaddr = COFF_LONG(scnsrc->s_vaddr);
		scn->size = COFF_LONG(scnsrc->s_size);
		scn->nreloc = COFF_LONG(scnsrc->s_nreloc);
		scn->flags = COFF_LONG(scnsrc->s_flags);
		scn->cobj = cobj;
		scnptr = COFF_LONG(scnsrc->s_scnptr);
		relptr = COFF_LONG(scnsrc->s_relptr);

		if (scnptr) {
			scn->data = malloc(scn->size);
			memcpy(scn->data, &src[scnptr], scn->size);
		} else
			scn->data = NULL;

		if (relptr) {
			COFF_RELOC *srel = (COFF_RELOC *)&src[relptr];
			scn->reloc = malloc(sizeof(struct reloc) * scn->nreloc);
			for (j = 0; j < scn->nreloc; j++) {
				COFF_RELOC *relsrc = &srel[j];
				struct reloc *rel = &scn->reloc[j];
				u32 symndx;

				rel->vaddr = COFF_LONG(relsrc->r_vaddr);
				rel->exa   = COFF_SHORT(relsrc->r_exa);
				rel->type  = COFF_SHORT(relsrc->r_type);
				symndx     = COFF_LONG(relsrc->r_symndx);
				if (rel_sym_field_type(rel->type) ==
				    RELOC_SYMBOL_FIELD_TYPE_SYMBOL) {
					if (symndx == 0xffffffff)
						/* internal relocation */
						rel->sym.sym = &symbol_internalreloc;
					else
						rel->sym.sym = (struct symbol *)list_find_by_idx(&cobj->symlist, symndx);
				} else if (rel_sym_field_type(rel->type) ==
					   RELOC_SYMBOL_FIELD_TYPE_VALUE) {
					rel->sym.val = symndx;
				}
			}
		} else
			scn->reloc = NULL;
	}
}

void section_putlist(struct coff *coff, u8 *dst, struct coffobj *cobj, u32 n)
{
	struct section *scn;
	COFF_SCNHDR *scnhdr;
	void *dst_data = (void *)coff->scnhdr + COFF_SCNHSZ * n;

	/* static info */
	scnhdr = coff->scnhdr;
	section_for_each(scn, &cobj->scnlist) {
		scnname_put(coff, scn->name, scnhdr->s_name);
		COFF_PUT_LONG(scn->paddr,  scnhdr->s_paddr);
		COFF_PUT_LONG(scn->vaddr,  scnhdr->s_vaddr);
		COFF_PUT_LONG(scn->size,   scnhdr->s_size);
		COFF_PUT_LONG(scn->nreloc, scnhdr->s_nreloc);
		COFF_PUT_LONG(scn->flags,  scnhdr->s_flags);
		COFF_PUT_LONG(0,           scnhdr->s_scnptr); /* temp */
		COFF_PUT_LONG(0,           scnhdr->s_relptr); /* temp */
		scnhdr = (void *)scnhdr + COFF_SCNHSZ;
	}

	/* section data */
	scnhdr = coff->scnhdr;
	section_for_each(scn, &cobj->scnlist) {
		u32 scnptr;

		if (scn->data == NULL)
			goto next_data;

		scnptr = (u32)dst_data - (u32)dst;
		COFF_PUT_LONG(scnptr, scnhdr->s_scnptr);
		memcpy(dst_data, scn->data, scn->size);
		dst_data += scn->size;
		while ((u32)dst_data & (out_data_align - 1))
			*(char *)(dst_data++) = 0;
next_data:
		scnhdr = (void *)scnhdr + COFF_SCNHSZ;
	}

	/* reloc info */
	scnhdr = coff->scnhdr;
	section_for_each(scn, &cobj->scnlist) {
		u32 relptr;
		int i;

		if (scn->nreloc == 0)
			goto next_reloc;

		relptr = (u32)dst_data - (u32)dst;
		COFF_PUT_LONG(relptr, scnhdr->s_relptr);

		for (i = 0; i < scn->nreloc; i++) {
			struct reloc *rel = &scn->reloc[i];
			COFF_RELOC *reldst = (COFF_RELOC *)dst_data;
			u32 symndx;

			if (rel_sym_field_type(rel->type) ==
			    RELOC_SYMBOL_FIELD_TYPE_SYMBOL) {
				if (rel->sym.sym == &symbol_internalreloc)
					/* internal relocation */
					symndx = 0xffffffff;
				else
					symndx = symbol_idx(&cobj->symlist, rel->sym.sym);
			} else if (rel_sym_field_type(rel->type) ==
				   RELOC_SYMBOL_FIELD_TYPE_VALUE) {
				symndx = rel->sym.val;
			} else
				symndx = 0;

			COFF_PUT_LONG(rel->vaddr, reldst->r_vaddr);
			COFF_PUT_SHORT(rel->exa,  reldst->r_exa);
			COFF_PUT_SHORT(rel->type, reldst->r_type);
			COFF_PUT_LONG(symndx,     reldst->r_symndx);
			dst_data += COFF_RELSZ;
		}

next_reloc:
		scnhdr = (void *)scnhdr + COFF_SCNHSZ;
	}
}

size_t section_total_datasz(struct list_head *list)
{
	struct section *scn;
	size_t cnt = 0;

	section_for_each(scn, list) {
		if (scn->data) {
			cnt += scn->size;
			while (cnt & (out_data_align - 1))
				cnt++;
		}
		cnt += scn->nreloc * COFF_RELSZ;
	}

	return cnt;
}

size_t section_total_strtblsz(struct list_head *list)
{
	struct section *scn;
	size_t cnt = 0;

	section_for_each(scn, list) {
		u32 len = strlen(scn->name);
		if (len > 8)
			cnt += len + 1;
	}

	return cnt;
}

struct section *section_find_by_name(struct list_head *list, char *name)
{
	struct section *scn;

	/* search in local list */
	section_for_each(scn, list) {
		if (!strcmp(scn->name, name))
			return scn;
	}

	/* not found */
	return NULL;
}

