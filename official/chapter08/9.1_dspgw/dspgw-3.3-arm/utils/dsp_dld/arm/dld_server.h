/*
 * dsp_dld/arm/dld_server.h
 *
 * DSP Dynamic Loader Daemon: dld_server.h
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
 * 2005/05/19:  DSP Gateway version 3.3
 */

#ifndef __DLD_SERVER_H
#define __DLD_SERVER_H

#ifndef DSP_EMULATION
extern int twch_tadd(struct taskent *te);
extern int twch_tdel(struct taskent *te);
#endif
extern server_return_t server(void);
extern void dspsig_handler(int signum);

#endif /* __DLD_SERVER_H */
