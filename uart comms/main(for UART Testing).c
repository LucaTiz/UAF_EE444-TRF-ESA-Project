#include <msp430.h>
#include "uart.h"

void initClock(void);   //System clock setup

int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;    //Disable watchdog

    initClock();    //Setup system timing; do first
    initUART();     //Initialize UART

    uartSendString("UART initialized\r\n");

    // Global interrupts not required for polling UART transmit
    // _EINT();

    // LPM0;    // Commented out during UART testing so CPU remains active for debugging and serial output verification

    while(1)
    {
        uartSendChar('A');
        uartSendChar('\r');
        uartSendChar('\n');
        __delay_cycles(3 * 8000000);
    }
}

//Clock Setup//
void initClock(void)
{
    //REFO for reference, configuring DCO to 8MHz
    UCSCTL3 = SELREF__REFOCLK;          //FLL reference = REFO
    UCSCTL4 = SELA__REFOCLK + SELS_2;   //ACLK = REFO, SMCLK = DCO

    __bis_SR_register(SCG0);            //Disable FLL
    UCSCTL0 = 0x0000;                   //Sets DCO register to lowest default values
    UCSCTL1 = DCORSEL_5;                //Set frequency range to support ~8Mhz
    UCSCTL2 = FLLD_0 | 243;             //FLLD_0 = divider 1
                                        //f_DCO = (N+1) * f_ref
                                        //8MHz / 32768Hz = 244, so N = 243

    __bic_SR_register(SCG0);            //Re-enable FLL
    __delay_cycles(25000);              //Time to allow clock stabilization
}