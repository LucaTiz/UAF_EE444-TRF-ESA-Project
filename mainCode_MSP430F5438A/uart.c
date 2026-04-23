#include <msp430.h>

//UART Setup// 57600 baud, oversampling, 8 data, 1 stop, even parity
void initUART(void)
{
    //Select UART pins (double check)
    P3SEL |= BIT4;		//P3.4 = UCA0TXD
    P3DIR |= BIT4;                      //UCA0TXD output pin direction
    UCA0CTL1 |= UCSWRST;		//Hold USCI in reset while configuring
    UCA0CTL1 |= UCSSEL_3;		//SMCLK as source
    UCA0CTL0 = UCPEN + UCPAR;	//even parity, 8 data bits, 1 stop bit
    //Divider for Clock:
    //oversampling on, reccommended values from table 36-5
    UCA0BR0 = 8;		//lower 8 bits
    UCA0BR1 = 0;			//upper 8 bits
    UCA0MCTL = UCBRS_0 | UCBRF_11 | UCOS16;
    UCA0CTL1 &= ~UCSWRST; 		//Enable UART
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

