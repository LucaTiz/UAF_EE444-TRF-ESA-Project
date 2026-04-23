#include <msp430.h>

//UART Setup// 57600 baud, oversampling, 8 data, 1 stop, even parity
void initUART(void)
{
    //Select UART pins (double check)
    P5SEL |= BIT6 | BIT7;		//P5.6 = UCA1TXD and P5.7 = UCA1RXD; for UCA1
    P5DIR |= BIT6;                      //UCA1TXD output pin direction
    UCA1CTL1 |= UCSWRST;		//Hold USCI in reset while configuring
    UCA1CTL1 |= UCSSEL_3;		//SMCLK as source
    UCA1CTL0 = UCPEN + UCPAR;	//even parity, 8 data bits, 1 stop bit
    //Divider for Clock:
    //oversampling on, reccommended values from table 36-5
    UCA1BR0 = 8;		//lower 8 bits
    UCA1BR1 = 0;			//upper 8 bits
    UCA1MCTL = UCBRS_0 | UCBRF_11 | UCOS16;
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

