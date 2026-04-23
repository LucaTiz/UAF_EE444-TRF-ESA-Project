#include <msp430.h>
#include <stdint.h>

void initClock(void); 	//System clock setup
void initUART(void); 	//UART for PC comms setup
void initSPI(void); 	//SPI setup for TDC7201 comms
void initTimer(void);   //timer setup for UART transmission
//UART output
void uartSendChar(char c);
void uartSendString(const char *str);

void spiWrite(unsigned char data, unsigned char address, uint8_t CS); 
void spiRead(unsigned char address, uint8_t CS);

uint32_t spiRead3(unsigned char address, unsigned int CS);
void TDCConfig(void);

float calculateTime (uint32_t raw, unsigned int CS); 

//note: SPI sends and receives simultaneously

volatile int tmp = 0; //temporary variable
volatile int i = 0; //temporary iteration variable
volatile int TDC1 = BIT5;
volatile int TDC2 = BIT6;
//volatile uint8_t  read;
//volatile uint32_t read3;
volatile uint32_t rawTDC1;
volatile uint32_t rawTDC2;
volatile unsigned char dummy;
volatile unsigned char read;
float time;
const int numBins = 5; //number of bins must match number of labels
volatile int bins[5];
volatile const char *labels[] =
{
  "label1",
  "label2",
  "label3",
  "label4",
  "label5"
};

int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;	//Disable watchdog
   
////////////// pin configuration ///////////////////
    P3SEL |= BIT1 | BIT2 | BIT3;	// P3.1 - UCB0SIMO, P3.2 - UCB0SOMI, P3.3 - UCB0CLK, UCB0STE
   /// P3DIR |= BIT1 | BIT3;	//STE, MOSI, and CLK pins as output
   /// P3DIR &= ~BIT2;			//SOMI as input
    P3REN &= ~BIT2; // Ensure internal resistor is OFF for SOMI

    P1DIR |= BIT3; // ENABLE1 pin
    P1OUT |= BIT3; // set ENABLE1 high 

    P1DIR |= BIT7; // OSC_ENABLE pin (TDC8MHz)
    P1OUT |= BIT7; // set OSC_ENABLE high 

    P1DIR |= BIT5; // CSB1 control 
    P1OUT |= BIT5; // set CSB1 (TDC1) high - not currently in a transaction 
  
    P1DIR |= BIT6; // CSB2 control 
    P1OUT |= BIT6; // set CSB2 (TDC2) high - not currently in a transaction 
    
    
    P1DIR &= ~BIT2; // interrupt pin for measurement ready  TDC1      
    P1IE |= BIT2; // on INTB1 to interrupt
    P1IES |= BIT2; // sets P21.2 interrupt to sensitive on fallng edge (button pressed in)
 //   P1REN |= BIT2; // enables internal resistor on P1.2.
 //   P1OUT |= BIT2; // configures resistor to pull-up on P1.2.

    P1DIR &= ~BIT4; // interrupt pin for measurement ready  TDC2                                                                   
    P1IE |= BIT4; // on INTB2 to interrupt
    P1IES |= BIT4; // sets P21.2 interrupt to sensitive on fallng edge (button pressed in)
 //   P1REN |= BIT4; // enables internal resistor on P1.4.
 //   P1OUT |= BIT4; // configures resistor to pull-up on P1.4.
   
    initClock();	//Setup system timing; do first
    initSPI();		//Initialize SPI after clocks
    
    
    __delay_cycles(8000);
    TDCConfig(); //configure and start measuring.

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
    __delay_cycles(2500);		//Time to allow clock stabilization
    UCSCTL4 = SELA__XT1CLK + SELS__DCOCLK + SELM__DCOCLK;		//ACLK = XT1 SMCLK = 
}

void initTimer(void)
{
  TA0CTL = TACLR | TASSEL__ACLK | MC__UP | ID_3; //clear previous settings, clock source ACLK, clock divider /8, UP mode, Enable mode control
  TA0CCTL0 = CCIE; //enable timer interrupt
  TA0CCR0 = 5 * 32767 / 8; //5 seconds of ACLK
}

// SPI Setup //
void initSPI(void)
{

    UCB0CTL1 |= UCSWRST; 		//Hold in reset
    UCB0CTL0 = UCCKPH | UCMSB | UCMST | UCSYNC | UCMODE_0; //SPI master, MSB first, 3 wire config 
    UCB0CTL1 = UCSSEL_2;      //SMCLK
    UCB0BR0 = 80;			//SPI clock divider
    UCB0BR1 = 0;			//Upper divider byte
    UCB0CTL1 &= ~UCSWRST;		//Release reset and enable SPI
    UCB0IE |= UCRXIE;                   //enable interrupt
}




//write data to a 6-bit address, and recieve the data back as confirmation
void spiWrite(unsigned char data, unsigned char address, uint8_t CS)
{
    while (!(UCB0IFG & UCTXIFG));	//Wait until TX buffer is empty
    P1OUT &= ~(CS); //
    UCB0TXBUF =  (0b01000000 | address);//write command from TDC datasheet - initiates write to address (TDCCONFIG1 at 0x00)
    while (!(UCB0IFG & UCTXIFG));
    UCB0TXBUF = data; // writes data (5 to TDCCONFIG1)
    while (!(UCB0IFG & UCTXIFG));
    while (UCB0STAT & UCBUSY);
    P1OUT |= CS;

}




//read data from a 8-bit address (contrl registers)
void spiRead(unsigned char address, uint8_t CS)
{
    P1OUT &= ~(CS);

    while (!(UCB0IFG & UCTXIFG));	//Wait until TX buffer is empty
    UCB0TXBUF =  (0b00000000 | address); // Read command (Bit 6 clear)
    while (UCB0STAT & UCBUSY);
    dummy = UCB0RXBUF;            // Clear the RX buffer from the first shift

    UCB0TXBUF = 0x00;             // Send dummy to clock in data
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
    
    return result; ///supposedly ~~10 us in total.

}

// TDC Configuration. Write 8 bit values to configuration registers

void TDCConfig(void){

spiWrite(0b10000001,0x00,TDC1);

spiRead(0x00, TDC1);

//spiWrite(0b10000001,0x00,TDC2);
spiWrite(0b11000000,0x01,TDC1);
spiRead(0x01, TDC1);
//spiWrite(0b11000000,0x01,TDC2);
spiWrite(0b11111111,0x04,TDC1);
spiRead(0x04, TDC1);
//spiWrite(0b11111111,0x04,TDC2);
spiWrite(0b11111111,0x05,TDC1);
spiRead(0x05, TDC1);
//spiWrite(0b11111111,0x05,TDC2);
spiWrite(0b00000111,0x03,TDC1);
spiRead(0x03, TDC1);
//spiWrite(0b00000111,0x03,TDC2);

///test ISR 

P1IFG = (BIT2 + BIT4);

}



float calculateTime (uint32_t raw, unsigned int CS) {
   uint32_t calCount = (spiRead3(0x1C,CS) - spiRead3(0x1B,CS)) / (40 - 1); // 40 periods (SUBJECT TO CHANGE)
   uint32_t normLSB = (1/8000000) / (calCount);
   return (raw * normLSB);
}

//void calculateAngletoBUF (float time) {


//}


void TDCreadreadyISR(void) __interrupt [PORT1_VECTOR] {
while (P1IFG != (BIT2 + BIT4));
   rawTDC1 = spiRead3(0x10, TDC1); // reads time 1 from TDC1 
   rawTDC2 = spiRead3(0x10, TDC2); // reads time 1 from TDC2

   ////calculate ....////
   time = calculateTime(rawTDC2, TDC2) - calculateTime(rawTDC1, TDC1);
   
   /// binnn based on angle "" ///
   //calculateAngletoBUF ()

   //////////////////////

   ///start measurement again; 
 
  spiWrite(0b10000001,0x00,TDC1);
  spiWrite(0b10000001,0x00,TDC2);
   ///// clears ISR queue while there is some interrupt present.
   do {
    P1IFG = 0;
   }  while (P1IFG != 0); 
}

//timer interrupt - time to transmit and reset the bins
void Timer(void) __interrupt [TIMER0_A0_VECTOR]
{
    uartSendString("{\""); //message start
    uartSendString(labels[0]); //first label
    for(i = 1; i <= numBins; i++)
    {
      
    }
}