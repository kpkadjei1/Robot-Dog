#include <msp430.h> 


/**
 * main.c
 */

int main(void)
{
    volatile int pwm1val, pwm2val, adval, prev; //declare variables as volatile
    unsigned int i;

    WDTCTL = WDTPW | WDTHOLD; // stop watchdog timer

    //Initialize ports
    P2DIR = BIT1 | BIT2 | BIT3| BIT4; // P2.1,2,3,4 output (H-bridge control)
    P2OUT = BIT5 | BIT7; // 2.5,2.7 hi (pullup)
    P2REN |= BIT5 | BIT7; //P2.5,2.7 pullup

    //***********Start code, set to 1 MHz (from examples)*
    DCOCTL = 0x00;
    BCSCTL1 = CALBC1_1MHZ; /* Set DCO to 1MHz */
    DCOCTL = CALDCO_1MHZ;
    /* Basic Clock System Control 1
    * XT2OFF -- Disable XT2CLK
    * ~XTS -- Low Frequency
    * DIVA_0 -- Divide by 1 */
    BCSCTL1 |= XT2OFF + DIVA_3;
    /* Basic Clock System Control 3
    * XT2S_0 -- 0.4 - 1 MHz
    * LFXT1S_2 -- If XTS = 0, XT1 = VLOCLK ; If XTS = 1, XT1 = 3 - 16-MHz crystal or resonator
    * XCAP_1 -- ~6 pF */
    BCSCTL3 = XT2S_0 + LFXT1S_2 + XCAP_1;
    // ***********End code, set to 1 MHz*************

    P1DIR = 0x11; //set P1.0, 1.4 output-> 0001 0001 in binary
    P2DIR = 0x54; //set P2.2, 2.4, 2.5 output-> 0011 0100 in binary
    P1SEL = 0x10; //use PxSEL to use peripheral module function of
                  //P1.4, which is SMCLK
    P2SEL = 0x14; //use PxSEL to use peripheral module function
                  //P2.2, P2.4 PWM outputs
    P2SEL2 = 0x00; //clearing 2nd nibble to get TA1.1,TA1.2
    P1OUT = 0x01; // Blink LED (Sanity check)

    for (i=0; i<50000; i++) ; //delay loop
    P1OUT = 0x00; // turn off LED
    // divide the divider for SMCLK, frequency is now 15kHz
    BCSCTL2 |= 0x04; // set bit 2
    // BCSCTL2 &= ~0x02; // clear bit 1
    TA1CTL = TASSEL_2 + MC_1; //set timer to SMCLK and up mode
    TA1CCTL1 = OUTMOD_7; //set timer TA1.1 to reset/set
    TA1CCTL2 = OUTMOD_7; //set timer TA1.2 to reset/set

    // Setup A/D
    ADC10CTL1 = INCH_5; //Set channel 3 to analog input
    ADC10CTL0 = ADC10ON | ENC; //Turn ADC on, enable conversions

    // Setup PWM
    // since timer is in up mode, the timer will count until TA1CCR0, as // seen in figure 11
    TA1CCR0 = 16-1;

    // PWM val is 0 to 16, 8 is 50%
    pwm1val = 3;
    pwm2val = 5;

    while (1) {
        for (i=0; i<50000; i++) ; //delay loop
        TA1CCR1 = pwm1val;
        TA1CCR2 = pwm2val; // initialize pwm values
        ADC10CTL0 |= ADC10SC; // Start conversion

        while (ADC10CTL1 & ADC10BUSY); // Wait for conversion to finish
        adval = ADC10MEM; // Get A/D conversion result

        if (prev < adval) {
            P1OUT |= BIT0;
            if(pwm1val == 3) {
                pwm1val = 4;
                pwm2val = 3;
            }else {
                pwm1val = 3;
                pwm2val = 5;
              }
        else {
            P1OUT &= ~BIT0;
        }
        prev = adval;
    }

}
