/*
 * dsp_dld/arm/dld_section.c
 *
 * DSP Dynamic Loader Daemon: dld_section.c
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
 * 2005/07/11:  DSP Gateway version 3.3
 */

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <malloc.h>
#include <fcntl.h>
#include "list.h"
#include "coff-c55x.h"
#include "dsp_dld.h"
#include "dld_daemon.h"
#include "dld_malloc.h"
#include "dld_coff.h"
#include "dld_memmgr.h"

#define RELOC_SYMBOL_FIELD_TYPE_INVALID	0
#define RELOC_SYMBOL_FIELD_TYPE_SYMBOL	1
#define RELOC_SYMBOL_FIELD_TYPE_VALUE	2

#define LDTYP_LOAD	1
#define LDTYP_NOLOAD	2
#define LDTYP_NODATA	3
#define LDTYP_ZEROSZ	4
#define LDTYP_CINIT_RAM	5

#ifdef STICKY_LIST
static LIST_HEAD(g_scnlist);
#endif
static u8 dummy_section_data[1];

static int load_to_dspmem(int fd, struct taskent *te, u32 addr,
			  u32 size, u8 *data);
#ifndef DSP_EMULATION
static void copy_byteswap(u8 *dst, u8 *src, size_t size);
#endif
static int cinit_bss_init(int fd, struct taskent *te, struct section *scn);
static int extra_relocate(struct section *scn, int i);
inline int rel_sym_field_type(u16 type);
static char *reloc_name(u16 type);
#ifdef DEBUG_RELOCATE
#define rel_sym_delta(scn,rel,org,new)	__rel_sym_val(scn,rel,org,new,1)
#define rel_sym_val(scn,rel)	__rel_sym_val(scn,rel,NULL,NULL,0)
static u32 __rel_sym_val(struct section *scn, struct reloc *rel,
			 u32 *org, u32 *new, int delta);
#else
#define rel_sym_delta(scn,rel)	__rel_sym_val(scn,rel,1)
#define rel_sym_val(scn,rel)	__rel_sym_val(scn,rel,0)
static u32 __rel_sym_val(struct section *scn, struct reloc *rel, int delta);
#endif
static int section_loadtype(struct section *scn);
static struct section *section_find_by_addr(struct list_head *list, u32 addr);

/*
 * dummy symbols
 */
static struct symbol symbol_internalreloc = {
	.list_head  = { NULL, NULL },
	.name       = "Internal Relocation",
	.value_orig = 0,
	.value      = 0,
	.scn        = NULL,
	.type       = COFF_T_NULL,
	.sclass     = COFF_C_NULL
};

/*
 * create and fill functions should be separated since
 * section and symbol refere each otehr with their index number.
 * It means fill_scnlist() needs symbol instances as well as
 * fill_symlist() needs section instances.
 */
void section_createlist(struct list_head *list, u16 n)
{
	int i;

	for (i = 0; i < n; i++) {
		struct section *scn = dld_malloc(sizeof(struct section), "section");
		list_add_tail(&scn->list_head, list);
		scn->name = NULL;
		scn->size = 0;
		scn->data = NULL;
		scn->nreloc = 0;
		scn->reloc = NULL;
	}
}

void section_freelist(struct list_head *list)
{
	struct section *scn, *tmp;

#ifdef STICKY_LIST
	if (list == NULL)
		list = &g_scnlist;
#endif
	section_for_each_safe(scn, tmp, list) {
		list_del(&scn->list_head);
		if (scn->name)
			dld_free(scn->name, "section->name");
		/* data might be pointing dummy data */
		if ((scn->data) && (scn->data != dummy_section_data))
			dld_free(scn->data, "section->data");
		if (scn->reloc)
			dld_free(scn->reloc, "section->reloc");
		dld_free(scn, "section");
	}
}

#define reloc_fieldtype_sym(rel) \
	(rel_sym_field_type(rel->type) == RELOC_SYMBOL_FIELD_TYPE_SYMBOL)
#define reloc_fieldtype_val(rel) \
	(rel_sym_field_type(rel->type) == RELOC_SYMBOL_FIELD_TYPE_VALUE)

void section_filllist(struct coff *coff, u8 *src,
		      struct coffobj *cobj, u16 n, int type)
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
		scn->name = dld_strdup(scnname(coff, scnsrc->s_name), "section->name");
		scn->vaddr = COFF_LONG(scnsrc->s_vaddr);
		scn->vaddr_orig = scn->vaddr;
		scn->size = COFF_LONG(scnsrc->s_size);
		scn->nreloc = COFF_LONG(scnsrc->s_nreloc);
		scn->flags = COFF_LONG(scnsrc->s_flags);
		scn->cobj = (type == COFFTYP_KERNEL) ? NULL : cobj;
		scnptr = COFF_LONG(scnsrc->s_scnptr);
		relptr = COFF_LONG(scnsrc->s_relptr);

		if (scnptr) {
			scn->data = dld_malloc(scn->size, "section->data");
			memcpy(scn->data, &src[scnptr], scn->size);
		} else
			scn->data = NULL;

		if (relptr) {
			COFF_RELOC *srel = (COFF_RELOC *)&src[relptr];
			scn->reloc = dld_malloc(sizeof(struct reloc) * scn->nreloc, "section->reloc");
			for (j = 0; j < scn->nreloc; j++) {
				COFF_RELOC *relsrc = &srel[j];
				struct reloc *rel = &scn->reloc[j];
				u32 symndx;

				rel->vaddr = COFF_LONG(relsrc->r_vaddr);
				rel->exa   = COFF_SHORT(relsrc->r_exa);
				rel->type  = COFF_SHORT(relsrc->r_type);
				symndx     = COFF_LONG(relsrc->r_symndx);
				if (reloc_fieldtype_sym(rel)) {
					rel->sym.sym = (symndx == 0xffffffff) ?
						&symbol_internalreloc:
						list_entry(list_find_by_idx(&cobj->symlist, symndx), struct symbol, list_head);
				} else if (reloc_fieldtype_val(rel)) {
					rel->sym.val = symndx;
				}
			}
		} else
			scn->reloc = NULL;
	}
}

#ifdef STICKY_LIST
void section_register_global(struct list_head *list)
{
	struct section *scn;

	/*
	 * We don't need section contents anymore.
	 *
	 * We need section header (in fact, only flags) for relocation
	 * to detect the relocation address points text or data.
	 *
	 * And we also need to know whether the section used to have
	 * data, so we release the actual data here but
	 * make 'scn->data' point to dummy data.
	 */
	section_for_each(scn, list) {
		if (scn->data) {
			dld_free(scn->data, "section->data");
			scn->data = dummy_section_data;
		}

		if (scn->reloc)	{ /* should not have... */
			dld_free(scn->reloc, "section->reloc");
			scn->reloc = NULL;
		}
	}

	/*
	 * All sections are moved to g_scnlist
	 */
	list_splice(list, &g_scnlist);
	INIT_LIST_HEAD(list);
}
#endif /* STICKY_LIST */

int section_load(struct list_head *list, struct taskent *te)
{
	int fd;
	struct section *scn;
	char *msg;
	int ret = 0;

#ifndef DSP_EMULATION
	fd = open(DEVNAME_DSPMEM, O_RDWR);
	if (fd < 0) {
		prmsg("Can't open %s\n", DEVNAME_DSPMEM);
		return -1;
	}
#endif

#if defined(DEBUG_SECTION) || defined(DEBUG_LOAD)
	prmsg("section loading ...\n");
#endif
	section_for_each(scn, list) {
		switch (section_loadtype(scn)) {
			case LDTYP_CINIT_RAM:
				/* .cinit will be treated later */
				continue;
			case LDTYP_NOLOAD:
				msg = " ignored.";
				break;
			case LDTYP_ZEROSZ:
			case LDTYP_NODATA:
				msg = "";
				break;
			case LDTYP_LOAD:
			{
				unsigned char *data;
				int cnt;

				if ((data = scn->data) == NULL) {
					data = dld_malloc(scn->size, "section->data");
					memset(data, 0, scn->size);
				}
				cnt = load_to_dspmem(fd, te, scn->vaddr,
						     scn->size, data);
				if (scn->data == NULL)
					dld_free(data, "sectionsection->data");

				if (cnt == scn->size) {
					msg = " initialized.";
				} else {
					msg = " initialization failed!!";
					ret = -1;
				}
			}
		}

#if defined(DEBUG_SECTION) || defined(DEBUG_LOAD)
		prmsg("%-20s : adr=0x%06lx, size=%06lx, flags=0x%08lx %s\n",
		      scn->name, scn->vaddr, scn->size, scn->flags, msg);
#endif
		if (ret < 0) {
			prmsg("error in loading %s!!\n", scn->name);
			goto finish;
		}
	}

	/* RAM mode .cinit handling */
	section_for_each(scn, list) {
		if (section_loadtype(scn) != LDTYP_CINIT_RAM)
			continue;

		if (cinit_bss_init(fd, te, scn) < 0) {
			msg = " .bss variables initialization failed!!";
			ret = -1;
		} else {
			msg = " .bss variables are initialized.";
		}

#if defined(DEBUG_SECTION) || defined(DEBUG_LOAD)
		prmsg("%-20s : adr=0x%06lx, size=%06lx, flags=0x%08lx %s\n",
		      scn->name, scn->vaddr, scn->size, scn->flags, msg);
#endif
		if (ret < 0) {
			prmsg("error in loading %s!!\n", scn->name);
			goto finish;
		}
	}

finish:
#ifndef DSP_EMULATION
	close(fd);
#endif
	return ret;
}

static int load_to_dspmem(int fd, struct taskent *te, u32 addr,
			  u32 size, u8 *data)
{
#ifdef DSP_EMULATION

	struct list_head *memlist;
	struct memmgr *mem;
	u32 offset;

	memlist = (te && te->lkcmd) ? &te->lkcmd->memlist : NULL;
	mem = memmgr_find_by_addr(memlist, addr, size);
	if (mem == NULL)
		return 0;

	offset = addr - mem->base;
	memcpy(mem->img + offset, data, size);
	return size;

#else /* DSP_EMULATION */

	/*
	 * We assume MPUI byteswap off for DARAM/SARAM.
	 * You should be careful that the copy_from_user routine
	 * for 1,2 and 3-bytes is tricky through MPUI.
	 */

	size_t size1, size2, size3;
	u8 data_bs[size+8], *dst;
	size_t writesize;
	u32 writeaddr;
	u8 *writesrc;

	if (space_find_by_addr(addr, size) == SPACE_CROSSING) {
		prmsg("section crossing memory boundary!\n");
		return -1;
	}

	size1 = (addr & 0x3) ? 4 - (addr & 0x3) : 0;	/* top odd bytes */
	size3 = (addr + size) & 0x3;			/* bottom odd bytes */
	size2 = size - size1 - size3;			/* multiple of 4 */
#ifdef DEBUG_LOAD
	prmsg("\n");
	prmsg("addr = %08lx\n", addr);
	prmsg("size = %08lx, %x:%x:%x\n", size, size1, size2, size3);
#endif

	writesize = 0;

	if (size1) {
#ifdef DEBUG_LOAD
		prmsg("top: seek to %08lx\n", addr & ~0x3);
#endif
		lseek(fd, addr & ~0x3, SEEK_SET);
		dst = data_bs;
		if (read(fd, dst, 4) < 4)
			return -1;

#ifdef DEBUG_LOAD
		prmsg("%02x %02x %02x %02x -> ",
		      dst[0], dst[1], dst[2], dst[3]);
#endif
		if (size1 >= 3)
			dst[0] = *(data++);
		if (size1 >= 2)
			dst[3] = *(data++);
		/* if(size1 >= 1) */
			dst[2] = *(data++);
#ifdef DEBUG_LOAD
		prmsg("%02x %02x %02x %02x\n", dst[0], dst[1], dst[2], dst[3]);
#endif
	}

	dst = data_bs + 4;
	copy_byteswap(dst, data, size2);
	data += size2;
	dst  += size2;

	if (size3) {
#ifdef DEBUG_LOAD
		prmsg("bottom: seek to %08lx\n", addr + size1 + size2);
#endif
		lseek(fd, addr + size1 + size2, SEEK_SET);
		if (read(fd, dst, 4) < 4)
			return -1;

#ifdef DEBUG_LOAD
		prmsg("%02x %02x %02x %02x ->", dst[0], dst[1], dst[2], dst[3]);
#endif
		/* if (size3 >= 1) */
			dst[1] = *(data++);
		if (size3 >= 2)
			dst[0] = *(data++);
		if (size3 >= 3)
			dst[3] = *(data++);
#ifdef DEBUG_LOAD
		prmsg("%02x %02x %02x %02x\n", dst[0], dst[1], dst[2], dst[3]);
#endif
	}

	writeaddr = size1 ? addr & ~0x3 : addr;
	writesize = (size1 ? 4 : 0) + size2 + (size3 ? 4 : 0);
	writesrc  = size1 ? data_bs : data_bs+4;
#ifdef DEBUG_LOAD
	prmsg("writeaddr = %08lx, writesize = %08lx, src offset = %d\n",
	      writeaddr, writesize, writesrc - data_bs);
#endif
	lseek(fd, writeaddr, SEEK_SET);
	if (write(fd, writesrc, writesize) < writesize)
		return -1;

	return size;

#endif /* DSP_EMULATION */
}

#ifndef DSP_EMULATION
static void copy_byteswap(u8 *dst, u8 *src, size_t size)
{
	for (; size >= 2; size -= 2, src += 2, dst += 2) {
		dst[1] = src[0];
		dst[0] = src[1];
	}
}
#endif /* DSP_EMULATION */

static int cinit_bss_init(int fd, struct taskent *te, struct section *scn)
{
	int i = 0;

	while (i < scn->size) {
		COFF_CINIT *cinit = (COFF_CINIT *)&scn->data[i];
		/* size and addr are written in big-endian format */
		u16 size  = COFF_SHORT_H(cinit->size) * 2;	/* size in word */
		u32 addr  = COFF_24BIT_H(cinit->addr) * 2;	/* addr in word */
		u8 flags = *cinit->flags;
		int cnt;

#ifdef DEBUG_CINIT
{
		/*
		 * at this moment for kernel, g_scnlist is vacant...
		 * therefore, section_find_by_addr can not be used. ;-(
		 */
		int j;

		prmsg("\n  i=%d, size=%04x, addr=%06lx, flags=%02x\n",
		      i, size, addr, flags);

		if (size) {
			for (j = 0; j < size; j += 2) {
				prmsg("%02x: %02x %02x\n",
				      j, (u8)cinit->data[j], (u8)cinit->data[j+1]);
			}
		}
}
#endif
		/*
		 * last entry may be 0-size
		 */
		if (size == 0) {
			i += 2;
			if (i != scn->size) {
				prmsg("warning: found zero-sized "
				      "cinit entry at 0x%x\n", i-2);
			}
			continue;
		}

		if (te &&
		    (section_find_by_addr(&te->cobj->scnlist, addr) == NULL)) {
			prmsg("Warning: cinit target address %06lx "
			      "is not in this task. ignoring it.\n", addr);
			i += COFF_CINITSZ + size;
			continue;
		}

		if ((flags & 0x01) == COFF_CINIT_FLAG_IO) {
			prmsg("We can't handle cinit data for I/O space!\n");
			return -1;
		}
		cnt = load_to_dspmem(fd, te, addr, size, cinit->data);
		if (cnt < size)
			return -1;
		i += COFF_CINITSZ + size;
	}

	if (i > scn->size) {
		prmsg("cinit data size is inconsistent.\n");
		return -1;
	}

	return 0;
}

/*
 * Address relocator
 *
 * @list: Section list
 *
 * Calculate address according to relocation info for sections
 *
 */
int section_relocate(struct list_head *list)
{
	int i;
	struct section *scn;
	struct reloc *rel;
	u32 vaddr_relative;
	u32 delta, val;
#ifdef DEBUG_RELOCATE
	u32 sym_org, sym_new;
#endif
	u8 *dst;

	section_for_each(scn, list) {
#ifdef DEBUG_RELOCATE
		if (scn->nreloc) {
			prmsg("%s: reloc entries\n", scn->name);
			prmsg("%3s %8s %-20s %8s %8s   %-16s %8s  %8s\n",
			      "ent", "addr", "symbol", "sym:olg", "sym:new",
			      "type", "original", "new");
		}
#endif
		for (i = 0; i < scn->nreloc; i++) {
			/*
			 * XXX: ___pinit__ blames.
			 * we disable the value check at the moment.
			 */
#if 0
			int safe_relocation = 1;
#else
			int safe_relocation = 0;
#endif

			rel = &scn->reloc[i];
			if (rel->type & COFF_R_EXTRA_BIT) {
				if ((i = extra_relocate(scn, i)) < 0)
					return -1;
				continue;
			}

			if (!strcmp(rel->sym.sym->name, "cinit")) {
				/*
				 * If the module has been compiled with
				 * RAM model initialization, it should
				 * not refer to the "cinit" symbol, but
				 * otherwise there can be this reference.
				 * the kernel side, cinit is dummy and
				 * the value is illegal (maybe ffffffff).
				 *
				 * We allow the case the module has not
				 * been compiled with RAM model.
				 */
				safe_relocation = 0;
			}

			vaddr_relative = rel->vaddr - scn->vaddr_orig;
#ifdef DEBUG_RELOCATE
			delta = rel_sym_delta(scn, rel, &sym_org, &sym_new);
#else
			delta = rel_sym_delta(scn, rel);
#endif
			dst = &scn->data[vaddr_relative];

			switch (rel->type) {

			case COFF_R_ABS:	/* no relocation */
				break;

			case COFF_R_LD3_k4:	/* unsigned 4-bit shift immed. */
				val = (dst[0] >> 4) + delta;
				if (safe_relocation && (val > 0xf))
					goto val_invalid;
				dst[0] = (val << 4) & (dst[0] & 0xf);
				break;

			case COFF_R_RELBYTE:	/* 8-bit */
			case COFF_R_LD3_K8:	/* 8-bit signed direct */
				val = dst[0] + delta;
				if (safe_relocation &&
				    (val > 0x7f) && (val < 0xffffff80))
					goto val_invalid;
				dst[0] = val;
				break;

			case COFF_R_LD3_k8:	/* 8-bit unsigned direct */
				val = dst[0] + delta;
				if (safe_relocation && (val > 0xff))
					goto val_invalid;
				dst[0] = val;
				break;

			case COFF_R_RELWORD:	/* 16-bit */
			case COFF_R_LD3_K16:	/* 16-bit signed direct */
				val = COFF_SHORT_H(dst) + delta;
				if (safe_relocation &&
				    (val > 0x7fff) && (val < 0xffff8000))
					goto val_invalid;
				COFF_PUT_SHORT_H(val, dst);
				break;

			case COFF_R_LD3_REL16:	/* speculation */
			case COFF_R_LD3_k16:	/* 16-bit unsigned direct */
				val = COFF_SHORT_H(dst) + delta;
				if (safe_relocation && (val > 0xffff))
					goto val_invalid;
				COFF_PUT_SHORT_H(val, dst);
				break;

			case COFF_R_REL24:	/* 24-bit */
			case COFF_R_LD3_REL23:	/* 23-bit unsigned value in 24-bit field */
				val = COFF_24BIT_H(dst) + delta;
				if (safe_relocation && (val > 0xffffff))
					goto val_invalid;
				COFF_PUT_24BIT_H(val, dst);
				break;

			case COFF_R_RELLONG:
				val = COFF_LONG_H(dst) + delta;
				COFF_PUT_LONG_H(val, dst);
				break;

			/*
			 * FIXME:
			 * Not sure how those types shold be handled
			 */
			case COFF_R_LD3_DMA:	/* 7 MSBs in a byte (unsigned) */
			case COFF_R_LD3_MDP:	/* 7 bits spanning 2bytes (unsigned) */
			case COFF_R_LD3_PDP:	/* 9 bits spanning 2bytes (unsigned) */
			case COFF_R_LD3_l8:	/* 8-bit unsigned relative */
			case COFF_R_LD3_l16:	/* 16-bit unsigned relative */
			case COFF_R_LD3_L8:	/* 8-bit signed relative */
			case COFF_R_LD3_L16:	/* 16-bit signed relative */
			case COFF_R_LD3_k5:	/* unsigned 5-bit shift immed. */
			case COFF_R_LD3_K5:	/* signed 5-bit shift immed. */
			case COFF_R_LD3_k6:	/* unsigned 6-bit shift immed. */
			case COFF_R_LD3_k12:	/* unsigned 12-bit shift immed. */
			default:
				prmsg("We can't handle this type of "
				      "relocation [%04x]! "
				      "(section %s, entry %d)\n"
				      "Please contact to developer.\n",
				      rel->type, scn->name, i);
				return -1;
			}
#ifdef DEBUG_RELOCATE
			prmsg(" %2d %08lx %-20s %08lx %08lx"
			      "   %04x(%-10s) %08lx  %08lx\n",
			      i, vaddr_relative, rel->sym.sym->name,
			      sym_org, sym_new,
			      rel->type, reloc_name(rel->type),
			      val - delta, val);
#endif
		}
	}
	return 0;

val_invalid:
#ifdef DEBUG_RELOCATE
	prmsg(" %2d %08lx %-20s %08lx %08lx"
	      "   %04x(%-10s) %08lx  %08lx\n",
	      i, vaddr_relative, rel->sym.sym->name, sym_org, sym_new,
	      rel->type, reloc_name(rel->type), val - delta, val);
#endif
	prmsg("relocated value %x is invalid for relocation type %s!\n",
	      val, reloc_name(rel->type));
	return -1;
}

static int extra_relocate(struct section *scn, int i)
{
	u32 vaddr = scn->reloc[i].vaddr;
	u32 vaddr_relative = vaddr - scn->vaddr_orig;
#define STKSZ	10
	u32 stack[STKSZ];
	int sp = 0;

	for (; i < scn->nreloc; i++) {
		struct reloc *rel = &scn->reloc[i];

		if (rel->vaddr != vaddr)
			break;

		switch (rel->type) {
			case COFF_R_EX_PSHSYM:
				stack[sp++] = rel_sym_val(scn, rel);
				break;

			case COFF_R_EX_PSHVAL:
				stack[sp++] = rel->sym.val;
				break;

			case COFF_R_EX_PSHINTOFF:
				/*
				 * internal reloc + offset value
				 */
				stack[sp++] = scn->vaddr + rel->sym.val;
				break;

			case COFF_R_EX_ADD:
				stack[0] += stack[--sp];
				break;

			case COFF_R_EX_SUB:
				stack[0] -= stack[--sp];
				break;

			case COFF_R_EX_RSHIFT:
				stack[0] >>= stack[--sp];
				break;

			case COFF_R_EX_MASK:
				stack[0] &= stack[--sp];
				break;

			/*
			 * XXX:
			 * Couldn't find out the difference among
			 * three below.
			 */
			case COFF_R_EX_WRITE1:
			case COFF_R_EX_WRITE2:
			case COFF_R_EX_WRITE3:
			{
				u8 roff, loff, masksz;
				u32 msb_ptr;
				u32 parm, mask, offset;
				u8 *dst;

				parm = rel->sym.val;
				roff   = (parm >> 16) & 0xff;
				masksz = (parm >>  8) & 0xff;
				loff   =  parm        & 0xff;

				if ((masksz > 32) || (roff > 32) || (loff > 32) ||
				    (roff & 0x07)) {
					prmsg("illegal writeback parm [%08x]! "
					      "(section %s, entry %d)\n"
					      "Please contact to developer.\n",
					      parm, scn->name, i);
					return -1;
				}
				mask = 0xffffffff >> (32-masksz) << loff;
				msb_ptr = vaddr_relative + (roff-loff+7)/8 - 4;
				dst = &scn->data[msb_ptr];
				offset = COFF_LONG_H(dst);
				stack[0] = (offset & ~mask) |
					   ((stack[0] << loff) & mask);
				COFF_PUT_LONG_H(stack[0],dst);
				break;
			}

			default:
				prmsg("We can't handle this type of relocation [%04x]! "
				      "(section %s, entry %d)\n"
				      "Please contact to developer.\n",
				      rel->type, scn->name, i);

		}

#ifdef DEBUG_RELOCATE
		switch (rel->type) {
			case COFF_R_EX_PSHSYM:
				prmsg(" %2d %08lx %-20s %8s %08lx"
				      "   %04x(%-10s)\n",
				      i, vaddr_relative,
				      rel->sym.sym->name,
				      "", stack[sp-1],
				      rel->type, reloc_name(rel->type));
				break;
			case COFF_R_EX_PSHVAL:
			case COFF_R_EX_PSHINTOFF:
				prmsg(" %2d %08lx   %08lx%10s %8s %8s"
				      "   %04x(%-10s)\n",
				      i, vaddr_relative, rel->sym.val,
				      "", "", "",
				      rel->type, reloc_name(rel->type));
				break;
			case COFF_R_EX_ADD:
			case COFF_R_EX_SUB:
			case COFF_R_EX_RSHIFT:
			case COFF_R_EX_MASK:
				prmsg(" %2d %08lx %-20s %8s %8s"
				      "   %04x(%-10s) %8s (%08lx)\n",
				      i, vaddr_relative, "", "", "",
				      rel->type, reloc_name(rel->type),
				      "", stack[0]);
				break;

			case COFF_R_EX_WRITE1:
			case COFF_R_EX_WRITE2:
			case COFF_R_EX_WRITE3:
				prmsg(" %2d %08lx   %08lx%10s %8s %8s"
				      "   %04x(%-10s) %8s (%08lx)\n",
				      i, vaddr_relative, rel->sym.val,
				      "", "", "",
				      rel->type, reloc_name(rel->type),
				      "", stack[0]);
				break;
		}
#endif

	}
	return i-1;
}

inline int rel_sym_field_type(u16 type)
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

static char *reloc_name(u16 type)
{
	switch (type) {
		case COFF_R_ABS:		return("ABS");
		case COFF_R_REL24:		return("REL24");
		case COFF_R_RELBYTE:		return("RELBYTE");
		case COFF_R_RELWORD:		return("RELWORD");
		case COFF_R_RELLONG:		return("RELLONG");
		case COFF_R_LD3_DMA:		return("LD3_DMA");
		case COFF_R_LD3_MDP:		return("LD3_MDP");
		case COFF_R_LD3_PDP:		return("LD3_PDP");
		case COFF_R_LD3_REL23:		return("LD3_REL23");
		case COFF_R_LD3_k8:		return("LD3_k8");
		case COFF_R_LD3_k16:		return("LD3_k16");
		case COFF_R_LD3_K8:		return("LD3_K8");
		case COFF_R_LD3_K16:		return("LD3_K16");
		case COFF_R_LD3_l8:		return("LD3_l8");
		case COFF_R_LD3_l16:		return("LD3_l16");
		case COFF_R_LD3_L8:		return("LD3_L8");
		case COFF_R_LD3_L16:		return("LD3_L16");
		case COFF_R_LD3_k4:		return("LD3_k4");
		case COFF_R_LD3_k5:		return("LD3_k5");
		case COFF_R_LD3_K5:		return("LD3_K5");
		case COFF_R_LD3_k6:		return("LD3_k6");
		case COFF_R_LD3_k12:		return("LD3_k12");
		case COFF_R_LD3_REL16:		return("LD3_REL16");
		case COFF_R_EX_ADD:		return("EX_ADD");
		case COFF_R_EX_SUB:		return("EX_SUB");
		case COFF_R_EX_RSHIFT:		return("EX_RSHIFT");
		case COFF_R_EX_MASK:		return("EX_MASK");
		case COFF_R_EX_WRITE1:		return("EX_WRITE1");
		case COFF_R_EX_WRITE2:		return("EX_WRITE2");
		case COFF_R_EX_PSHSYM:		return("EX_PSHSYM");
		case COFF_R_EX_PSHVAL:		return("EX_PSHVAL");
		case COFF_R_EX_PSHINTOFF:	return("EX_PSHINTOFF");
		case COFF_R_EX_WRITE3:		return("EX_WRITE3");
	}
	return("unknown");
}

#ifdef DEBUG_RELOCATE
static u32 __rel_sym_val(struct section *scn, struct reloc *rel,
			 u32 *org, u32 *new, int delta)
#else
static u32 __rel_sym_val(struct section *scn, struct reloc *rel, int delta)
#endif
{
	struct symbol *sym;
	struct section *scn_target;
	u32 _org, _new, val;

	sym = rel->sym.sym;
	if (sym == &symbol_internalreloc) {
		_org = scn->vaddr_orig;
		_new = scn->vaddr;
		scn_target = scn;
	} else {
		_org = sym->value_orig;
		_new = sym->value;
		scn_target = sym->scn;
	}
	/*
	 * text pointer: byte address
	 * data pointer: word address
	 */
	if (!(scn_target->flags & COFF_STYP_TEXT)) {
		_org >>= 1;
		_new >>= 1;
	}
	val = delta ? _new - _org : _new;

#ifdef DEBUG_RELOCATE
	/* debug info return */
	if (org)
		*org = _org;
	if (new)
		*new = _new;
#endif

	return val;
}

static int section_loadtype(struct section *scn)
{
	if (scn->size == 0)
		return LDTYP_ZEROSZ;

	if (scn->data == NULL) {
		if (scn->flags & COFF_STYP_BSS)
			return LDTYP_LOAD;
		else
			return LDTYP_NODATA;
	}

	if (scn->flags & (COFF_STYP_DSECT | COFF_STYP_NOLOAD | COFF_STYP_PAD))
		return LDTYP_NOLOAD;

	if (scn->flags & COFF_STYP_COPY) {
		if (!strcmp(scn->name, ".cinit"))
			return LDTYP_CINIT_RAM;
		else
			return LDTYP_NOLOAD;
	}

	return LDTYP_LOAD;
}

static struct section *section_find_by_addr(struct list_head *list, u32 addr)
{
	struct section *scn;

	section_for_each(scn, list) {
		if ((addr >= scn->vaddr) &&
		    (addr < scn->vaddr + scn->size) &&
		    !(scn->flags & (COFF_STYP_DSECT |
				    COFF_STYP_NOLOAD |
				    COFF_STYP_PAD |
				    COFF_STYP_COPY)))
			return scn;
	}

	/* not found */
	return NULL;
}

struct section *section_find_by_name(struct list_head *list, char *name)
{
	struct section *scn;

	section_for_each(scn, list) {
		if (!strcmp(scn->name, name))
			return scn;
	}

	/* not found */
	return NULL;
}

void section_sendstat(struct list_head *list, int fd)
{
	char buf[256];
	size_t strsz = 256 - SERVER_EVENT_HDRSZ;
	struct server_event *e = (struct server_event *)buf;
	struct section *scn;
	int cnt;

#ifdef STICKY_LIST
	if (list == NULL)
		list = &g_scnlist;
#endif

	e->event = DLD_EVENT_STRING;

	cnt = snprintf(e->data.s, strsz, "%-20s %8s  %8s  %4s %10s\n",
		       "name", "vaddr", "size", "nrel", "flags");
	e->len = SERVER_EVENT_HDRSZ + cnt;
	write(fd, e, e->len);

	section_for_each(scn, list) {
		int real_addr;

		real_addr = (scn->flags & (COFF_STYP_DSECT |
					   COFF_STYP_NOLOAD |
					   COFF_STYP_PAD |
					   COFF_STYP_COPY)) ? 0 : 1;
		cnt = snprintf(e->data.s, strsz,
			       "%-20s 0x%06lx%c 0x%06lx%c %4ld 0x%08lx\n",
			       scn->name,
			       scn->vaddr, real_addr ? ' ' : '-',
			       scn->size,  scn->data ? ' ' : '-',
			       scn->nreloc, scn->flags);
		e->len = SERVER_EVENT_HDRSZ + cnt;
		write(fd, e, e->len);
	}
}

/*
 * debug stuff
 */
void section_printstat(struct list_head *list)
{
	struct section *scn;

	prmsg("section list status ...\n");
	prmsg("  %-20s %8s  %8s  %4s %10s\n",
	      "name", "vaddr", "size", "nrel", "flags");
	section_for_each(scn, list) {
		int real_addr;

		real_addr = (scn->flags & (COFF_STYP_DSECT |
					   COFF_STYP_NOLOAD |
					   COFF_STYP_PAD |
					   COFF_STYP_COPY)) ? 0 : 1;
		prmsg("  %-20s 0x%06lx%c 0x%06lx%c %4ld 0x%08lx\n",
		      scn->name,
		      scn->vaddr, real_addr ? ' ' : '-',
		      scn->size,  scn->data ? ' ' : '-',
		      scn->nreloc,
		      scn->flags);
	}
}
