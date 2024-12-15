#include <msp430.h> 
#include <stdint.h>
#define NUM_OF_SAMPLE (10)
#define BUTTON_POLLING_PERIOD        (1048)  /* ~32ms (31.25ms) */

volatile uint8_t stop_flag = 0; // flag to tell the main program to stop
volatile uint8_t tmp = 0;
volatile uint8_t block = 0; // additional logic value to block SW1 (LOWER SWITCH)

/**
 *  variables who assist us in knowing what state we are currently in
 *  note this can be don with fewer variables but with these names it makes it easer to keep track what is happening
*/
volatile uint8_t send_channel_1 = 0; // is set to 1 when we are ready to send the data, 0 when we are not
volatile uint8_t send_channel_2 = 0;
volatile uint8_t sample_channel_1 = 0; // is set to 1 when sampling the current channel, 0 when we are finished
volatile uint8_t sample_channel_2 = 0;

volatile uint8_t cnt = 0; // current sample as counter
volatile uint8_t data_channel_1[NUM_OF_SAMPLE] = {0}; // data buffer for channel 1 in witch we store the samples
volatile uint8_t data_channel_2[NUM_OF_SAMPLE] = {0}; // // data buffer for channel 2 in witch we store the samples
volatile char str_ch_1[12] = {'P', 'r', 'v', 'i', ' ', 'k', 'a', 'n', 'a', 'l', ':', ' '}; // Additional message to display for channel 1 (BLACK)
volatile char str_ch_2[13] = {'D', 'r', 'u', 'g', 'i', ' ', 'k', 'a', 'n', 'a', 'l', ':', ' '}; // Additional message to display for channel 2 (RED)


/**
 * We use ADC ISR to read conversion results of both channels
 */
int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;   // Stop watchdog timer

    // Initialize UART
    P4SEL |= BIT4 | BIT5;       // enable P3.4 and P3.5 for UART

    UCA1CTL1 |= UCSWRST;        // set software reset

    UCA1CTL0 = 0;
    UCA1CTL1 |= UCSSEL__SMCLK ; // use SMCLK
    UCA1BRW = 109;              // BR = 109
    UCA1MCTL = UCBRS_2;         // BRS = 2 for 9600 bps

    UCA1CTL1 &= ~UCSWRST;       // release software reset

    UCA1IFG = 0;                // clear IFG

    /***************************************************/

    // OVO JE UZETO SA VEZBI
    // initialize ADC
    P6SEL |= BIT0;              // set P6.0 for ADC
    P6SEL |= BIT1;              // set P6.1 for ADC
    ADC12CTL0 = ADC12ON;        // turn on ADC
    ADC12CTL1 = ADC12SHS_1 | ADC12CONSEQ_3; // set SHS = 1 (TA0.0 used for SAMPCON) and repeat-single-channel mode
    ADC12MCTL0 = ADC12INCH_1;               // select channel 1
    ADC12MCTL1 = ADC12INCH_0 | ADC12EOS;    // select channel 0 and
    ADC12CTL0 |= ADC12ENC;                  // enable conversion

    /*************************************************/

    P2DIR |= BIT4;              // set P2.4 as out, BLACK
    P2DIR |= BIT5;              // set P2.5 as out, RED
    P2OUT &= ~(BIT4 | BIT5);             // turn off bouth LEDs
    P1DIR &= ~(BIT4 | BIT5);             // set P1.4 as in
    P1REN |= (BIT4 | BIT5);              // This is important because there is no PullUp Resistor on the board
    P1OUT |= (BIT4 | BIT5);              // This is important because there is no PullUp Resistor on the board

    /* initialize Timer A0 for sampling 3 samples per second*/
    TA0CCR0 = 10912 / 2;                    // period is 0.333s
    TA0CCTL1 = OUTMOD_3;                    // use set/reset mode
    TA0CCR1 = 5456 / 2;
    TA0CTL = TASSEL__ACLK | MC__UP;         // ACLK source, UP mode

    /* initialize Timer A1 for button debouncing*/
    TA1CCR0     = BUTTON_POLLING_PERIOD;
    TA1CCTL0    = CCIE;            // enable CCR0 interrupt
    TA1CTL      = TASSEL__ACLK | MC__UP;

    __enable_interrupt();                   // GIE

    while (1){
        int i = 0; // additional variable for all the *for* loops
        if(send_channel_1 == 1){
            for(i = 0; i < 12; i++){ // sending the additional message telling the PC it`s from channel 1 (BLACK)
                UCA1TXBUF = str_ch_1[i];
                while((UCA1IFG & UCTXIFG) == 0);    // wait until byte is sent
            }
            for(i = 0; i < NUM_OF_SAMPLE; i++){ // sending the result of sampling the channel
                UCA1TXBUF = data_channel_1[i];
                while((UCA1IFG & UCTXIFG) == 0);    // wait until byte is sent
            }
            UCA1TXBUF = '\n';
            while((UCA1IFG & UCTXIFG) == 0);    // wait until byte is sent
            UCA1TXBUF = '\r';
            while((UCA1IFG & UCTXIFG) == 0);    // wait until byte is sent
            send_channel_1 = 0;
            while(sample_channel_2 == 1); // waiting for sampling on channel 2
            // we are done sapling the previus channel now we can sample the other one under the condition that stop_flag is not set
            //by the other switch (SW2)
            if(stop_flag == 0) sample_channel_1 = 1;
        }
        if(send_channel_2 == 1){
            for(i = 0; i < 13; i++){ // sending the additional message telling the PC it`s from channel 2 (RED)
                UCA1TXBUF = str_ch_2[i];
                while((UCA1IFG & UCTXIFG) == 0);    // wait until byte is sent
            }
            for(i = 0; i < NUM_OF_SAMPLE; i++){ // sending the result of sampling the channel
                UCA1TXBUF = data_channel_2[i];
                while((UCA1IFG & UCTXIFG) == 0);    // wait until byte is sent
            }
            UCA1TXBUF = '\n';
            while((UCA1IFG & UCTXIFG) == 0);    // wait until byte is sent
            UCA1TXBUF = '\r';
            while((UCA1IFG & UCTXIFG) == 0);    // wait until byte is sent
            send_channel_2 = 0;                 // we are done with sending
            while(sample_channel_1 == 1);       // waiting for sampling on channel 1
            // we are done sapling the previus channel now we can sample the other one under the condition that stop_flag is not set
            if(stop_flag == 0) sample_channel_2 = 1;
        }
        if(stop_flag == 1){
           while(sample_channel_1 == 1); // waiting for sampling on channel 1
           while(sample_channel_2 == 1); // waiting for sampling on channel 2
           if(send_channel_1 == 1){
               for(i = 0; i < 12; i++){ // sending the additional message telling the PC it`s from channel 1 (BLACK)
                   UCA1TXBUF = str_ch_1[i];
                   while((UCA1IFG & UCTXIFG) == 0);    // wait until byte is sent
               }
               for(i = 0; i < NUM_OF_SAMPLE; i++){ //// sending the data that was sampled from channel 1 (BLACK)
                   UCA1TXBUF = data_channel_1[i];
                   while((UCA1IFG & UCTXIFG) == 0);    // wait until byte is sent
           }
           UCA1TXBUF = '\n';
           while((UCA1IFG & UCTXIFG) == 0);    // wait until byte is sent
           UCA1TXBUF = '\r';
           while((UCA1IFG & UCTXIFG) == 0);    // wait until byte is sent
       }
       else if(send_channel_2 == 1){
           for(i = 0; i < 13; i++){ // sending the additional message telling the PC it`s from channel 2 (RED)
               UCA1TXBUF = str_ch_2[i];
               while((UCA1IFG & UCTXIFG) == 0);    // wait until byte is sent
               }
           for(i = 0; i < NUM_OF_SAMPLE; i++){ // sending the data that was sampled from channel 2 (RED)
               UCA1TXBUF = data_channel_2[i];
               while((UCA1IFG & UCTXIFG) == 0);    // wait until byte is sent
           }
           UCA1TXBUF = '\n';
           while((UCA1IFG & UCTXIFG) == 0);    // wait until byte is sent
           UCA1TXBUF = '\r';
           while((UCA1IFG & UCTXIFG) == 0);    // wait until byte is sent
       }
       // nothing to send so we set these values to 0
       send_channel_1 = 0;
       send_channel_2 = 0;
       ADC12IE &= ~(ADC12IE0 | ADC12IE1);  // DISABLE interrupt for MEM0
       block = 0;                          // SW1 is no longer blocked
       stop_flag = 0;                      // reset stop_flag
       }
    }
}
/**
 * ADC ISR
 */
void __attribute__ ((interrupt(ADC12_VECTOR))) ADC12ISR (void)
{
    switch(__even_in_range(ADC12IV,34))
    {
    case  0: break;                           // Vector  0:  No interrupt
    case  2: break;                           // Vector  2:  ADC overflow
    case  4: break;                           // Vector  4:  ADC timing overflow
    case  6:                                  // Vector  6:  ADC12IFG0
        if(sample_channel_1 == 1){                                  // as long as cnt is below 10 we sample this channel
            if(cnt < 10){                                           //// as long as cnt is below 10 we sample this channel
                data_channel_1[cnt] = (ADC12MEM0 >> 4 & 0xff);      // take 8 MSB and store them in the corresponding buffer
                cnt++;                                              // increment current sample
                P2OUT |= BIT4;                                      // turn on LED for channel 1 (BLUE DIODE)
                P2OUT &= ~BIT5;
            } else{
                cnt = 0;
                sample_channel_1 = 0;                       // tell the program that we are done sampling channel 1
                if(stop_flag == 0) sample_channel_2 = 1;    // sample next channel if stop_flag is not set
                send_channel_1 = 1;                         // we can send what we have sampled
                P2OUT &= ~BIT4;
                tmp = ADC12MEM0 >> 4;                       // for removing the interrupt flag
            }
        }
        else{
            tmp = ADC12MEM0 >> 4;   // for removing the interrupt flag
        }
        break;
    case  8:                                  // Vector  8:  ADC12IFG1
        if(sample_channel_2 == 1){
            if(cnt < 10){ // as long as cnt is below 10 we sample this channel
                data_channel_2[cnt] = (ADC12MEM1 >> 4 & 0xff);
                cnt++;
                P2OUT |= BIT5;
                P2OUT &= ~BIT4;
            } else{
                cnt = 0;                                 // reset for next sampling
                if(stop_flag == 0) sample_channel_1 = 1; // sample next channel if stop_flag is not set
                sample_channel_2 = 0;                    // tell the program that we are done sampling channel 2
                send_channel_2 = 1;                      // and that we can send what we have sampled
                P2OUT &= ~BIT5;
                tmp = ADC12MEM1 >> 4;                    // for removing the interrupt flag
            }
        }
        else{
            tmp = ADC12MEM1 >> 4;   // for removing the interrupt flag
        }
        break;
    case 10: break;                           // Vector 10:  ADC12IFG2
    case 12: break;                           // Vector 12:  ADC12IFG3
    case 14: break;                           // Vector 14:  ADC12IFG4
    case 16: break;                           // Vector 16:  ADC12IFG5
    case 18: break;                           // Vector 18:  ADC12IFG6
    case 20: break;                           // Vector 20:  ADC12IFG7
    case 22: break;                           // Vector 22:  ADC12IFG8
    case 24: break;                           // Vector 24:  ADC12IFG9
    case 26: break;                           // Vector 26:  ADC12IFG10
    case 28: break;                           // Vector 28:  ADC12IFG11
    case 30: break;                           // Vector 30:  ADC12IFG12
    case 32: break;                           // Vector 32:  ADC12IFG13
    case 34: break;                           // Vector 34:  ADC12IFG14
    default: break;
    }
}

// ISR of timer A0 for polling SW1 and SW2
void __attribute__ ((interrupt(TIMER1_A0_VECTOR))) TAIEISR (void)
{
    static uint8_t P14IN_old = 0;
    static uint8_t P15IN_old = 0;

    if (((P14IN_old & BIT4) != 0) && ((P1IN & BIT4) == 0) && block == 0)
    {
        // SW1 pressed
        sample_channel_1 = 1;           // start samling the firs channel
        block = 1;                      // block SW1
        ADC12IE |= ADC12IE0 | ADC12IE1; // enable interrupt for MEM0
    }
    P14IN_old = P1IN;
    if (((P15IN_old & BIT5) != 0) && ((P1IN & BIT5) == 0))
    {
        // SW2 pressed
       stop_flag = 1;
    }
    P15IN_old = P1IN;
    return;
}

