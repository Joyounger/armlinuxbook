/*
 * dsp_dld/arm/dld_malloc.c
 *
 * DSP Dynamic Loader Daemon: dld_malloc.c
 *
 * Copyright (C) 2005 Nokia Corporation
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

#include <string.h>
#include <malloc.h>
#include "dsp_dld.h"
#include "dld_daemon.h"

void *dld_malloc(size_t size, char *msg)
{
	void *p = malloc(size);

#ifdef DEBUG_MALLOC
	prmsg("alloc %s: %p\n", msg, p);
#endif
	return p;
}

char *dld_strdup(char *s, char *msg)
{
	char *p = strdup(s);

#ifdef DEBUG_MALLOC
	prmsg("alloc %s: %p\n", msg, p);
#endif
	return p;
}

void dld_free(void *p, char *msg)
{
#ifdef DEBUG_MALLOC
	prmsg("free %s: %p\n", msg, p);
#endif
	free(p);
}
