#include <msp430.h>

//UART Setup//
void initUART(void)
{
    //Select UART pins (double check)
    P3SEL |= BIT4 | BIT5;		// P3.4 = TX and P3.5 = RX

    UCA0CTL1 |= UCSWRST;		//Hold USCI in reset while configuring
    UCA0CTL1 |= UCSSEL_2;		//SMCLK as source
    UCOS16 = 1;
    //table 

    //Divider for Clock:
    //Divider = Clk / baud rate = 8e6 / 9600 = 833.333
    //BR0/BR1 = 833 
    //note: MSP430 can not store 833 in single register, max value per register is 255
    //Values are stored in two registers instead

    UCA0BR0 = 833 & 0xFF;		//lower 8 bits (65)
    UCA0BR1 = (833 >> 8);		//upper 8 bits (3 * 2^8)
    UCA0MCTL = UCBRS_6;			//Approx modulation

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

