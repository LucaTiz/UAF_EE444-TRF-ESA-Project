#include <msp430.h>

//UART Setup// 57600 baud, oversampling, 8 data, 1 stop, even parity
void initUART(void)
{
    //Select UART pins (double check)
    P5SEL |= BIT6 | BIT7;		//P5.6 = UCA1TXD and P5.7 = UCA1RXD; for UCA1
    UCA1CTL1 |= UCSWRST;		//Hold USCI in reset while configuring
    UCA1CTL1 |= UCSSEL_2;		//SMCLK as source
    UCA1CTL0 = UCPEN + UCPAR;	//even parity, 8 data bits, 1 stop bit
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
    while (!(UCA0IFG & UCTXIFG));
    UCA0TXBUF = c;
}
void uartSendString(const char *str)
{
    while(*str)
    {
         uartSendChar(*str++);
    }
}    

