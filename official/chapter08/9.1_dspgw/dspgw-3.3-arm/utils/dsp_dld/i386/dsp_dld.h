/*
 * dsp_dld/arm/dsp_dld.h
 *
 * DSP Dynamic Loader Daemon: dsp_dld.h
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
 * 2005/07/06:  DSP Gateway version 3.3
 */

#ifndef __DSP_DLD_H
#define __DSP_DLD_H

/*
 * Debug configuration
 */
#ifdef DEBUG_ALL
#define DEBUG_BOOT
#define DEBUG_MEMMGR
#define DEBUG_SECTION
#define DEBUG_SYMBOL
#define DEBUG_LOAD
#define DEBUG_CINIT
#define DEBUG_RESOLVE
#define DEBUG_PLACEMENT
#define DEBUG_RELOCATE
#endif

#include "coff-c55x.h"
#include "list.h"

#define SOCK_NAME	"/tmp/sock_dsp_dld"
#define DEVNAME_DSPMEM	"/dev/dspctl/mem"
#define DEVNAME_CONTROL	"/dev/dspctl/ctl"
#define DEVNAME_TWCH	"/dev/dspctl/twch"
#define DEVNAME_ERR	"/dev/dspctl/err"
#define TASKDEV_DIR	"/dev/dsptask"

typedef unsigned long u32;
typedef unsigned short u16;
typedef unsigned char u8;

#define DSP_MEM_SIZE	0x028000	/* 1510, 1610 */
#define VECTPG_BASE	0xfff000

#define SPACE_INTERNAL	1
#define SPACE_VECTPG	2
#define SPACE_EXTERN	3
#define SPACE_CROSSING	-2

#define COFFTYP_KERNEL	1
#define COFFTYP_TASK	2

typedef enum {
	SERVER_OK = 0,
	SERVER_DONE,
	SERVER_ERROR,
	SERVER_RESTART,
} server_return_t;

/*
 * configurations
 */
struct dld_conf {
	char *cfgfn;
	char *knlfn;
	char *cmdfn;
};

/*
 * directive
 */
#define DIRSCN_STYPE_DSECT	0x1
#define DIRSCN_STYPE_COPY	0x2
#define DIRSCN_STYPE_NOLOAD	0x4

struct dir_scn {
	u32 stype;
	u32 align;
	u32 block;
	u32 fill;
	char *mem;
	u32 addr;
};

enum iscn_type {
	IT_ISCN,
	IT_HOLE,
	IT_EXPR
};

struct iscn_entry {
	char *fn;
	char *scnnm;
};

enum hole_operator {
	HL_EQ,
	HL_PLUSEQ,
	HL_EQ_ALIGN,
	HL_EQPLUS_ALIGN
};

struct hole_entry {
	enum hole_operator op;
	u32 val;
};

struct dir_iscn {
	struct list_head list_head;
	enum iscn_type type;
	union {
		struct iscn_entry iscn;
		struct hole_entry hole;
		struct list_head expr;
	} u;
};

#define iscn_for_each(pos, head) \
	list_for_each_entry(pos, head, list_head)
#define iscn_for_each_safe(pos, n, head) \
	list_for_each_entry_safe(pos, n, head, list_head)

struct directive {
	struct list_head list_head;
	char *scnnm;
	struct list_head iscnlist;
	struct dir_scn load;
	struct dir_scn run;
};

#define directive_for_each(pos, head) \
	list_for_each_entry(pos, head, list_head)
#define directive_for_each_safe(pos, n, head) \
	list_for_each_entry_safe(pos, n, head, list_head)

enum expr_type {
	ET_VAR,
	ET_CONSTANT,
	ET_EQ,
	ET_PLUS,
	ET_MINUS,
	ET_MULT,
	ET_DIV,
	ET_PLUSEQ,
	ET_MINUSEQ,
	ET_MULTEQ,
	ET_DIVEQ
};

struct expr_tree {
	struct list_head list_head;
	enum expr_type type;
	unsigned long val;
	char *name;
};

#define exprtree_for_each(pos, head) \
	list_for_each_entry(pos, head, list_head)
#define exprtree_for_each_safe(pos, n, head) \
	list_for_each_entry_safe(pos, n, head, list_head)

struct globexpr {
	struct list_head list_head;
	struct list_head expr;
};

#define globexpr_for_each(pos, head) \
	list_for_each_entry(pos, head, list_head)
#define globexpr_for_each_safe(pos, n, head) \
	list_for_each_entry_safe(pos, n, head, list_head)

struct lopt {
	struct list_head list_head;
	char *opt;
};

#define lopt_for_each(pos, head) \
	list_for_each_entry(pos, head, list_head)
#define lopt_for_each_safe(pos, n, head) \
	list_for_each_entry_safe(pos, n, head, list_head)

struct lkcmd {
	char *fn;
	struct list_head memlist;
	struct list_head dirlist;
	struct list_head exprlist;
	struct list_head loptlist;
};

/*
 *
 */
struct coff {
	COFF_FILHDR *coffhdr;
	COFF_SCNHDR *scnhdr;
	COFF_SYMENT *symtbl;
	char *strtbl;
};

struct coffobj {
	char *fn;
	unsigned int taskcount;
	unsigned int usecount;
	u32 entry;
	struct list_head scnlist;
	struct list_head symlist;
#ifdef USE_FORK
	int request_lock;
#endif
};

enum task_request {
	TREQ_NONE = 0,
	TREQ_ADD,
	TREQ_DEL,
};

struct taskent {
	struct list_head list_head;
	char *devname;
	char *taskname;
	unsigned int pri;
	struct coffobj *cobj;
	struct lkcmd *lkcmd;
	int minor;
	enum task_request request;
};

#define taskent_for_each(pos, head) \
	list_for_each_entry(pos, head, list_head)
#define taskent_for_each_safe(pos, n, head) \
	list_for_each_entry_safe(pos, n, head, list_head)

struct memmgr {
	struct list_head list_head;
	char *name;
	u32 base;
	u32 size;
#ifdef DSP_EMULATION
	u8 *img;
#else
	u32 exmap_size;
#endif
	u32 seg_base;
	struct list_head seglist;
	struct lkcmd *lkcmd;
};

#define memmgr_for_each(pos, head) \
	list_for_each_entry(pos, head, list_head)
#define memmgr_for_each_safe(pos, n, head) \
	list_for_each_entry_safe(pos, n, head, list_head)

struct memseg {
	struct list_head list_head;
	u32 base;
	u32 end;
	u32 wp;
/*
	struct taskent *task;
*/
	struct coffobj *cobj;
};

#define memseg_for_each(pos, head) \
	list_for_each_entry(pos, head, list_head)
#define memseg_for_each_safe(pos, n, head) \
	list_for_each_entry_safe(pos, n, head, list_head)

/*
 *
 */
struct symbol {
	struct list_head list_head;
	char *name;
	u32 value_orig;		/* original value */
	u32 value;		/* relocated value */
	struct section *scn;
	u16 type;
	u8 sclass;
};

#define symbol_for_each(pos, head) \
	list_for_each_entry(pos, head, list_head)
#define symbol_for_each_safe(pos, n, head) \
	list_for_each_entry_safe(pos, n, head, list_head)

struct reloc {
	u32 vaddr;
	union {
		struct symbol *sym;
		u32 val;
	} sym;
	u16 exa;
	u16 type;
};

struct section {
	struct list_head list_head;
	char *name;
	u32 vaddr_orig;		/* original address */
	u32 vaddr;		/* relocated address */
	u32 size;
	u8 *data;
	u32 nreloc;
	struct reloc *reloc;
	u32 flags;
	struct coffobj *cobj;
};

#define section_for_each(pos, head) \
	list_for_each_entry(pos, head, list_head)
#define section_for_each_safe(pos, n, head) \
	list_for_each_entry_safe(pos, n, head, list_head)

/*
 * client-server protocol
 */
struct server_event {
	u32 len;
	u32 event;
	union {
		struct {	/* TADD/TDEL/TKILL event */
			u8 minor;
		} task;
		struct {	/* memory dump request event */
			u32 addr;
			u32 size;
		} memdump;
		struct {	/* reset vector set event */
			u32 addr;
		} rstvect;
		struct {	/* exmap event */
			u32 dspadr;
			u32 size;
		} exmap;
		struct {	/* mbsend event */
			u16 cmd;
			u16 data;
		} mbsend;
		char s[0]; /* string */
	} data;
};
#define SERVER_EVENT_HDRSZ	8

#define DLD_EVENT_NONE			0
#define DLD_EVENT_DONE			1
#define DLD_EVENT_ERROR			2
#define DLD_EVENT_STRING		10
#define DLD_EVENT_TADD			20
#define DLD_EVENT_TADD_DONE		21
#define DLD_EVENT_TDEL			22
#define DLD_EVENT_TDEL_DONE		23
#define DLD_EVENT_TKILL			24
#define DLD_EVENT_TKILL_DONE		25
#define DLD_EVENT_DSP_RESET		30
#define DLD_EVENT_DSP_RUN		31
#define DLD_EVENT_RSTVECT		32
#define DLD_EVENT_CPU_IDLE		33
#define DLD_EVENT_DSPCFG		34
#define DLD_EVENT_DSPUNCFG		35
#define DLD_EVENT_SUSPEND		36
#define DLD_EVENT_RESUME		37
#define DLD_EVENT_EXMAP			38
#define DLD_EVENT_MMUINIT		39
#define DLD_EVENT_MAPFLUSH		40
#define DLD_EVENT_MBSEND		41
#define DLD_EVENT_GETSTAT_TASKENT	50
#define DLD_EVENT_GETSTAT_MEMMGR	51
#define DLD_EVENT_GETSTAT_SYMBOL	52
#define DLD_EVENT_GETSTAT_SECTION	53
#define DLD_EVENT_MEMDUMP		54
#define DLD_EVENT_RESTART		98
#define DLD_EVENT_TERMINATE		99

#endif /* __DSP_DLD_H */
