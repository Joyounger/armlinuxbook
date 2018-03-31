#ifndef _S3C2410_ADC_H_
#define _S3C2410_ADC_H_
#define ADC_WRITE(ch, prescale) ((ch)<<16|(prescale))
#define ADC_WRITE_GETCH(data) (((data)>>16)&0x7)
#define ADC_WRITE_GETPRE(data) ((data)&0xff)
#define ADC_INPUT(x)  (x<<3)
#define PRSCVL(x)  (x<<6)
#endif /* _S3C2410_ADC_H_ */
