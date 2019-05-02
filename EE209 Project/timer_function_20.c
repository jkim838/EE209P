	/*
 * EE209 Project.c
 *
 * Created: 9/5/2017 12:04:58 AM
 * Author : Group 20
 */ 

// Header files...
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>

// Custom header files...
#include "timer_prototypes_20.h"

// Custom macro definitions...
#define F_CPU 16000000UL						// Define CPU Operational Clock

// Include delay header
#include <util/delay.h>

void timer_init(){
	
	/*** TCCR0A: Timer / Counter Control Register A ***/
	/*** Compare Match Output A Mode ***/
	TCCR0A &= ~(1 << COM0A1);						// OC0A on Compare Match 'off'
	TCCR0A &= ~(1 << COM0A0);
	/*** Compare Match Output B Mode ***/
	TCCR0A &= ~(1 << COM0B1);						// 0C0B on Compare Match 'off'
	TCCR0A &= ~(1 << COM0B0);
	/*** Waveform Generation Mode ***/
	TCCR0B &= ~(1 << WGM02);						// Normal Counter-Timer Operation
	TCCR0A &= ~(1 << WGM01);							
	TCCR0A &= ~(1 << WGM00);
	
	/*** TCCR0B: Timer / Counter Control Register B ***/
	/*** Force Output Compare A ***/
	TCCR0B &= ~(1 << FOC0A);							// Force Output Compare A 'off'
	/*** Force Output Compare B ***/
	TCCR0B &= ~(1 << FOC0B);							// Force Output Compare B 'off'
	/*** Clock Select Bits ***/
	TCCR0B |= (1 << CS02);							// Pre-scaler: 256
	TCCR0B &= ~(1 << CS01);							// 16MHz / 256 = 62.5KHz
	TCCR0B &= ~(1 << CS00);
	
	/*** TCNT0: Timer / Counter Register ***/
	/*** Setup desired delay between counts here...
	Equation to follow: 
	F(timer) = 62.5KHz
	T(timer) = 0.016ms
	TCNT0 = 10 / 0.016 = 625
	Define a variable that keeps a track of number of overflows...
	On third round of count...
	625 - 512  - 1 = 112
	TCNT0 = 112 = 0x70
	***/

	/*** OCR0A: Output Compare Register A ***/
	// A match between OCR0A and TCNT0 will call interrupt, or generate waveform output on OC0A pin
	/*** OCR0B: Output Compare Register B ***/
	// A match between OCR0B and TCNT0 will call interrupt, or generate waveform output on OC0B pin
	//OCR0B = 'NEED VALUE';
	
	/*** TIMSK0: Timer / Counter 0 Mask Register (Actuator) ***/
	/*** Timer / Counter 0 Output Compare Match A Interrupt ***/
	TIMSK0 &= ~(1 << OCIE0A);						// Interrupt when TCNT0 = OCR0A 'Off'
	/*** Timer / Counter 0 Output Compare Match B Interrupt ***/
	TIMSK0 &= ~(1 << OCIE0B);						// Interrupt when TCNT0 = OCR0B 'Off'
	/*** Timer / Counter 0 Overflow Interrupt Enable ***/
	TIMSK0 |= (1 << TOIE0);							// Interrupt when Overflow On
	
	/*** TIFR0: Timer / Counter 0 Mask Register (Flag) ***/
	/*** Timer Counter Output Compare A Match Flag ***/
	//TIFR0 |= (1 << OCF0A);							// Flag when TCNT0 = OCR0A 'On'
	/*** Timer Counter Output Compare B Match Flag ***/
	//TIFR0 |= (1 << OCF0B);							// Flag when TCNT0 = OCR0B 'On'
	/*** Timer / Counter 0 Overflow Flag ***/
	//TIFR0 |= (1 << TOV0);								// Flag when Overflow 'On'
	
}