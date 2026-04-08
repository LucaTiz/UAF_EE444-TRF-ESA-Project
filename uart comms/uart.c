#include <msp430.h>

//UART Setup//
void initUART(void)
{
    //Select UART pins (double check)
    P5SEL |= BIT6 | BIT7;		// P5.6 = UCA1TXD and P5.7 = UCA1RXD

    UCA1CTL1 |= UCSWRST;		//Hold USCI in reset while configuring
    UCA1CTL1 |= UCSSEL_2;		//SMCLK as source
    //UCOS16 = 1; //Wrong implimentation, bit part of UCA0MCTL
    //table 

    //UART Baud setup depends on mode:
    //If UC0S16 =0 then N = CLK / Baud : (non-oversampling)
    //N = Clk / baud rate = 8e6 / 9600 = 833.333
    //BR0/BR1 = 833 
    //If UC0S16 = 1 then N = CLK / (16 * Baud) : (Oversample)
    //N = CLK / Baud = 8e6 / (16*115200) = 4.34...
    // Fractional portion: 0.3403; modulation value: 0.3403*16 = 5.44
    //BR0/BR1 = 4/0
    // note: MSP430 can not store 833 in single register, max value per register is 255
    //Values are stored in two registers instead

    UCA1BR0 = 52;		    //lower divider byte
    UCA1BR1 = 0;		    //upper divider byte
    UCA1MCTL = UCBRF_5 | UCBRS_0 | UCOS16;	// 1st stage modulation(fractional baud correction) + 2nd stage modulation + oversampling

    UCA1CTL1 &= ~UCSWRST; 		//Enable UART
}

//UART transmit functions
void uartSendChar(char c)
{
    while (!(UCA1IFG & UCTXIFG));
    UCA1TXBUF = c;
}
void uartSendString(const char *str)
{
    while(*str)
    {
         uartSendChar(*str++);
    }
}