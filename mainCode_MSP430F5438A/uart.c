#include <msp430.h>

//UART Setup//
void initUART(void)
{
    //Select UART pins (double check)
    P3SEL |= BIT4 | BIT5;		// P3.4 = TX and P3.5 = RX
    UCA0CTL0 = UCPEN + UCPAR;		//even parity, 8 data bits, 1 stop bit
    UCA0CTL1 |= UCSWRST;		//Hold USCI in reset while configuring
    UCA0CTL1 |= UCSSEL_2;		//SMCLK as source
    //Divider for Clock:
    //Divider = Clk / baud rate = 8 MHz / 460.8 kHz = 17.36
    UCA0BR0 = 17	;		//lower 8 bits
    UCA0BR1 = 0;			//upper 8 bits
    UCA0MCTL = UCBRS_3;			//oversampling off, reccommended values from table 36-4

    UCA0CTL1 &= ~UCSWRST; 		//Enable UART
}
*/


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

