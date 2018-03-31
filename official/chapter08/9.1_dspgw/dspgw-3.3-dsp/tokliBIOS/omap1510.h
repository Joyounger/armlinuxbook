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
 * 2005/06/20:  DSP Gateway version 3.3
 */

#define inw(adr)	(*(volatile ioport Uns *)(adr))
#define outw(dat,adr)	(*(volatile ioport Uns *)(adr) = (dat))
#define inl(adr)	(*(volatile ioport LgUns *)(adr))
#define outl(dat,adr)	(*(volatile ioport LgUns *)(adr) = (dat))

#define DARAM_BASE	0x000000
#define DARAM_SIZE	0x008000
#define SARAM_BASE	0x008000
#define SARAM_SIZE	0x00c000
#define DSPRAM_SIZE	0x014000

/*
 * ioport addresses (16bit word address)
 */
#define _ICR		0x0001
#define _ISR		0x0002

#define _ICR_RESERVED_BITS	0xffc0
#define _ICR_EMIF_IDLE_DOMAIN	0x0020
#define _ICR_DPLL_IDLE_DOMAIN	0x0010
#define _ICR_PER_IDLE_DOMAIN	0x0008
#define _ICR_CACHE_IDLE_DOMAIN	0x0004
#define _ICR_DMA_IDLE_DOMAIN	0x0002
#define _ICR_CPU_IDLE_DOMAIN	0x0001

/*
 * I-Cache
 */
#define _ICACHE_GCR	0x1400
#define _ICACHE_NWCR	0x1403
#define _ICACHE_ISR	0x1404

#define _ICACHE_GCR_CUT_CLOCK		0x8000
#define _ICACHE_GCR_AUTO_GATING		0x4000
#define _ICACHE_GCR_FLUSH_LINE		0x1000
#define _ICACHE_GCR_GLOBAL_FLUSH	0x0800
#define _ICACHE_GCR_RAMSET_PRESENCE	0x0400
#define _ICACHE_GCR_WAY_PRESENCE	0x0200
#define _ICACHE_GCR_RAMSET_NUMBER_MASK	0x01e0
#define _ICACHE_GCR_RAMSET_NUMBER_1	0x0000
#define _ICACHE_GCR_RAMSET_NUMBER_2	0x0020
#define _ICACHE_GCR_WAY_NUMBER_MASK	0x0018
#define _ICACHE_GCR_WAY_NUMBER_1	0x0000
#define _ICACHE_GCR_WAY_NUMBER_2	0x0008
#define _ICACHE_GCR_STREAMING		0x0004
#define _ICACHE_GCR_RAM_FILL_MODE	0x0002
#define _ICACHE_GCR_GLOBAL_ENABLE	0x0001
#define _ICACHE_NWCR_WAY_SIZE_MASK	0x001c
#define _ICACHE_NWCR_WAY_SIZE_8K	0x000c
#define _ICACHE_NWCR_FLUSH		0x0002
#define _ICACHE_NWCR_ENABLE		0x0001
#define _ICACHE_ISR_ENABLE		0x0004
#define _ICACHE_ISR_DEBUG_MODE		0x0002
#define _ICACHE_ISR_BUSERROR		0x0001

/*
 * Timer
 */
#define	_CNTL_TIMER1	0x2800
#define _LOAD_TIM1	0x2802
#define _LOAD_TIM_HI1	0x2802
#define _LOAD_TIM_LO1	0x2803
#define _READ_TIM1	0x2804
#define _READ_TIM_HI1	0x2804
#define _READ_TIM_LO1	0x2805

#define	_CNTL_TIMER2	0x2c00
#define _LOAD_TIM2	0x2c02
#define _LOAD_TIM_HI2	0x2c02
#define _LOAD_TIM_LO2	0x2c03
#define _READ_TIM2	0x2c04
#define _READ_TIM_HI2	0x2c04
#define _READ_TIM_LO2	0x2c05

#define	_CNTL_TIMER3	0x3000
#define _LOAD_TIM3	0x3002
#define _LOAD_TIM_HI3	0x3002
#define _LOAD_TIM_LO3	0x3003
#define _READ_TIM3	0x3004
#define _READ_TIM_HI3	0x3004
#define _READ_TIM_LO3	0x3005

#define _CNTL_TIMERn_ST			0x0001
#define _CNTL_TIMERn_AR			0x0002
#define _CNTL_TIMERn_PTV(ptv)		((ptv)<<2)
#define _CNTL_TIMERn_CLOCK_ENABLE	0x0020
#define _CNTL_TIMERn_FREE		0x0040
#define _CNTL_TIMERn_SOFT		0x0080

/*
 * WDT
 */
#define _CNTL_TIMER	0x3400
#define _LOAD_TIM	0x3402
#define _READ_TIM	0x3402
#define _TIMER_MODE	0x3404

#define _CNTL_TIMER_FREE	0x0001
#define _CNTL_TIMER_ST		0x0080
#define _CNTL_TIMER_AR		0x0100
#define _CNTL_TIMER_PTV_MASK	0x0e00
#define _CNTL_TIMER_PTV(ptv)	((ptv)<<9)

/*
 * Clock
 */
#define _DSP_CKCTL	0x4000
#define _DSP_IDLECT1	0x4002
#define _DSP_IDLECT2	0x4004
#define _DSP_RSTCT2	0x400a
#define _DSP_SYSST	0x400c

#define _DSP_CKCTL_TIMXO		0x0100
#define _DSP_IDLECT1_IDLTIM_DSP		0x0100
#define _DSP_IDLECT1_IDLGPIO_DSP	0x0080
#define _DSP_IDLECT1_WKUP_MODE		0x0040
#define _DSP_IDLECT1_IDLDPLL_DSP	0x0020
#define _DSP_IDLECT1_IDLIF_DSP		0x0010
#define _DSP_IDLECT1_IDLPER_DSP		0x0004
#define _DSP_IDLECT1_IDLXORP_DSP	0x0002
#define _DSP_IDLECT1_IDLWDT_DSP		0x0001
#define _DSP_IDLECT2_EN_TIMCK		0x0020
#define _DSP_IDLECT2_EN_GPIOCK		0x0010
#define _DSP_IDLECT2_EN_PERCK		0x0004
#define _DSP_IDLECT2_EN_XORPCK		0x0002
#define _DSP_IDLECT2_EN_WDTCK		0x0001

/*
 * Mailbox
 */
#define _ARM2DSP1	0xf800
#define _ARM2DSP1b	0xf802
#define _DSP2ARM1	0xf804
#define _DSP2ARM1b	0xf806
#define _DSP2ARM2	0xf808
#define _DSP2ARM2b	0xf80a
#define _ARM2DSP1_Flag	0xf80c
#define _DSP2ARM1_Flag	0xf80e
#define _DSP2ARM2_Flag	0xf810

/********************************************************
 * OMAP5910 TIPB Switch Registers*
 *********************************************************/
#define _RHSW_DSP_CNF1  (0xE400)
#define _RHSW_DSP_STA1  (0xE401)
#define _RHSW_DSP_CNF2  (0xE410)
#define _RHSW_DSP_STA2  (0xE411)
#define _RHSW_DSP_CNF3  (0xE420)
#define _RHSW_DSP_STA3  (0xE421)
/********************************************************
 * C55x Clock Generation and Reset Control Registers	*
 ********************************************************/
#define DSP_IDLECT2				(*(volatile ioport unsigned short*)0x4004)
#define DSP_RSTCT2				(*(volatile ioport unsigned short*)0x400a)

//DSP_IDLECT2
#define EN_TIMCK_DSP			((0x1 << 5))
#define EN_GPIOCK_DSP			((0x1 << 4))
#define EN_XORPCK_DSP			((0x1 << 1))
#define EN_WDTCK_DSP			((0x1 << 0))

//DSP_RSTCT2
#define PER_EN_DSP					((0x1 << 0)) // peripherals released from reset


/********************************************************
																*
 ********************************************************/

/*
 * memory mapped registers
 */
#define _IER0	((volatile Uns *)0x00)
#define _IFR0	((volatile Uns *)0x01)
#define _ST1_55	((volatile Uns *)0x03)
#define _ST3_55	((volatile Uns *)0x04)
#define _IER1	((volatile Uns *)0x45)
#define _IFR1	((volatile Uns *)0x46)
#define _IVPD	((volatile Uns *)0x49)
#define _IVPH	((volatile Uns *)0x4a)

#define _ST1_55_INTM	0x0800
#define _ST3_55_CAFRZ	0x8000
#define _ST3_55_CAEN	0x4000
#define _ST3_55_CACLR	0x2000
#define _ST3_55_HOM_R	0x0200
#define _ST3_55_HOM_P	0x0100

/*
 * interrupts
 */
#define INT_TC_ABORT	4
#define INT_MAILBOX1	5
#define INT_HSAB	6
#define INT_GPIO	7
#define INT_TIMER3	8
#define INT_DMA1	9
#define INT_MCU		10
#define INT_UART	12
#define INT_WDGTIMER	13
#define INT_DMA4	14
#define INT_DMA5	15
#define INT_EMIF	16
#define INT_LOCALBUS	17
#define INT_DMA0	18
#define INT_TIPB	19
#define INT_DMA2	20
#define INT_DMA3	21
#define INT_TIMER2	22
#define INT_TIMER1	23
