
#include "dsp5910.h"



/*******************************************************/
void Uart3_Setup(void)
{
  UART3_LCR = 0xBF;// access to EFR
  UART3_EFR = ENHANCED_EN | AUTO_CTS_EN | AUTO_RTS_EN; // for expand
  UART3_LCR = 0x80;//access to mcr
  
  //Sets the trigger level for the TX,RX FIFO
  UART3_MCR |= TCR_TLR;//access to tcr,tlr
  UART3_SCR |= TX_TRIG_GRANU1;//enable tx trigger 1
  UART3_TLR = 0x11;//tx trigger 1, rx trigger 4
  //UART3_FCR &= 0x0f;
  UART3_TCR = 0x0F;//for flow control
  
  UART3_LCR = 0xBF;
  UART3_EFR &= ~ENHANCED_EN;
  UART3_LCR &= ~DIV_EN;
  UART3_MCR &= ~TCR_TLR;
  
  
  UART3_LCR =  CHAR_LENGTH_8;
  UART3_OSC_12M_SEL = 1;// en 12m/6.5 to support 115200
  
  //Setup Baud Rate
  UART3_LCR |= DIV_EN;
  UART3_DLL = 0x1;//115200, 0x4e to 9600
  UART3_DLH = 0x0;
  UART3_LCR &= ~DIV_EN;
  
  UART3_IER |= RHR_IT;// enable rx int
  
  UART3_MDR1 = MODE_SELEC_UART;
}

void Uart3_Send(char *buffer)
{
  while( *buffer != '\0')
    {
      while( ! (UART3_LSR & TX_FIFO_E));
      UART3_THR = *buffer++;
    }
  
}

void Uart3_SendByte(char * bt)
{
  while( ! (UART3_LSR & TX_FIFO_E));
  UART3_THR = *bt;
}

void Uart3_ReadByte(char *rlt)
{
  while( !(UART3_LSR & RX_FIFO_E));
  *rlt = UART3_RHR;
}

/*******************************************************/


