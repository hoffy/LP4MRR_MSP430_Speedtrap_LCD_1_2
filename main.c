/*
* IR Beam Detected Speed Trap (Single Track Version) Version 1.2 LCD Edition
*
* Provided under a Creative Commons Attribution, Non-Commercial Share Alike,3.0 Unported License
*
* COPYRIGHT 2015 - S.D. "Hoffy" Hofmeister, et al
*************************************************************************************************
* CREDITS:
*
* Speed calculations & initial concept coding by: Toni Ryan
*
* IR Beam Detection & LCD Integration by: S.D. "Hoffy" Hofmeister
*
* LCD Library (lcdLib.h & lcdLib.c) The University of Texas at El Paso -
* College of Engineering: http://www.ece.utep.edu/courses/web3376/Lab_5_-_LCD.html
* Elliott Gurrola, Luis Carlos Bañuelos-Chacon, Elias N Jaquez
*
* This code is a group source project through The Launchpad for Model Railroading Project
* - http://launchpad4mrr.blogspot.com/
*
**************************************************************************************************************
* TARGETED TO MSP430 LANUCHPAD W/MSP430G2553 PROCESSOR
*
* Design Notes:
*
* This code is designed to to trigger a speed calculation for a model train passing through 2 IR beam Sensors.
* The code is designed to take a "speed reading" or "Time Count" in both directions.
*
* Distance between Tip of IR Emitter and Tip of Detector has only been tested up to 3.5 Inches under
* incandescent and fluorescent lighting conditions with no failures.
*
* LCD Coding is designed for a QC1602A Ver 2.0 LCD Module
*
* Circuit Pinout:
* PIN 1.0 = RED INDICATOR LED
* PIN 1.1 = UNASSIGNED - UART
* PIN 1.2 = UNASSIGNED - UART
* PIN 1.3 = UNASSIGNED
* PIN 1.4 = Cathode of IR Beam Receiver #1	> Anode to Ground
* PIN 1.5 = Cathode of IR Beam #2	> Anode to Ground
* PIN 1.6 = GREEN INDICATOR LED
* PIN 1.7 = UNASSIGNED
* PIN 2.0 = LCD D4
* PIN 2.1 = LCD D5
* PIN 2.2 = LCD D6
* PIN 2.3 = LCD D7
* PIN 2.4 = E (Enable)
* PIN 2.5 = RS (Register Signal)
*
* PINS 1.1, 1.2, 1.5, 1.7 are left open for integration into other projects
*
* Note Anodes for the IR Emitters connect to VCC and Cathodes to Ground
*/
//*****************************************************************************************************
#include <msp430g2553.h>
#include "lcdLib.h"
// Declare functions
void delay ( unsigned int );	// delay for xx 10ms increments
// Declare variables
float scale = 160; 	// denominator of scale (N scale 1:160)
float gate = 12; 	// timing gate spacing (in inches)
float speed = 0;	// speed in MPH
long unsigned int factor = 109090;	// number of ms to travel "gate" inches at 1 MPH for "scale" scale (1 MPH slowest speed we can display)
long unsigned int counter = 0;	// millisecond counter
int counter_active = 0; // flag to increment counters or stop counting
int output_count = 0;	// flag to display time
int lockgate_1 = 0;  // Locks Gate #1 from interrupting until unlocked
int lockgate_2 = 0; // Locks Gate #2 from interrupting until unlocked
void main(void)
{
  WDTCTL = WDTPW + WDTHOLD;                 // Stop WDT
  BCSCTL1 = CALBC1_1MHZ; // Set range
  DCOCTL = CALDCO_1MHZ; // Set DCO step + modulation
  CCTL0 = CCIE;                             // CCR0 interrupt enabled
  TACTL = TASSEL_2 + MC_1 + ID_3;           // SMCLK/8, upmode
  CCR0 =  125; 								// 1000 Hz = 1 ms clock

  P1DIR |= BIT0;					// Port 1 P1.6 (Indicator #1) as output
  P1OUT &= ~BIT0;					// Port 1 P1.6 (Indicator #1) Set to off State
  P1DIR |= BIT6;					// Port 1 P1.7 (Indicator #1) as output
  P1OUT &= ~BIT6;					// Port 1 P1.7 (Indicator #1) Set to off State

  P1DIR |= 0x00;
  P1OUT &= 0x00;                        	// Shut. Down. Everything... :)

  P1REN |= BIT4;							// Port 1 Resistor enable
  P1OUT |= BIT4;							// pull up bit3
  P1REN |= BIT5;							// Port 1 Resistor enable
  P1OUT |= BIT5;							// pull up bit3
  __enable_interrupt();

lcdInit();// Initialize LCD

//Credits on LCD
lcdSetText("Speed Trap 1trak", 0, 0);
lcdSetText("Version 1.2 LCD", 0,1);
//     ---->________________<---- Maximum 16 Character String Length
delay(200);
lcdClear();
lcdSetText("CC Licensed", 2, 0);
lcdSetText("Copyright 2015 ", 1,1);
delay(200);
lcdClear();
lcdSetText("S.D. Hofmeister,", 0, 0);
lcdSetText("et al ", 5,1);
delay(200);
lcdClear();
lcdSetText("The Launchpad", 1, 0);
lcdSetText("for", 6,1);
delay(200);
lcdClear();
lcdSetText("Model", 5,0);
lcdSetText("Railroading", 2,1);
delay(200);
lcdClear();
lcdSetText("Project", 4,0);
delay(200);
lcdClear();
//END Credits on LCD

lcdSetText("Gate #1 LED Test", 0,0);
P1OUT |= BIT0;
delay(200);
P1OUT &= ~BIT0;

lcdSetText("Gate #2 LED Test", 0,1);
P1OUT |= BIT6;
delay(200);
P1OUT &= ~BIT6;
lcdClear();
if ((P1IN & BIT4) == 0) {
lcdSetText("IR Gate #1 Pass", 0,0);
delay(200);
} else {lcdSetText("IR Gate #1 FAIL", 0,0); delay(500);}

if ((P1IN & BIT5) == 0) {
lcdSetText("IR Gate #2 Pass", 0,1);
delay(200);
} else {lcdSetText("IR Gate #2 FAIL", 0,1); delay(500);}
lcdClear();

while(1)                                  //Loop forever, we work with interrupts!
  {
// Basically we loop here doing nothing until an IR Beam is triggered

lcdSetText("Waiting for", 3, 0);
lcdSetText("Train", 6, 1);

		  if( (P1IN & BIT4) > 0 && lockgate_2 == 0)  // If Gate #1 is detecting an object and Gate #2 is not
        	{
			  do{
			  	 lockgate_1 = 1; // Gate #1 Locked so that it will not interrupt
			  	P1OUT |= BIT0; // Turn on RED Indicator to show Gate #1 as locked
			  	 lcdClear();
        		 lcdSetText("Clocking Speed", 1, 0);
        		 lcdSetText(" ", 0,1);
        		 counter_active = 1; // Start Counting from Gate #1 to Gate #2
  				 output_count = 1; 	// tell the rest of the program we counted
			     }while ((P1IN & BIT5) == 0 && lockgate_1 == 1);  // Run the above code while Gate #2 is not detecting and Gate #1 is locked
        	} else {lockgate_1 = 0; P1OUT &= ~BIT0;} // Otherwise unlock Gate #1 and turn off RED INDICATOR LED



          if( (P1IN & BIT5) > 0 && lockgate_1 == 0 ) // If Gate #2 is detecting an object and Gate #1 is not
          {
        	  do {
        		  lockgate_2 = 1; // Gate #2 Locked so that it will not interrupt
        		  P1OUT |= BIT6; // Turn on GREEN Indicator to show Gate #2 as locked
        		  lcdClear();
        		  lcdSetText("Clocking Speed", 1, 0);
        		  lcdSetText(" ", 0,1);
        		  counter_active = 1; // Start Counting from Gate #2 to Gate #1
        		  output_count = 1; 	// tell the rest of the program we counted
               	 }while ((P1IN & BIT4) == 0 && lockgate_2 == 1); // Run the above code while Gate #1 is not detecting and Gate #2 is locked

          	}else {lockgate_2 = 0; P1OUT &= ~BIT6;} // Otherwise unlock Gate #2 and turn off GREEN INDICATOR LED

   counter_active = 0; 	// done counting - ready to output

  		if ( output_count == 1 ) {	// show the count
  			// calculate speed in MPH
  			speed = factor / counter;	// calculate MPH
  			lcdClear();
  			lcdSetText("Train Speed", 2, 0);
  			lcdSetInt(speed, 4, 1);
  			lcdSetText("MPH", 8,1);
  			delay(500);
  			output_count = 0;	// stop displaying and wait for another trigger
  			lcdClear();

  	} 	// end of IF
  } 	// end of WHILE
} 	// end of MAIN

// Timer A0 interrupt service routine
#pragma vector=TIMER0_A0_VECTOR
__interrupt void Timer_A (void)
{
	if (counter_active == 1) {
		counter = counter + 1; 	// just counting milliseconds

		if (counter > factor) { 	// if our counter reaches factor, we're going less than 1 MPH
									// and we can't display it, so ...
			counter = 0; 			// zero counter
			counter_active = 0;		// stop counting
			delay(1000);			// and hang for a bit
			}
	}// then return to main
}// end of TIMER_A interrupt handler

// Port 1 interrupt service routine
#pragma vector=PORT1_VECTOR
__interrupt void Port_1(void)
{
	while (lockgate_1 == 1)  {  // Locks interrupt from disturbing opposite direction detection
	   P1IFG &= ~BIT5;                     		// P1.4 IFG cleared
	   P1IES ^= BIT5; // toggle the interrupt edge,
	}

	while (lockgate_2 == 1) { // Locks interrupt from disturbing opposite direction detection
	   P1IFG &= ~BIT4;                     		// P1.5 IFG cleared
	   P1IES ^= BIT4; // toggle the interrupt edge,
	}
	   // the interrupt vector will be called
	   // when P1.4 or P1.5 goes from HitoLow as well as
	   // LowtoHigh
}

void delay(unsigned int ms)	// delay for ms in 10 millisecond intervals
{
 while (ms--)
    {
        __delay_cycles(10000); // set for 1 Mhz (10000=10ms)
    }
}
