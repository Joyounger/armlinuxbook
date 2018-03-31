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
 * 2005/06/23:  DSP Gateway version 3.3
 */

#include <std.h>
#include "tokliBIOSlib.h"
#include "omap1510.h"
#include "timer.h"

/*
 * FIXME: check the timer queue and disable timer while sleep
 */
#undef STOP_TIMER_WHILE_SLEEP

#define go_SAM() \
	do { \
		*_ST3_55 &= ~(_ST3_55_HOM_R | _ST3_55_HOM_P); \
		asm(" NOP_16"); \
		asm(" NOP_16"); \
		asm(" NOP_16"); \
		asm(" NOP_16"); \
		asm(" NOP_16"); \
		asm(" NOP_16"); \
	} while(0)

#define go_HOM() \
	do { \
		*_ST3_55 |= (_ST3_55_HOM_R | _ST3_55_HOM_P); \
		do { \
			asm(" NOP_16"); \
		} while ((*_ST3_55 & (_ST3_55_HOM_R | _ST3_55_HOM_P)) != \
			 (_ST3_55_HOM_R | _ST3_55_HOM_P)); \
	} while(0)

typedef enum {
	DOM_CPU = 0,
	DOM_DMA,
	DOM_CACHE,
	DOM_PER,
	DOM_DPLL,
	DOM_EMIF,
	DOM_MAX
} domain_num_t;

struct domain {
	Uns usecount;
};
struct domain domain[DOM_MAX];
#define dom_using(dom)	(domain[dom].usecount > 0)

static Uns icr_mask = 0;
static Uns icr_idle = 0;
#ifdef DEBUG_WAKEUP_CNT
static LgUns wakeup_cnt = 0;
#endif

#define _ICR_ALL_IDLE_DOMAIN	(_ICR_EMIF_IDLE_DOMAIN | \
				 _ICR_DPLL_IDLE_DOMAIN | \
				 _ICR_PER_IDLE_DOMAIN | \
				 _ICR_CACHE_IDLE_DOMAIN | \
				 _ICR_DMA_IDLE_DOMAIN | \
				 _ICR_CPU_IDLE_DOMAIN)

#pragma CODE_SECTION(sleep_dsp, "sleep_code");
Void sleep_dsp(Void)
{
	Uns intm_saved;
#ifdef STOP_TIMER_WHILE_SLEEP
	Uns timer1_saved;
#endif
	Uns icache_saved;

	for (;;) {
		intm_saved = HWI_disable();

		wdt_stop();
		set_load_state(LOAD_STATE_IDLE);

		if ((icr_idle & _ICR_ALL_IDLE_DOMAIN) == 0) {
			do {} while (!(*_IFR0 & *_IER0) &&
				     !(*_IFR1 & *_IER1 & 0x00ff));
			goto restore;
		}

#ifdef STOP_TIMER_WHILE_SLEEP
		if ((icr_idle & _ICR_ALL_IDLE_DOMAIN) == _ICR_ALL_IDLE_DOMAIN)
			timer1_saved = timer1_disable();
		else
			timer1_saved = 0;
#endif

		if (icr_idle & _ICR_CACHE_IDLE_DOMAIN)
			icache_saved = icache_disable();
		else
			icache_saved = 0;

		outw(icr_idle, _ICR);
		go_HOM();
		asm(" IDLE");
		asm(" NOP_16");
		asm(" NOP_16");
		asm(" NOP_16");
		asm(" NOP_16");
		asm(" NOP_16");
		asm(" NOP_16");

		if (!(icr_idle & _ICR_CPU_IDLE_DOMAIN)) {
			/*
			 * if CPU domain does not stop,
			 * we should stay here until
			 * any enabled interrupt comes.
			 * (if CPU stops, this is done by HW.)
			 */
			do {} while (!(*_IFR0 & *_IER0) &&
				     !(*_IFR1 & *_IER1 & 0x00ff));
		}

		go_SAM();
		outw(0, _ICR);
		asm(" IDLE");
		asm(" NOP_16");
		asm(" NOP_16");
		asm(" NOP_16");
		asm(" NOP_16");
		asm(" NOP_16");
		asm(" NOP_16");

		if (icache_saved)
			icache_restore(icache_saved);
#ifdef STOP_TIMER_WHILE_SLEEP
		if (timer1_saved)
			timer1_restore(timer1_saved);
#endif

restore:
		set_load_state(LOAD_STATE_BUSY);
		wdt_start();
#ifdef DEBUG_WAKEUP_CNT
		wakeup_cnt++;
#endif

		HWI_restore(intm_saved);
	}
}

static Void generate_icr(Void)
{
	Uns icr_tmp;

	/*
	 * ICR value when going idle
	 */
	icr_tmp = ((dom_using(DOM_EMIF)  ? 0 : _ICR_EMIF_IDLE_DOMAIN) |
		   (dom_using(DOM_DPLL)  ? 0 : _ICR_DPLL_IDLE_DOMAIN) |
		   (dom_using(DOM_PER)   ? 0 : _ICR_PER_IDLE_DOMAIN) |
		   (dom_using(DOM_CACHE) ? 0 : _ICR_CACHE_IDLE_DOMAIN) |
		   (dom_using(DOM_DMA)   ? 0 : _ICR_DMA_IDLE_DOMAIN) |
		   (dom_using(DOM_CPU)   ? 0 : _ICR_CPU_IDLE_DOMAIN)) &
		  icr_mask;

	/*
	 * only if all domains are going to stop, DPLL can stop.
	 * otherwise, DPLL should remain active.
	 */
	if (icr_tmp != _ICR_ALL_IDLE_DOMAIN)
		icr_tmp &= ~_ICR_DPLL_IDLE_DOMAIN;

	/*
	 * FIXME:
	 * if DMA stays alive, any other domain shouln't stop.
	 */
	if (!(icr_tmp & _ICR_DMA_IDLE_DOMAIN))
		icr_tmp = 0;

	/*
	 * fill reserved bit with 1.
	 */
	icr_tmp |= _ICR_RESERVED_BITS;

	icr_idle = icr_tmp;
}

Uns get_icr_mask(Void)
{
	return icr_mask;
}

Void set_icr_mask(Uns val)
{
	icr_mask = val;
	generate_icr();
}

Void init_icr(Void)
{
#if 0
	/*
	 * all domains stop when idle
	 */
	set_icr_mask(_ICR_EMIF_IDLE_DOMAIN |
		     _ICR_DPLL_IDLE_DOMAIN |
		     _ICR_PER_IDLE_DOMAIN |
		     _ICR_CACHE_IDLE_DOMAIN |
		     _ICR_DMA_IDLE_DOMAIN |
		     _ICR_CPU_IDLE_DOMAIN);
#else
	/*
	 * We do nothing here. GPP takes care about this.
	 */
	set_icr_mask(0);
#endif
	memset(domain, 0, sizeof(struct domain) * DOM_MAX);

	/*
	 * if sleep_dsp() is placed at other than DARAM, prevent sleep.
	 * FIXME: should be warned to ARM.
	 */
	if (mem_type(sleep_dsp, 1, PTR_CODE) != MEM_TYPE_DARAM)
		enable_domain(_ICR_ALL_IDLE_DOMAIN);
}

Void enable_domain(Uns val)
{
	Uns i;

	for (i = 0; i < DOM_MAX; i++) {
		if (val & (1 << i))
			domain[i].usecount++;
	}
	generate_icr();
}

Void disable_domain(Uns val)
{
	Uns i;

	for (i = 0; i < DOM_MAX; i++) {
		if (val & (1 << i))
			domain[i].usecount--;
	}
	generate_icr();
}

#ifdef DEBUG_WAKEUP_CNT
LgUns get_wakeup_cnt(Void)
{
	return wakeup_cnt;
}

Void clear_wakeup_cnt(Void)
{
	wakeup_cnt = 0;
}
#endif

Void init_clock(Void)
{
	/*
	 * auto idle setting
	 */
#ifdef STOP_TIMER_WHILE_SLEEP
	outw(_DSP_IDLECT1_IDLTIM_DSP |
	     _DSP_IDLECT1_IDLPER_DSP |
	     _DSP_IDLECT1_IDLWDT_DSP, _DSP_IDLECT1);
#else
	outw(_DSP_IDLECT1_IDLPER_DSP |
	     _DSP_IDLECT1_IDLWDT_DSP, _DSP_IDLECT1);
#endif
	/*
	 * we should not enable any bit in DSP_IDLECT2.
	 * all controls are done in ARM.
	 */
	//outw(_DSP_IDLECT2_EN_XORPCK, _DSP_IDLECT2);
}
