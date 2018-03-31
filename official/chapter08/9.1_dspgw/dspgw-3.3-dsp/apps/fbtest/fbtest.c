/*
 * Copyright (c) 2004-2005, Nokia
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer. 
 *
 * Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution. 
 *
 * Neither the name of Nokia nor the names of its contributors may be used to
 * endorse or promote products derived from this software without specific
 * prior written permission. 
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * Toshihiro Kobayashi <toshihiro.kobayashi@nokia.com>
 * 2004/09/27:  DSP Gateway version 3.3
 */

#include <std.h>
#include "mailbox.h"
#include "tokliBIOS.h"


#define FB_START 0x300800
#define FB_SIZE  0x04B000
#define CAM_START (FB_START + FB_SIZE)			

extern void uart_printf ( Char *fmt, ...);

static Uns demo_rcv_wdsnd(struct dsptask *task, Uns data)
{
	long  i;
        uart_printf("DSP received wordsend, data = 0x%x\n",data);

	if( data == 0xffff)
	        wdsnd(task, 0x0);	/* '<' */
	if( data == 0x0){
	  for(i = FB_START; i < FB_START + 0x12C00 ; i++)
	        *((unsigned short*) i)=0x07e0;
	  for(i = FB_START + 0x12C00; i < FB_START + 0x25800 ; i++)
	        *((unsigned short*) i)=0xffff;
	  for(i = FB_START + 0x25800; i < FB_START + 0x38400 ; i++)
	        *((unsigned short*) i)=0x001f;
	  for(i = FB_START + 0x38400; i < FB_START + 0x4B000 ; i++)
	        *((unsigned short*) i)=0xf800;
	}
	return 0;
}

#pragma DATA_SECTION(task_fbtest, "dspgw_task")
struct dsptask task_fbtest = {
	TID_MAGIC,	/* tid */
	"fbtest",	/* name */
	MBCMD_TTYP_WDDM | MBCMD_TTYP_WDMD |
	MBCMD_TTYP_ASND | MBCMD_TTYP_PRCV,
			/* ttyp: active word snd, passive word rcv */
	demo_rcv_wdsnd,	/* rcv_snd */
	NULL,		/* rcv_req */
	NULL,		/* rcv_tctl */
	NULL,		/* tsk_attrs */
	NULL,		/* mmap_info */
	NULL		/* udata */
};
 
