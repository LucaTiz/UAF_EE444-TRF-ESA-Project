#include <msp430.h>
#include <stdint.h>

void initClock(void); 	//System clock setup
void initUART(void); 	//UART for PC comms setup
void initSPI(void); 	//SPI setup for TDC7201 comms
//UART output
void uartSendChar(char c);
void uartSendString(const char *str);

void spiWrite(unsigned char data, unsigned char address, unsigned int CS); 
uint8_t spiRead(unsigned char address, unsigned int CS);

uint32_t spiRead3(unsigned char address, unsigned int CS);

int calculateTime (void);

//note: SPI sends and receives simultaneously

volatile int tmp = 0;
volatile int TDC1 = BIT3;
volatile int TDC2 = BIT5;
volatile uint8_t  read;
volatile uint32_t read3;

int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;	//Disable watchdog


    initClock();	//Setup system timing; do first
 //   initUART();		//Initialize UART
    initSPI();		//Initialize SPI after clocks
    spiWrite(0x05, 0x00, TDC1);
//    spiConfig();
    read = spiRead(0x00, TDC1);
    read3 = spiRead3(0x10, TDC1);
    _EINT(); 
    LPM0;
}



//Clock Setup//
void initClock(void)
{
    //REFO for reference, configuring DCO to 8MHz
    UCSCTL3 = SELREF__XT1CLK; 		//FLL reference = XT1

    __bis_SR_register(SCG0);		//Disable FLL
    UCSCTL0 = 0x0000;			//Sets DCO register to lowest default values
    UCSCTL1 = DCORSEL_5; 		//Set frequency range to support ~8Mhz
    UCSCTL2 = FLLD_0 | 243;		//FLLD_0 = divider 1
					//f_DCO = (N+1) * f_ref
					//8MHz / 32768Hz = 244, so N = 243

    __bic_SR_register(SCG0);		//Re-enable FLL
    __delay_cycles(25000);		//Time to allow clock stabilization
    UCSCTL4 = SELA__XT1CLK + SELS__DCOCLK + SELM__DCOCLK;		//ACLK = XT1 SMCLK = 
}

// SPI Setup //
void initSPI(void)
{
    P3SEL |= BIT1 | BIT2 | BIT3;	// P3.1 - UCB0SIMO, P3.2 - UCB0SOMI, P3.3 - UCB0CLK, UCB0STE
    P3DIR |= BIT1 | BIT3;	//STE, MOSI, and CLK pins as output
    P3DIR &= ~BIT2;			//SOMI as input
    P1DIR |= BIT3; // CSB1 control 
    P1DIR |= BIT5; // CSB2 control 
    P1OUT |= BIT3; // set CSB high - not currently in a transaction 
    P1DIR |= BIT6; // enable pin
    P1OUT |= BIT6; // set ENABLE high 
       
    UCB0CTL1 |= UCSWRST; 		//Hold in reset
    UCB0CTL0 = UCCKPH | UCMSB | UCMST | UCSYNC | UCMODE_0; //SPI master, MSB first, 3 wire config 
    UCB0CTL1 = UCSSEL_2;      //SMCLK
    UCB0BR0 = 8;			//SPI clock divider
    UCB0BR1 = 0;			//Upper divider byte
    UCB0CTL1 &= ~UCSWRST;		//Release reset and enable SPI
    UCB0IE |= UCRXIE;                   //enable interrupt
}

//write data to a 6-bit address, and recieve the data back as confirmation
void spiWrite(unsigned char data, unsigned char address, unsigned int CS)
{
    while (!(UCB0IFG & UCTXIFG));	//Wait until TX buffer is empty
    P1OUT &= ~CS; // either 5 or 3, fix later 
    UCB0TXBUF =  (0b01000000 | address);//write command from TDC datasheet - initiates write to address (TDCCONFIG1 at 0x00)
    while (!(UCB0IFG & UCTXIFG));
    UCB0TXBUF = data; // writes data (5 to TDCCONFIG1)
    while (!(UCB0IFG & UCTXIFG));
    while (UCB0STAT & UCBUSY);
    P1OUT |= CS;
  //  UCB0TXBUF =  (0x00 | address);//read command to dout on next sclk
  //  while (!(UCB0IFG & UCTXIFG));
  //  P1OUT = 1;
}




//read data from a 8-bit address (contrl registers)
uint8_t spiRead(unsigned char address, unsigned int CS)
{
    unsigned char dummy;
    while (!(UCB0IFG & UCTXIFG));	//Wait until TX buffer is empty
    P1OUT &= ~CS;
    UCB0TXBUF =  (0b00000000 | address); // Read command (Bit 6 clear)
    while (UCB0STAT & UCBUSY);
    dummy = UCB0RXBUF;            // Clear the RX buffer from the first shift

    UCB0TXBUF = 0x00;             // Send dummy data to clock in data
    while (!(UCB0IFG & UCRXIFG));
    while (UCB0STAT & UCBUSY);
    read = UCB0RXBUF;          // Capture actual data
    
    P1OUT |= CS;              // CS High
}


uint32_t spiRead3(unsigned char address, unsigned int CS)
{
    uint8_t data[3];
    uint32_t result = 0;

    char dummy1;
    while (!(UCB0IFG & UCTXIFG));	//Wait until TX buffer is empty
    P1OUT &= ~CS;
    UCB0TXBUF =  (0b00000000 | address); // Read command (Bit 6 clear)
   
    while (UCB0STAT & UCBUSY);
    dummy1 = UCB0RXBUF;            // Clear the RX buffer from the first shift
    UCB0TXBUF = 0x00;             // Send dummy to clock in data
    while (!(UCB0IFG & UCRXIFG));
    while (UCB0STAT & UCBUSY);
    data[0] = UCB0RXBUF;          // Capture actual data
    
    while (UCB0STAT & UCBUSY);
    dummy1 = UCB0RXBUF;            // Clear the RX buffer from the first shift
    UCB0TXBUF = 0x00;             // Send dummy to clock in data
    while (!(UCB0IFG & UCRXIFG));
    while (UCB0STAT & UCBUSY);
    data[1] = UCB0RXBUF;          // Capture actual data

    while (UCB0STAT & UCBUSY);
    dummy1 = UCB0RXBUF;            // Clear the RX buffer from the first shift
    UCB0TXBUF = 0x00;             // Send dummy to clock in data
    while (!(UCB0IFG & UCRXIFG));
    while (UCB0STAT & UCBUSY);
    data[2] = UCB0RXBUF;          // Capture actual data

    P1OUT |= CS;              // CS High

    result |= ((uint32_t)data[0] << 16) | ((uint32_t)data[1] << 8) | ((uint32_t)data[2]);
    
    return result;

}

/*
// UCA ISR
void RX(void) __interrupt [USCI_B0_VECTOR]
{
  if(UCB0IFG & UCRXIFG) //Check RX bit of interrupt flag register
  {
    tmp = UCB0RXBUF; //record recieved character
  }
}
*/

/*
//UART Setup//
void initUART(void)
{
    //Select UART pins (double check)
    P3SEL |= BIT4 | BIT5;		// P3.4 = TX and P3.5 = RX

    UCA0CTL1 |= UCSWRST;		//Hold USCI in reset while configuring
    UCA0CTL1 |= UCSSEL_2;		//SMCLK as source

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

/*
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
*/
