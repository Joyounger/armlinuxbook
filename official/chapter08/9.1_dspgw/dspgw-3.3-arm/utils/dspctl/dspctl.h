/*
 * dspapps/utils/dspctl.h
 *
 * header for dspctl
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
 * 2004/03/17:  DSP Gateway version 3.3
 */

#include "coff-c55x.h"

#define VERSION_STR	"3.3"
#define IFVER_STR	"3.3"	/* kernel I/F version name */

#define CTLDEVNM	"/dev/dspctl/ctl"
#define TWCHDEVNM	"/dev/dspctl/twch"
#define DSPMEMDEVNM	"/dev/dspctl/mem"
#define TASKDEV_DIR	"/dev/dsptask"

struct dspgw_version {
	unsigned short major;
	unsigned short minor;
	unsigned short extra1;
	unsigned short extra2;
};

extern unsigned long load_coff(char *coffname);
extern int read_dspgw_version(struct dspgw_version *version, char *coffname);
