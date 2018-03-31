/*
 * Copyright (c) 2002-2005, Nokia
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
 * 2005/05/19:  DSP Gateway version 3.3
 */

#define MBPROT_REVISION	0x0019

#define MBCMD_WDSND	0x10
#define MBCMD_WDREQ	0x11
#define MBCMD_BKSND	0x20
#define MBCMD_BKREQ	0x21
#define MBCMD_BKYLD	0x23
#define MBCMD_BKSNDP	0x24
#define MBCMD_BKREQP	0x25
#define MBCMD_TCTL	0x30
#define MBCMD_TCTLDATA	0x31
#define MBCMD_POLL	0x32
#define MBCMD_RUNLEVEL	0x51
#define MBCMD_PM	0x52
#define MBCMD_SUSPEND	0x53
#define MBCMD_KFUNC	0x54
#define MBCMD_TCFG	0x60
#define MBCMD_TADD	0x62
#define MBCMD_TDEL	0x63
#define MBCMD_TSTOP	0x65
#define MBCMD_DSPCFG	0x70
#define MBCMD_REGRW	0x72
#define MBCMD_GETVAR	0x74
#define MBCMD_SETVAR	0x75
#define MBCMD_ERR	0x78
#define MBCMD_DBG	0x79

#define MBCMD_UARTSET  0x0E
#define MBCMD_FBTSET   0x0F


#define MBCMD_TCTL_TINIT	0x0000
#define MBCMD_TCTL_TEN		0x0001
#define MBCMD_TCTL_TDIS		0x0002
#define MBCMD_TCTL_TCLR		0x0003
#define MBCMD_TCTL_TKILL	0x0004

#define MBCMD_POLL_WDT		0x0001
#define MBCMD_POLL_ARM		0x0002

#define MBCMD_RUNLEVEL_USER	0x01
#define MBCMD_RUNLEVEL_SUPER	0x0e
#define MBCMD_RUNLEVEL_RECOVERY	0x10

#define MBCMD_PM_DISABLE	0x00
#define MBCMD_PM_ENABLE		0x01

#define MBCMD_KFUNC_FBCTL	0x00

#define MBCMD_FBCTL_ENABLE	0x0002
#define MBCMD_FBCTL_DISABLE	0x0003

#define MBCMD_TDEL_SAFE		0x0000
#define MBCMD_TDEL_KILL		0x0001

#define MBCMD_DSPCFG_REQ	0x00
#define MBCMD_DSPCFG_SYSADRH	0x28
#define MBCMD_DSPCFG_SYSADRL	0x29
#define MBCMD_DSPCFG_PROTREV	0x70
#define MBCMD_DSPCFG_ABORT	0x78
#define MBCMD_DSPCFG_LAST	0x80

#define MBCMD_REGRW_MEMR	0x00
#define MBCMD_REGRW_MEMW	0x01
#define MBCMD_REGRW_IOR		0x02
#define MBCMD_REGRW_IOW		0x03
#define MBCMD_REGRW_DATA	0x04

#define MBCMD_VARID_ICRMASK	0x00
#define MBCMD_VARID_LOADINFO	0x01

#define MBCMD_TTYP_ARCV		0x0001
#define MBCMD_TTYP_PRCV		0x0000
#define MBCMD_TTYP_ASND		0x0002
#define MBCMD_TTYP_PSND		0x0000
#define MBCMD_TTYP_BKMD		0x0004
#define MBCMD_TTYP_WDMD		0x0000
#define MBCMD_TTYP_BKDM		0x0008
#define MBCMD_TTYP_WDDM		0x0000
#define MBCMD_TTYP_PVMD		0x0010
#define MBCMD_TTYP_GBMD		0x0000
#define MBCMD_TTYP_PVDM		0x0020
#define MBCMD_TTYP_GBDM		0x0000

#define MBCMD_EID_NOERR		0x00
#define MBCMD_EID_BADTID	0x10
#define MBCMD_EID_BADTCN	0x11
#define MBCMD_EID_BADBID	0x20
#define MBCMD_EID_BADCNT	0x21
#define MBCMD_EID_NOTLOCKED	0x22
#define MBCMD_EID_STVBUF	0x23
#define MBCMD_EID_BADADR	0x24
#define MBCMD_EID_BADTCTL	0x30
#define MBCMD_EID_BADPARAM	0x50
#define MBCMD_EID_FATAL		0x58
#define MBCMD_EID_NOMEM		0xc0
#define MBCMD_EID_NORES		0xc1
#define MBCMD_EID_IPBFULL	0xc2
#define MBCMD_EID_WDT		0xd0
#define MBCMD_EID_TASKNOTRDY	0xe0
#define MBCMD_EID_TASKBSY	0xe1
#define MBCMD_EID_TASKERR	0xef
#define MBCMD_EID_BADCFGTYP	0xf0
#define MBCMD_EID_DEBUG		0xf8
#define MBCMD_EID_BADSEQ	0xfe
#define MBCMD_EID_BADCMD	0xff

#define MBCMD_TNM_LEN		16

#define MBCMD_TID_FREE		0xff
#define MBCMD_TID_ANON		0xfe

#define MBCMD_BID_NULL		0xffff
#define MBCMD_BID_PVT		0xfffe

