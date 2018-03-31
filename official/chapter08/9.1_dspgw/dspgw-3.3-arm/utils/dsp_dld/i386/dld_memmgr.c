/*
 * dsp_dld/arm/dld_memmgr.c
 *
 * DSP Dynamic Loader Daemon: dld_memmgr.c
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
 * 2005/07/07:  DSP Gateway version 3.3
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include "list.h"
#include "dsp_dld.h"
#include "dld_malloc.h"
#include "dld_daemon.h"
#include "dld_cmd.h"
#include "dld_taskent.h"
#include <asm/arch/dsp.h>

#define PLACE_MODE_BASENAME	0x1
#define PLACE_MODE_EXACTNAME	0x0
#define PLACE_MODE_ANYWHERE	0x2
#define place_mode_basename(mode)	(mode & PLACE_MODE_BASENAME)
#define place_mode_exactname(mode)	(!(mode & PLACE_MODE_BASENAME))
#define place_mode_anywhere(mode)	(mode & PLACE_MODE_ANYWHERE)

#define DSPADDR_ILLEGAL	0xffffffff

#define ceil_int(n,align)	(((n)+(align)-1)&~((align)-1))
#define floor_int(n,align)	((n)&~((align)-1))

static LIST_HEAD(g_memlist);

static struct memmgr *memmgr_default(void);
static u32 memmgr_malloc(struct memmgr *mem, struct taskent *te,
			 u32 size, u32 align);
static void memmgr_freeseg(struct memseg *seg);
int memmgr_exmap(struct list_head *list);
struct memmgr *memmgr_range_user(struct list_head *list, u32 base, u32 size);
int space_find_by_addr(u32 addr, u32 size);
static struct memmgr *placescn(struct taskent *te, struct section *scn,
			       struct lkcmd *lkcmd, unsigned int mode);
static struct memseg *memseg_new(u32 base, u32 size);
static void memseg_freelist(struct list_head *list);

/*
 * default directive
 */
static struct directive directive_default = {
	.list_head = LIST_HEAD_INIT(directive_default.list_head),
	.scnnm = "default",
	.iscnlist = LIST_HEAD_INIT(directive_default.iscnlist),
	.load = {
		.stype = 0,
		.align = 0,
		.block = 0,
		.fill = 0,
		.mem = NULL,
		.addr = 0
	},
	.run = {
		.stype = 0,
		.align = 0,
		.block = 0,
		.fill = 0,
		.mem = NULL,
		.addr = 0
	},
};

/*
 * default memmgr
 */
static struct memmgr *memmgr_default(void)
{
	/* first element of memmgr list */
	return list_entry(g_memlist.next, struct memmgr, list_head);
}

static u32 memmgr_malloc(struct memmgr *mem, struct taskent *te,
			 u32 size, u32 align)
{
	struct memseg *seg;

	if (size == 0)
		return mem->base;

	memseg_for_each(seg, &mem->seglist) {
		u32 allocbase, allocend;

		allocbase = ceil_int(seg->wp, align);
		allocend = allocbase + size;

		if (allocend <= seg->end) {
			if (seg->cobj == te->cobj) {
				seg->wp = allocend;
			} else {
				struct memseg *newseg = dld_malloc(sizeof(struct memseg), "memseg");
				newseg->base = allocbase;
				newseg->wp   = allocend;
				newseg->end  = seg->end;
				newseg->cobj = te->cobj;
				seg->end = allocbase;
				list_add(&newseg->list_head, &seg->list_head);
			}
			return allocbase;
		}
	}
	return DSPADDR_ILLEGAL;
}

static void memmgr_freeseg(struct memseg *seg)
{
	struct memseg *prev;

	prev = list_entry(seg->list_head.prev, struct memseg, list_head);
	prev->end = seg->end;
	list_del(&seg->list_head);
	dld_free(seg, "memseg");
}

struct memmgr *memmgr_new(char *name, u32 base, u32 size)
{
	struct memmgr *mem;
	struct memseg *seg;

	mem = dld_malloc(sizeof(struct memmgr), "memmgr");
	if (mem == NULL)
		return NULL;
	mem->name = dld_strdup(name, "memmgr->name");
	if (mem->name == NULL)
		goto fail;
	mem->base = base;
	mem->size = size;
#ifdef DSP_EMULATION
	mem->img = dld_malloc(mem->size, "memmgr->img");
	if (mem->img == NULL)
		goto fail;
#else /* DSP_EMULATION */
	mem->exmap_size = 0;	/* set later */
#endif /* DSP_EMULATION */
	mem->seg_base = base;
	INIT_LIST_HEAD(&mem->seglist);
	seg = memseg_new(base, size);
	if (seg == NULL)
		goto fail;
	list_add_tail(&seg->list_head, &mem->seglist);
	mem->lkcmd = NULL;
	return mem;

fail:
	if (mem->name)
		dld_free(mem->name, "memmgr->name");
#ifdef DSP_EMULATION
	if (mem->img)
		dld_free(mem->name, "memmgr->img");
#endif
	if (mem)
		dld_free(mem, "memmgr");
	if (seg)
		dld_free(seg, "memseg");
	return NULL;
}

int memmgr_validate(struct memmgr *mem)
{
	struct memmgr *mem_user;
	char *name = mem->name;
	u32 base = mem->base;
	u32 size = mem->size;
	int space = space_find_by_addr(base, size);

	if ((((mem_user = memmgr_range_user(NULL, base, size))) != NULL) ||
	    (((mem_user = taskent_mem_range_user(base, size))) != NULL)) {
		prmsg("MEMORY segment %s has overlap with "
		      "previously defined one:\n"
		      "  new seg  -- %s: base = 0x%06lx, size = 0x%lx\n"
		      "  old user -- %s (@ %s): base = 0x%06lx, size = 0x%lx\n",
		      name, name, base, size,
		      mem_user->name,
		      mem_user->lkcmd ? mem_user->lkcmd->fn :
					"base command file",
		      mem_user->base, mem_user->size);
		return 0;
	}

	if (space == SPACE_CROSSING) {
		prmsg("MEMORY segment %s is crossing "
		      "memory boundary of DSP hardware!\n"
		      "base = 0x%06lx, size = 0x%lx\n", name, base, size);
		return 0;
	}

	return 1;
}

/*
 * if list == NULL in following handlers, it is for g_memlist.
 */
void memmgr_freelist(struct list_head *list)
{
	struct memmgr *mem, *tmp;

	if (list == NULL)
		list = &g_memlist;
	memmgr_for_each_safe(mem, tmp, list) {
		list_del(&mem->list_head);
#ifdef DSP_EMULATION
		if (mem->img)
			dld_free(mem->img, "memmgr->img");
#else /* DSP_EMULATION */
		if (mem->exmap_size > 0) {
			int fd;

			if ((fd = open(DEVNAME_DSPMEM, O_RDWR)) < 0) {
				prmsg("%s open failed at %s line %d\n",
				      DEVNAME_CONTROL, __FILE__, __LINE__);
				goto unmap_done;
			}
			prmsg("releasing external memory map: adr = 0x%06lx\n",
			      mem->base);
			ioctl(fd, OMAP_DSP_MEM_IOCTL_EXUNMAP, mem->base);
			close(fd);
		}
unmap_done:
#endif /* DSP_EMULATION */
		if (mem->name)
			dld_free(mem->name, "memmgr->name");
		memseg_freelist(&mem->seglist);
		dld_free(mem, "memmgr");
	}
}

void memmgr_add(struct list_head *list, struct memmgr *mem)
{
	if (list == NULL)
		list = &g_memlist;
	list_add_tail(&mem->list_head, list);
}

int memmgr_register_global(struct list_head *list)
{
	struct memmgr *mem, *tmp;

	if (!list_empty(&g_memlist)) {
		prmsg("g_memlist is not empty at %s line %d\n",
		      __FILE__, __LINE__);
	}

	memmgr_for_each_safe(mem, tmp, list) {
		if (!memmgr_validate(mem))
			return -1;
		mem->lkcmd = NULL;
		list_del(&mem->list_head);
		list_add_tail(&mem->list_head, &g_memlist);
	}
#ifndef DSP_EMULATION
	memmgr_exmap(&g_memlist);
#endif

	return 0;
}

#ifndef DSP_EMULATION

#define need_exmap(mem) \
	(((mem)->exmap_size > 0) && (strcmp((mem)->name, "FRAMEBUFFER")))

int memmgr_exmap(struct list_head *list)
{
	struct memmgr *mem1, *mem2;
	int ret = 0;
	int fd;

	if (list == NULL)
		list = &g_memlist;

	if ((fd = open(DEVNAME_DSPMEM, O_RDWR)) < 0) {
		prmsg("%s open failed at %s line %d\n",
		      DEVNAME_CONTROL, __FILE__, __LINE__);
		return -1;
	}

	memmgr_for_each(mem1, list) {
		if (space_find_by_addr(mem1->base, mem1->size) == SPACE_EXTERN)
			mem1->exmap_size = mem1->size;
	}

	/*
	 * join consecutive memory sections for one exmap.
	 */
	memmgr_for_each(mem1, list) {
		if (!need_exmap(mem1))
			continue;

		memmgr_for_each(mem2, list) {
			if (!need_exmap(mem2))
				continue;

			if (mem1->base + mem1->exmap_size == mem2->base) {
				/* join */
				mem1->exmap_size += mem2->size;
				mem2->exmap_size = 0;
			}
		}
	}

	memmgr_for_each(mem1, list) {
		if (mem1->exmap_size == 0)
			continue;

		if (!strcmp(mem1->name, "FRAMEBUFFER")) {
			/*
			 * export framebuffer
			 */
			u32 base = mem1->base;

			prmsg("exporting framebuffer to 0x%06lx\n", base);
			if (ioctl(fd, OMAP_DSP_MEM_IOCTL_FBEXPORT, &base) < 0) {
				prmsg("FBEXPORT failed\n");
				ret = -1;
				goto out;
			}
			prmsg("  (actual address = 0x%06lx)\n", base);
		} else {
			/*
			 * allocate new memory
			 */
			struct omap_dsp_mapinfo mapinfo = { mem1->base, mem1->exmap_size };

			prmsg("mapping external memory: "
			      "adr = 0x%06lx, size = 0x%lx\n",
			      mem1->base, mem1->exmap_size);
			if (ioctl(fd, OMAP_DSP_MEM_IOCTL_EXMAP, &mapinfo) < 0) {
				prmsg("EXMAP failed\n");
				ret = -1;
				goto out;
			}
		}
	}

out:
	close(fd);
	return ret;
}
#endif /* !DSP_EMULATION */

struct memmgr *memmgr_range_user(struct list_head *list, u32 base, u32 size)
{
	struct memmgr *mem;

	if (list == NULL)
		list = &g_memlist;
	memmgr_for_each(mem, list) {
		if ((base < mem->base + mem->size) &&
		    (base + size > mem->base))
			/* overlapping */
			return mem;
	}

	/* no overlap */
	return NULL;
}

struct memmgr *memmgr_find_by_addr(struct list_head *list, u32 base, u32 size)
{
	struct memmgr *mem;

	/*
	 * search in local list
	 */
	if (list && (list != &g_memlist)) {
		memmgr_for_each(mem, list) {
			if ((base >= mem->base) &&
			    (base + size <= mem->base + mem->size))
				return mem;	/* found! */
		}
	}

	/*
	 * search in global list
	 */
	memmgr_for_each(mem, &g_memlist) {
		if ((base >= mem->base) &&
		    (base + size <= mem->base + mem->size))
			return mem;	/* found! */
	}

	/* not found */
	return NULL;
}

struct memmgr *memmgr_find_by_name(struct list_head *list, char *name)
{
	struct memmgr *mem;

	/*
	 * search in local list
	 */
	if (list && (list != &g_memlist)) {
		memmgr_for_each(mem, list) {
			if (!strcmp(mem->name, name))
				return mem;	/* found! */
		}
	}

	/*
	 * search in global list
	 */
	memmgr_for_each(mem, &g_memlist) {
		if (!strcmp(mem->name, name))
			return mem;	/* found! */
	}

	/* not found */
	return NULL;
}

/*
 * memory space
 */
#define spaceof(a) \
	(((a) < DSP_MEM_SIZE) ? SPACE_INTERNAL : \
	 ((a) < VECTPG_BASE) ? SPACE_EXTERN : \
	 SPACE_VECTPG)

int space_find_by_addr(u32 addr, u32 size)
{
	int space = spaceof(addr);

	if (spaceof(addr + size - 1) == space)
		return space;
	else
		return SPACE_CROSSING;
}

void memmgr_occupy_kernel(struct list_head *scnlist)
{
	/*
	 * use only g_memlist
	 */
	struct section *scn;
	struct memmgr *mem;

	/*
	 * search the highest address of the area allocated
	 * for kernel sections
	 */
	section_for_each(scn, scnlist) {
		u32 scnend;

		if (scn->size == 0)
			continue;

		mem = memmgr_find_by_addr(NULL, scn->vaddr, scn->size);
		if (mem == NULL)
			continue;

		scnend = scn->vaddr + scn->size;
		if (scnend > mem->seg_base)
			mem->seg_base = scnend;
	}

	memmgr_for_each(mem, &g_memlist) {
		struct memseg *seg;

#if 0
		u32 a;
		/*
		 * decide segment align according to the memory size
		 */
		mem->seg_align = 1;
		a = mem->base + mem->size - mem->seg_base;
		while (a >>= 1) {
			mem->seg_align <<= 1;
		}
		mem->seg_align >>= 4;	/* max 32 segments */
		if (mem->seg_align == 0)
			mem->seg_align = 1;

		mem->seg_base = ceil_int(mem->seg_base, mem->seg_align);
#endif

		seg = list_entry(mem->seglist.next, struct memseg, list_head);
		seg->base = mem->seg_base;
		seg->wp   = mem->seg_base;
		seg->end  = mem->base + mem->size;
	}
}

int memmgr_placetask(struct taskent *te, struct coffobj *cobj,
		     struct lkcmd *gbl_lkcmd)
{
	struct section *scn;
	struct symbol *sym;

	section_for_each(scn, &cobj->scnlist) {
#if 0	/* allow non-zero section address */
		if (scn->vaddr != 0) {
			prmsg("section %s: "
			      "attempted to place the section "
			      "which has address value (0x%08lx)???\n",
			      scn->name, scn->vaddr);
			break;
		}
#endif

		/*
		 * RAM model initialization
		 * Do not allocate real memory to .cinit.
		 */
		if (!strcmp(scn->name, ".cinit")) {
			scn->flags |= COFF_STYP_COPY;
			continue;
		}

		/* search in task specific directive */
		if (te->lkcmd &&
		    (placescn(te, scn, te->lkcmd,
			      PLACE_MODE_EXACTNAME) != NULL))
			goto place_done;

		/* search in global directive */
		if (placescn(te, scn, gbl_lkcmd,
			     PLACE_MODE_EXACTNAME) != NULL)
			goto place_done;

		/*
		 * if section name contains ':' (ex. .text:aaa),
		 * try with its basename.
		 */
		if (strchr(scn->name, ':')) {
			/* search in task specific directive */
			if (te->lkcmd &&
			    (placescn(te, scn, te->lkcmd,
				      PLACE_MODE_BASENAME) != NULL))
				goto place_done;

			/* search in global directive */
			if (placescn(te, scn, gbl_lkcmd,
				     PLACE_MODE_BASENAME) != NULL)
				goto place_done;
		}

		/* place anywhere */
		prmsg("section %s: could not find directive. ", scn->name);
		if (placescn(te, scn, NULL, PLACE_MODE_ANYWHERE) != NULL)
			goto place_done;

		prmsg("section %s: failed to place!\n", scn->name);
		return -1;

place_done:
		/* update all symbol values in this section */
		symbol_for_each(sym, &cobj->symlist) {
			if (sym->scn == scn) {
				sym->value += scn->vaddr - scn->vaddr_orig;
			}
		}
	}

	return 0;
}

static struct memmgr *placescn(struct taskent *te, struct section *scn,
			       struct lkcmd *lkcmd, unsigned int mode)
{
	struct directive *dir;
	struct memmgr *mem;
	struct list_head *dirlist;
	struct list_head *memlist;
	u32 align;

	if (place_mode_anywhere(mode)) {
		dir = &directive_default;
		mem = memmgr_default();
		prmsg("placing it to default memory (%s).\n", mem->name);
		goto do_place;
	}

	/* can be null only when STICKY_LIST */
	dirlist = lkcmd ? &lkcmd->dirlist : NULL;
	if (place_mode_basename(mode)) {
		/*
		 * if this function called with basename mode,
		 * the section name must contain ':'.
		 */
		char *tmpname;

		tmpname = strdup(scn->name);
		*strchr(tmpname, ':') = '\0';
		dir = directive_find_by_scnnm(dirlist, tmpname);
		free(tmpname);
	} else
		dir = directive_find_by_scnnm(dirlist, scn->name);

	if (dir == NULL)
		return NULL;

	/*
	 * FIXME: dir can have constant addr!
	 */
	if (dir->load.mem == NULL) {
		prmsg("section %s: "
		      "could not get memory property for "
		      "directive...\n", scn->name);
		return NULL;
	}
	/* can be null only when STICKY_LIST */
	memlist = lkcmd ? &lkcmd->memlist : NULL;
	mem = memmgr_find_by_name(memlist, dir->load.mem);

do_place:
	if ((align = dir->load.align) == 0) {
		/* use default align */
		if (scn->flags & COFF_STYP_BSS)
			align = 4;	/* bss section (XXX: Is this true?) */
		else if (scn->flags & COFF_STYP_TEXT)
			align = 1;	/* text section */
		else
#if 0
			align = 2;	/* data section */
#else
			align = 4;	/* data section // XXX: for secure ... Is it needed? */
#endif
	}

	scn->vaddr = memmgr_malloc(mem, te, scn->size, align);
	if (scn->vaddr == DSPADDR_ILLEGAL) {
		prmsg("section %s: size = 0x%06x, align = 0x%x ... "
		      "no space left in %s!\n",
		      scn->name, scn->size, align, mem->name);
		return NULL;
	}
	return mem;
}

void memmgr_cleartask(struct taskent *te)
{
	struct memmgr *mem;
	struct memseg *seg, *tmp;

	memmgr_for_each(mem, &g_memlist) {
		memseg_for_each_safe(seg, tmp, &mem->seglist) {
			if (seg->cobj == te->cobj)
				memmgr_freeseg(seg);
		}
	}
}

void memmgr_sendstat(struct list_head *list, int fd)
{
	struct memmgr *mem;
	struct memseg *seg;
	char buf[256];
	size_t strsz = 256 - SERVER_EVENT_HDRSZ;
	struct server_event *e = (struct server_event *)buf;
	int cnt;

	if (list == NULL)
		list = &g_memlist;

	e->event = DLD_EVENT_STRING;

	cnt = snprintf(e->data.s, strsz,
		       "%-10s %-6s %-6s %-6s  %-6s %-6s %-6s %-20s\n",
		       "name", "base", "size", "sgbase",
		       "sgbase", "segwp", "segend", "task");
	e->len = SERVER_EVENT_HDRSZ + cnt;
	write(fd, e, e->len);

	memmgr_for_each(mem, list) {
		cnt = snprintf(e->data.s, strsz,
			       "%-10s %06lx %06lx %06lx\n",
			       mem->name, mem->base, mem->size, mem->seg_base);
		e->len = SERVER_EVENT_HDRSZ + cnt;
		write(fd, e, e->len);

		memseg_for_each(seg, &mem->seglist) {
			cnt = snprintf(e->data.s, strsz,
				       "%10s %6s %6s %6s  %06lx %06lx %06lx %-20s\n",
				       "", "", "", "",
				       seg->base, seg->wp, seg->end,
				       seg->cobj ? seg->cobj->fn : "(none)");
			e->len = SERVER_EVENT_HDRSZ + cnt;
			write(fd, e, e->len);
		}
	}
}

/*
 * debug stuff
 */
void memmgr_printstat(struct list_head *list)
{
	struct memmgr *mem;
	struct memseg *seg;

	if (list == NULL)
		list = &g_memlist;
	prmsg("memmgr list status...\n");
	prmsg("  %-10s %-6s %-6s %-6s  %-6s %-6s %-6s %-20s\n",
	      "name", "base", "size", "sgbase",
	      "sgbase", "segwp", "segend", "task");
	memmgr_for_each(mem, list) {
		prmsg("  %-10s %06lx %06lx %06lx\n",
		      mem->name, mem->base, mem->size, mem->seg_base);

		memseg_for_each(seg, &mem->seglist) {
			prmsg("  %10s %6s %6s %6s  %06lx %06lx %06lx %-20s\n",
			      "", "", "", "",
			      seg->base, seg->wp, seg->end,
			      seg->cobj ? seg->cobj->fn : "(none)");
		}
	}
}

/*
 * memseg
 */
static struct memseg *memseg_new(u32 base, u32 size)
{
	struct memseg *seg = dld_malloc(sizeof(struct memseg), "memseg");

	if (seg == NULL)
		return NULL;
	seg->base = base;
	seg->end  = base + size;
	seg->wp   = base;
	seg->cobj = NULL;
	return seg;
}

static void memseg_freelist(struct list_head *list)
{
	struct memseg *seg, *tmp;

	memseg_for_each_safe(seg, tmp, list) {
		list_del(&seg->list_head);
		dld_free(seg, "memseg");
	}
}
