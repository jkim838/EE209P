/*
 * EE209 Project.c
 *
 * Created: 9/5/2017 11:25:58 AM
 * Author : Group 20
 */ 

// Header files...
#include <avr/io.h>
#include <stdint.h>

// Custom header files...
#include "usart_prototypes_20.h"

// Custom macro definitions...
#define F_CPU 16000000UL						// Define CPU Operational Clock
#define DATA_REG_IS_FULL !((1 << UDRE0) & UCSR0A)
#define ASCII_CONVERT 48						// Convert to ASCII Characters
#define UNIT_VOLT 0b00001011
#define UNIT_AMPS 0b10001010
#define UNIT_POWR 0b01001100
#define GROUP_G 0b11111001
#define GROUP_P 0b11101100
#define GROUP_2 0b11010010
#define GROUP_0 0b11000000

// include delay headers
#include <util/delay.h>
// USART Transmitter Properties setup...
void usart_init(uint16_t UBRR){
	
	//Enable USART Transmitter
	UCSR0B |= (1 << TXEN0);
	//UCSR0B |= (1 << RXEN0);
	UCSR0B |= (1 << RXCIE0);
	
	// Sets the mode of the USART to be asynchronous.
	UCSR0C &= ~(1 << UMSEL01);
	UCSR0C &= ~(1 << UMSEL00);
	
	// Disables the parity bit.
	UCSR0C &= ~(1 << UPM01);
	UCSR0C &= ~(1 << UPM00);
	
	// Sets only one stop bit.
	UCSR0C &= ~(1 << USBS0);
	
	// Sets the data width to be 8 bits (UCSZ0[2..0] = 011).
	UCSR0B &= ~(1 << UCSZ02);
	UCSR0C |= ((1 << UCSZ01) | (1 << UCSZ00));
	// Sets the UBRR to be 103.
	UBRR0H = (UBRR >> 8);
	UBRR0L = UBRR;
	
}

void usart_decompose(uint8_t *target, double quantized_value, uint8_t parameter_type){

	if(quantized_value < 10){
		quantized_value = quantized_value * 100;
	}
	else if(quantized_value < 100 && (parameter_type != 2)){
		quantized_value = quantized_value * 10;
	}
	else if(parameter_type == 1){
		quantized_value = quantized_value /10;
	}

	target[2] = (uint16_t)(quantized_value / 100);
	target[1] = (uint16_t)(quantized_value / 10) % 10;
	target[0] = (uint16_t)quantized_value % 10;

}

void usart_setUnits(uint8_t parameter_type){

	if(parameter_type == 0){
		// transmit v
		usart_transmit(UNIT_VOLT);
	}
	else if(parameter_type == 1){
		// transmit a
		usart_transmit(UNIT_POWR);
	}
	else if(parameter_type == 2){
		// transmit p
		usart_transmit(UNIT_AMPS);
	}


}

void usart_setGroup(){

	usart_transmit(GROUP_G);
	usart_transmit(GROUP_P);
	usart_transmit(GROUP_2);
	usart_transmit(GROUP_0);
	
}

void usart_prepare_data(double quantized_value, uint8_t parameter_type){

	// for voltage... we are expecting outcome "xx.x"
	// for current... we are expecting outcome "xxx"
	// for power... we are expecting outcome "x.xx"
	
	/*** Declared Variables ***/
	uint8_t decomposed[3];										// Array to store three digits of decomposed digital values
	uint8_t sending_digit = 0b00000000;							// Encoded Digit to be sent to USART transmitter
	/*** Data Encoding ***/
	usart_decompose(decomposed, quantized_value, parameter_type);				// Decompose digital value to three digits
	sending_digit |= ((parameter_type) << 6);					// Do not reset these parameter digits: these are frequently re-used within this scope.	%2
//	sending_digit |= ((parameter_type) << 6); // /2)%2

	uint8_t i;
	for (i = 3; i > 0; i--){
//		uint8_t bin_second_digit = i % 2;						// Decomposed second digit of index number in binary
//		uint8_t bin_first_digit = (i / 2) % 2;					// Decomposed first digit of index number in binary
		sending_digit |= (i << 4);				// Set 5-bit of sending digit according to second digit of the index number
//		sending_digit |= (bin_first_digit << 4);				// Set 4-bit of sending digit according to first digit of the index number
		sending_digit |= decomposed[i - 1];						// Select data-bits associated with the index number. Set [3:0] bits.
		usart_transmit(sending_digit);							// Transmit data
		sending_digit &= ~(0b00111111);							// Reset position and data-bits for next transmission
	}

	usart_setUnits(parameter_type);
	
}

// Begin data transmission...
void usart_transmit(uint8_t data) {
	
	while (DATA_REG_IS_FULL);		// Waits for the UDR register to be empty.
	UDR0 = data;					// Transmits the data.
	_delay_ms(20);

}
