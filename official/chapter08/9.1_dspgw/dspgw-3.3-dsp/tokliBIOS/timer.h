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
 * 2005/06/02:  DSP Gateway version 3.3
 */

/*
 * start / stop functions control ST bit
 * ck_enable / disable functions control CLOCK_ENABLE bit
 * enable / disable functions control both ST and CLOCK_ENABLE bits
 * restore function restores ST and CLOCK_ENABLE bits previously saved
 */
#define DECLARE_TIMER_FUNC(n) \
	inline Uns timer##n##_start(Void) \
	{ \
		Uns val = inw(_CNTL_TIMER##n); \
		outw(val | _CNTL_TIMERn_ST, _CNTL_TIMER##n); \
		return val & _CNTL_TIMERn_ST; \
	} \
	inline Uns timer##n##_stop(Void) \
	{ \
		Uns val = inw(_CNTL_TIMER##n); \
		outw(val & ~_CNTL_TIMERn_ST, _CNTL_TIMER##n); \
		return val & _CNTL_TIMERn_ST; \
	} \
	inline Uns timer##n##_ck_enable(Void) \
	{ \
		Uns val = inw(_CNTL_TIMER##n); \
		outw(val | _CNTL_TIMERn_CLOCK_ENABLE, _CNTL_TIMER##n); \
		return val & _CNTL_TIMERn_CLOCK_ENABLE; \
	} \
	inline Uns timer##n##_ck_disable(Void) \
	{ \
		Uns val = inw(_CNTL_TIMER##n); \
		outw(val & ~_CNTL_TIMERn_CLOCK_ENABLE, _CNTL_TIMER##n); \
		return val & _CNTL_TIMERn_CLOCK_ENABLE; \
	} \
	inline Uns timer##n##_enable(Void) \
	{ \
		Uns val = inw(_CNTL_TIMER##n); \
		outw(val | _CNTL_TIMERn_ST | _CNTL_TIMERn_CLOCK_ENABLE, \
		     _CNTL_TIMER##n); \
		return val & (_CNTL_TIMERn_ST | _CNTL_TIMERn_CLOCK_ENABLE); \
	} \
	 \
	inline Uns timer##n##_disable(Void) \
	{ \
		Uns val = inw(_CNTL_TIMER##n); \
		outw(val & ~(_CNTL_TIMERn_ST | _CNTL_TIMERn_CLOCK_ENABLE), \
		     _CNTL_TIMER##n); \
		return val & (_CNTL_TIMERn_ST | _CNTL_TIMERn_CLOCK_ENABLE); \
	} \
	inline Void timer##n##_restore(Uns val) \
	{ \
		outw((inw(_CNTL_TIMER##n) & \
		      ~(_CNTL_TIMERn_ST | _CNTL_TIMERn_CLOCK_ENABLE)) | \
		     (val & (_CNTL_TIMERn_ST | _CNTL_TIMERn_CLOCK_ENABLE)), \
		     _CNTL_TIMER##n); \
	}

DECLARE_TIMER_FUNC(1)
DECLARE_TIMER_FUNC(2)
DECLARE_TIMER_FUNC(3)

#define __wdt_start() \
	do { \
		outw(inw(_CNTL_TIMER) | _CNTL_TIMER_ST, _CNTL_TIMER); \
	} while(0)

#define __wdt_stop() \
	do { \
		outw(inw(_CNTL_TIMER) & ~_CNTL_TIMER_ST, _CNTL_TIMER); \
	} while(0)

#define __wdt_ck_enable() \
	do { \
		outw(inw(_DSP_IDLECT2) | _DSP_IDLECT2_EN_WDTCK, _DSP_IDLECT2); \
	} while(0)

#define __wdt_disable() \
	do { \
		outw(0xf5, _TIMER_MODE); \
		outw(0xa0, _TIMER_MODE); \
	} while(0)
