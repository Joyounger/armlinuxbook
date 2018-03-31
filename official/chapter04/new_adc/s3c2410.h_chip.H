#define ADC_CTL_BASE		0x58000000
#define bADC_CTL(Nb)		__REG(ADC_CTL_BASE + (Nb))
/* Offset */
#define oADCCON			0x00	/* R/W, ADC control register */
#define oADCTSC			0x04	/* R/W, ADC touch screen ctl reg */
#define oADCDLY			0x08	/* R/W, ADC start or interval delay reg */
#define oADCDAT0		0x0c	/* R  , ADC conversion data reg */
#define oADCDAT1		0x10	/* R  , ADC conversion data reg */
/* Registers */
#define ADCCON			bADC_CTL(oADCCON)
#define ADCTSC			bADC_CTL(oADCTSC)
#define ADCDLY			bADC_CTL(oADCDLY)
#define ADCDAT0			bADC_CTL(oADCDAT0)
#define ADCDAT1			bADC_CTL(oADCDAT1)
/* Field */
#define fADCCON_PRSCVL		Fld(8, 6)
#define fADCCON_INPUT		Fld(3, 3)
#define fTSC_XY_PST		Fld(2, 0)
#define fADC_DELAY		Fld(6, 0)
#define fDAT_UPDOWN		Fld(1, 15)
#define fDAT_AUTO_PST		Fld(1, 14)
#define fDAT_XY_PST		Fld(2, 12)
#define fDAT_XPDATA		Fld(10, 0)
#define fDAT_YPDATA		Fld(10, 0)
/* ... */
#define ADC_IN0                 0
#define ADC_IN1                 1
#define ADC_IN2                 2
#define ADC_IN3                 3
#define ADC_IN4                 4
#define ADC_IN5                 5
#define ADC_IN6                 6
#define ADC_IN7                 7
#define ADC_BUSY		1
#define ADC_READY		0
#define NOP_MODE		0
#define X_AXIS_MODE		1
#define Y_AXIS_MODE		2
#define WAIT_INT_MODE		3
/* ... */
#define ADCCON_ECFLG		(1 << 15)
#define PRESCALE_ENDIS		(1 << 14)
#define PRESCALE_DIS		(PRESCALE_ENDIS*0)
#define PRESCALE_EN		(PRESCALE_ENDIS*1)

#define ADCCON_STDBM		(1 << 2)        /* 1: standby mode, 0: normal mode */
#define ADC_NORMAL_MODE		FClrBit(ADCCON, ADCCON_STDBM)
#define ADC_STANDBY_MODE	(ADCCON_STDBM*1)
#define ADCCON_READ_START	(1 << 1)
#define ADC_START_BY_RD_DIS	FClrBit(ADCCON, ADCCON_READ_START)
#define ADC_START_BY_RD_EN	(ADCCON_READ_START*1)
#define ADC_START		(1 << 0)
