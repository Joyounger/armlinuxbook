/********************************************************
 * OMAP5910 UART Registers								*
*********************************************************/

#define OMAP_MPU_UART1_BASE		0x8000
#define OMAP_MPU_UART2_BASE		0x8400
#define OMAP_MPU_UART3_BASE		0xcc00

//******************** 	UART1
#define UART1_RHR	 			(*(volatile ioport unsigned short*)(OMAP_MPU_UART1_BASE + 0))
#define UART1_THR	 			(*(volatile ioport unsigned short*)(OMAP_MPU_UART1_BASE + 0))
#define UART1_DLL	 			(*(volatile ioport unsigned short*)(OMAP_MPU_UART1_BASE + 0))
#define UART1_IER	 			(*(volatile ioport unsigned short*)(OMAP_MPU_UART1_BASE + 1))
#define UART1_DLH	 			(*(volatile ioport unsigned short*)(OMAP_MPU_UART1_BASE + 1))
#define UART1_IIR	 			(*(volatile ioport unsigned short*)(OMAP_MPU_UART1_BASE + 2))	
#define UART1_FCR	 			(*(volatile ioport unsigned short*)(OMAP_MPU_UART1_BASE + 2))
#define UART1_EFR	 			(*(volatile ioport unsigned short*)(OMAP_MPU_UART1_BASE + 2))
#define UART1_LCR	 			(*(volatile ioport unsigned short*)(OMAP_MPU_UART1_BASE + 0x3))
#define UART1_MCR	 			(*(volatile ioport unsigned short*)(OMAP_MPU_UART1_BASE + 0x4))
#define UART1_XON1 				(*(volatile ioport unsigned short*)(OMAP_MPU_UART1_BASE + 0x4))
#define UART1_LSR	 			(*(volatile ioport unsigned short*)(OMAP_MPU_UART1_BASE + 0x5))
#define UART1_XON2 				(*(volatile ioport unsigned short*)(OMAP_MPU_UART1_BASE + 0x5))
#define UART1_TCR	 			(*(volatile ioport unsigned short*)(OMAP_MPU_UART1_BASE + 0x6))
#define UART1_MSR	 			(*(volatile ioport unsigned short*)(OMAP_MPU_UART1_BASE + 0x6))
#define UART1_XOFF1	 			(*(volatile ioport unsigned short*)(OMAP_MPU_UART1_BASE + 0x6))
#define UART1_SPR	 			(*(volatile ioport unsigned short*)(OMAP_MPU_UART1_BASE + 0x7))
#define UART1_TLR	 			(*(volatile ioport unsigned short*)(OMAP_MPU_UART1_BASE + 0x7))
#define UART1_XOFF2	 			(*(volatile ioport unsigned short*)(OMAP_MPU_UART1_BASE + 0x7))
#define UART1_MDR1	 			(*(volatile ioport unsigned short*)(OMAP_MPU_UART1_BASE + 0x8))
#define UART1_UASR	 			(*(volatile ioport unsigned short*)(OMAP_MPU_UART1_BASE + 0x9))
#define UART1_SCR	 			(*(volatile ioport unsigned short*)(OMAP_MPU_UART1_BASE + 0x10))
#define UART1_SSR	 			(*(volatile ioport unsigned short*)(OMAP_MPU_UART1_BASE + 0x11))
#define UART1_OSC_12M_SEL 		(*(volatile ioport unsigned short*)(OMAP_MPU_UART1_BASE + 0x13))
#define UART1_MVR	 			(*(volatile ioport unsigned short*)(OMAP_MPU_UART1_BASE + 0x14))


//******************** 	UART3
#define UART3_RHR	 			(*(volatile ioport unsigned short*)(OMAP_MPU_UART3_BASE + 0))
#define UART3_THR	 			(*(volatile ioport unsigned short*)(OMAP_MPU_UART3_BASE + 0))                                                    
#define UART3_DLL	 			(*(volatile ioport unsigned short*)(OMAP_MPU_UART3_BASE + 0))                                                    
#define UART3_IER	 			(*(volatile ioport unsigned short*)(OMAP_MPU_UART3_BASE + 1))                                                    
#define UART3_DLH	 			(*(volatile ioport unsigned short*)(OMAP_MPU_UART3_BASE + 1))                                                    
#define UART3_IIR	 			(*(volatile ioport unsigned short*)(OMAP_MPU_UART3_BASE + 2))	                                                     
#define UART3_FCR	 			(*(volatile ioport unsigned short*)(OMAP_MPU_UART3_BASE + 2))                                                    
#define UART3_EFR	 			(*(volatile ioport unsigned short*)(OMAP_MPU_UART3_BASE + 2))                                                    
#define UART3_LCR	 			(*(volatile ioport unsigned short*)(OMAP_MPU_UART3_BASE + 0x3))                                                       
#define UART3_MCR	 			(*(volatile ioport unsigned short*)(OMAP_MPU_UART3_BASE + 0x4))                                                       
#define UART3_XON1 				(*(volatile ioport unsigned short*)(OMAP_MPU_UART3_BASE + 0x4))                                                       
#define UART3_LSR	 			(*(volatile ioport unsigned short*)(OMAP_MPU_UART3_BASE + 0x5))                                                       
#define UART3_XON2 				(*(volatile ioport unsigned short*)(OMAP_MPU_UART3_BASE + 0x5))                                                       
#define UART3_TCR	 			(*(volatile ioport unsigned short*)(OMAP_MPU_UART3_BASE + 0x6))                                                       
#define UART3_MSR	 			(*(volatile ioport unsigned short*)(OMAP_MPU_UART3_BASE + 0x6))                                                       
#define UART3_XOFF1	 			(*(volatile ioport unsigned short*)(OMAP_MPU_UART3_BASE + 0x6))                                                       
#define UART3_SPR	 			(*(volatile ioport unsigned short*)(OMAP_MPU_UART3_BASE + 0x7))                                                       
#define UART3_TLR	 			(*(volatile ioport unsigned short*)(OMAP_MPU_UART3_BASE + 0x7))                                                       
#define UART3_XOFF2	 			(*(volatile ioport unsigned short*)(OMAP_MPU_UART3_BASE + 0x7))                                                       
#define UART3_MDR1	 			(*(volatile ioport unsigned short*)(OMAP_MPU_UART3_BASE + 0x8))                                                       
#define UART3_UASR	 			(*(volatile ioport unsigned short*)(OMAP_MPU_UART3_BASE + 0x9))                                                       
#define UART3_SCR	 			(*(volatile ioport unsigned short*)(OMAP_MPU_UART3_BASE + 0x10))                                                       
#define UART3_SSR	 			(*(volatile ioport unsigned short*)(OMAP_MPU_UART3_BASE + 0x11))                                                       
#define UART3_OSC_12M_SEL 		(*(volatile ioport unsigned short*)(OMAP_MPU_UART3_BASE + 0x13))                                                       
#define UART3_MVR	 			(*(volatile ioport unsigned short*)(OMAP_MPU_UART3_BASE + 0x14))                                                       



//FCR	FIFO Control Register
#define RX_FIFO_TRIG_8			((0x0 << 6))	//depand on scr.7
#define RX_FIFO_TRIG_16			((0x1 << 6))
#define RX_FIFO_TRIG_56			((0x2 << 6))
#define RX_FIFO_TRIG_60			((0x3 << 6))
#define TX_FIFO_TRIG_8			((0x0 << 4))	//depand on scr.6 
#define TX_FIFO_TRIG_16			((0x1 << 4))
#define TX_FIFO_TRIG_56			((0x2 << 4))
#define TX_FIFO_TRIG_60			((0x3 << 4))
#define DMA_MODE0				((0x0 << 3))
#define DMA_MODE1				((0x1 << 3))	//only valid when scr.0=0
#define TX_FIFO_CLEAR			((0x1 << 2))
#define RX_FIFO_CLEAR			((0x1 << 1))
#define FIFO_EN					((0x1 << 0))

//SCR	Supplementary Control Register
#define RX_TRIG_GRANU1			((0x1 << 7))
#define TX_TRIG_GRANU1			((0x1 << 6))
#define DSR_IT					((0x1 << 5))	//Enables the DSR interrupt
#define RX_CTS_DSR_WAKE_UP_ENABLE ((0x1 << 4))
#define TX_EMPTY_CTL_IT			((0x1 << 3))
#define DMA_MODE_2				((0x1 << 2))
#define DMA_MODE_3				((0x3 << 2))
#define DMA_MODE_CT				((0x1 << 0))

//LCR	Line Control Register
#define DIV_EN					((0x1 << 7))
#define BREAK_EN				((0x1 << 6))
#define PARITY_TYPE2			((0x1 << 5))
#define PARITY_TYPE1			((0x1 << 4))
#define PARITY_EN				((0x1 << 3))
#define NB_STOP					((0x1 << 2))
#define CHAR_LENGTH_8			((0x3 << 0))

//LSR 	UART Mode Line Status Register
#define RX_FIFO_STS				((0x1 << 7))
#define TX_SR_E					((0x1 << 6))
#define TX_FIFO_E				((0x1 << 5))	//tx fifo empty
#define RX_BI					((0x1 << 4))
#define RX_FE					((0x1 << 3))
#define RX_PE					((0x1 << 2))
#define RX_OE 					((0x1 << 1))
#define RX_FIFO_E				((0x1 << 0))	//rx fifo not empty

//SSR 	Supplementary Status Register
#define RX_CTS_DSR_WAKE_UP_STS	((0x1 << 1))
#define TX_FIFO_FULL			((0x1 << 0))	//tx fifo full

//MCR 	Modem Control Register
#define CLKSEL					((0x1 << 7))	//clk/4
#define TCR_TLR					((0x1 << 6))	//enable tcr,tlr
#define XON_EN					((0x1 << 5)) 	//xon for any
#define LOOPBACK_EN				((0x1 << 4)) 	//mcr to msr
#define RTS						((0x1 << 1))  	//rts to low
#define DTR						((0x1 << 0)) 	//dtr to low

//MSR 	Modem Status Register
#define NDSR_STS				((0x1 << 5)) 	//dsr 
#define NCTS_STS				((0x1 << 4)) 	//cts
#define DSR_STS					((0x1 << 1))  	//dsr changed
#define CTS_STS					((0x1 << 0)) 	//cts changed

//IER	Interrupt Enable Register
#define CTS_IT					((0x1 << 7)) 	
#define RTS_IT					((0x1 << 6))
#define XOFF_IT					((0x1 << 5))
#define SLEEP_MODE				((0x1 << 4)) 	//sleep mood enable
#define MODEM_STS_IT			((0x1 << 3)) 	//modem status reg
#define LINE_STS_IT				((0x1 << 2)) 	//line status reg
#define THR_IT					((0x1 << 1))
#define RHR_IT					((0x1 << 0))

//IIR 	Interrupt Identification Register
#define IT_PENDING				((0x1 << 0)) 	//no interrupt pending

//EFR	Enhanced Feature Register
#define AUTO_CTS_EN				((0x1 << 7))
#define AUTO_RTS_EN				((0x1 << 6))
#define SPECIAL_CHAR_DETECT		((0x1 << 5))
#define ENHANCED_EN				((0x1 << 4))

//TCR	Transmission Control Register
#define RX_FIFO_TRIG_START_MASK	((0xf << 4))	//RESTORE
#define RX_FIFO_TRIG_HALT_MASK	((0xf << 0))	//halt

//TLR	Trigger Level Register
#define RX_FIFO_TRIG_DMA_MASK	((0xf << 4))	//scr.7
#define TX_FIFO_TRIG_DMA_MASK	((0xf << 0))	//scr.6

//MDR1	Mode Definition Register
#define MODE_SELEC_UART			((0x0 << 0))
#define MODE_SELEC_AUTO			((0x2 << 0))
#define MODE_SELEC_DISABLE		((0x7 << 0))

//OSC_12M_SEL 	OSC_12_MHz Register Select
#define OSC_12M_SEL				((0x1 << 0)) 	// 12m/6.5
 
