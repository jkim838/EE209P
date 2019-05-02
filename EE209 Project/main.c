/*
 * EE209 Project.c
 *
 * Created: 9/5/2017 11:03:58 AM
 * Author : Group 20
 * Revision 8
 *
 */ 

/*** Header files ***/
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>
#include <stdio.h>

/*** Custom header files ***/
#include "usart_prototypes_20.h"
#include "adc_prototypes_20.h"
#include "timer_prototypes_20.h"
//#include "interrupt_prototypes_20.h"

/*** Custom macro definitions ***/
#define F_CPU 16000000UL						// Define CPU Operational Clock
/*** USART ***/
#define BAUD_RATE 9600							// Define Baud Rate
#define UBRR_VALUE F_CPU / 16 / BAUD_RATE -1	// Define UBRR value
#define ASCII_CONVERT 48						// Convert to ASCII Characters
#define CS_R2_RES 33.0
#define CS_R1_RES 5.6
#define CS_SHUNT_RES 0.5
#define VS_RA_RES 82.0
#define VS_RB_RES 10.0
#define ROOT2 1.414213562

/*** include delay headers ***/
#include <util/delay.h>

/*** ISR definitions ***/
volatile uint8_t tot_overflow; //global variable to count the number of overflows
volatile uint8_t isr_voltage_count = 0;
volatile uint8_t isr_current_count = 0;
volatile uint16_t maximum_voltage =0;
volatile uint16_t maximum_current =0;
volatile uint16_t samples_counter = 0;
ISR(TIMER0_OVF_vect){
	
// 	tot_overflow++;	// increment overflow count one when overflow occurs.
// 	TIFR0 |= (1 << TOV0);

}

 ISR(USART_RX_vect){

 }

// keep a track of how many values were added to the array
// when it reaches xx values only then we calculate using values of array

ISR(ADC_vect){
	
	samples_counter++;
	
	if (ADC_toggle == 0){
		if(isr_voltage_count < 24){
			current_package[isr_current_count] = ADC;
		}	
		if(ADC>maximum_current){
			maximum_current = ADC;
		}
		
		ADC_toggle = 1;
		isr_current_count++;
		ADMUX &= 0xf0;													//reset to channel 0
		ADMUX |= 0x05;													//select channel 5

	}
	else if(ADC_toggle == 1){
		if(isr_voltage_count < 24){
			voltage_package[isr_voltage_count] = ADC;	
		}
		if(ADC>maximum_voltage){
			maximum_voltage = ADC;
		}
		
		ADC_toggle = 0;
		isr_voltage_count++;
		ADMUX &= 0xf0;													//select channel 0

	}
	
	ADCSRA |= (1 << ADSC);											// Start new conversion
	
}

ISR(INT0_vect){
	
	// do something...
	
}

ISR(INT1_vect){
	
	// do something...
	
}


 	
 //	volatile double current_package[24];
	volatile double power_average;

 	volatile double power_average_array[50];
	
void take_voltage_sample(){
	
	uint8_t i;
	for(i = 0; i < 24; i++){
		voltage[i] = (adc_Vin_magnitude(Vin_voltage[i])) * ((VS_RA_RES + VS_RB_RES) / VS_RB_RES);
	}

}

void take_current_sample(){
	
	uint8_t i;
	for(i = 0; i < 24; i++){
		current[i] = ((adc_Vin_magnitude(Vin_current[i])) / (CS_R2_RES / CS_R1_RES)) / CS_SHUNT_RES * 1000;
	}
	
}



int main(void){
	/*** Run once code ***/
	/*** Pin I/O set-up ***/
	// Initially set all pins as output. Change to input mode when necessary.
	DDRB = 0xff; 
	DDRC = 0xff; //|= (1 << PC5);
	DDRD = 0xff;
	DDRC&=~ (1 << PC0);
	DDRC&=~ (1 << PC5);
	sei();
/*	ADC_toggle = 0; //initialise this so that we're reading voltage input first.
	ADC_voltage = 0;
	ADC_current = 0;
*/
	/*** Module Configuration ***/
	//timer_init();
	adc_init();
	usart_init(UBRR_VALUE);
	timer_init();
	
	/*** Initialize shared variables ***/
	ADC_toggle = 0;
	/*** For ADC ***/
// 	uint8_t voltage_package_counter = 0;
// 	uint8_t current_package_counter = 0;
	
	uint8_t cycles_count = 0; //counter for power averaging

// 	double maximum_voltage = 0.0; //moved to isr
// 	double maximum_current = 0.0;
	ADCSRA |= (1 << ADSC);											// Start new conversion
	// transmit group number
	usart_setGroup();
    while (1) {
		// Run multiple code...
		// This code is using interrupt method to produce delay of 1 second...
// 		if(tot_overflow >= 244){				// check if number of overflow is 2
// 			if(TCNT0 >= 0x23){					// check if timer count reached 112
// 				PORTC ^= (1 << PC1);			// toggles LED
// 				TCNT0 = 0;						// reset counter
// 				tot_overflow = 0;				// reset overflow counter
// 			}
// 		}

		// PuTTy Debug Variable...
		uint8_t target[3];

		/*** ADC Lab Conversion Test ***/
		//adc_read();
		// after 24 samples of voltage and current were taken...
		if(isr_voltage_count >= 24 && isr_current_count >= 24){
			cli();
			isr_current_count = 0;
			isr_voltage_count = 0;
			adc_calculate();
			take_voltage_sample();
 			take_current_sample();
			power_average_array[cycles_count] = calculate_average(voltage, current);
	//		usart_prepare_data(power_average_array[cycles_count], 1);
			cycles_count++;
			
			
			if(cycles_count >= 50){
				
				cycles_count = 0;
				uint8_t x =0;	
				for (x=0; x<50;x++){
					power_average+= power_average_array[x];
				}
				power_average = power_average/50;
				//usart_prepare_data(power_average, 1);
				
			}
			
			if(samples_counter >= 1200){
				
				samples_counter= 0;
				double max_I;
				double max_V;
				max_I = adc_calculate_max(maximum_current);
				max_V = adc_calculate_max(maximum_voltage);
				
				max_V = (adc_Vin_magnitude(max_V)) * ((VS_RA_RES + VS_RB_RES) / VS_RB_RES);
				max_I = ((adc_Vin_magnitude(max_I)) /(CS_R2_RES / CS_R1_RES)) / CS_SHUNT_RES * 1000 ; //(CS_R2_RES / CS_R1_RES)

				double avgP = (max_V/ROOT2) * (max_I/ROOT2);
				
				usart_decompose(target, max_V, 0);
				uint8_t i;
				for (i = 3; i > 0; i--){
					usart_transmit(target[i-1] + ASCII_CONVERT);
				}
				usart_transmit('V');
				usart_transmit(' ');
				
				usart_decompose(target, max_I, 2);
				uint8_t j;
				for (j = 3; j > 0; j--){
					usart_transmit(target[j-1] + ASCII_CONVERT);
				}
				usart_transmit('m');
				usart_transmit('A');
				usart_transmit(' ');

				usart_decompose(target, avgP, 1);
				uint8_t k;
				for (k = 3; k > 0; k--){
					usart_transmit(target[k-1] + ASCII_CONVERT);
				}
				usart_transmit('W');
				usart_transmit(' ');

// 				usart_prepare_data(max_I, 2);
// 				usart_prepare_data((max_V / ROOT2), 0);
				// transmit group number
				usart_setGroup();
					
				maximum_voltage = 0;
				maximum_current = 0;
				max_I = 0.0;
				max_V = 0.0;
								
			}
			sei();	
		}
// 		adc_calculate();
// 		
// 		if(voltage_package_counter < 24 || current_package_counter < 24){
// 			if(ADC_toggle == 0){
// 				// calculate load voltage value
// 				take_voltage_sample();
// // 				if(voltage > maximum_voltage){
// // 					maximum_voltage = voltage;
// // 				}
// 				// for power average
// 				voltage_package[voltage_package_counter] = voltage;
// 				voltage_package_counter++;
// 			}
// 			else if(ADC_toggle == 1){
// 				// calculate load current value
// 				current = ((adc_Vin_magnitude(Vin_current)) / (CS_R2_RES / CS_R1_RES)) / CS_SHUNT_RES * 1000;
// // 				if(current > maximum_current){
// // 					maximum_current = current;
// // 				}
// 				// for power average		
// 				current_package[current_package_counter] = current;
// 				current_package_counter++;
// 			}		
// 			samples_counter++;
// 	//		usart_prepare_data((double)samples_counter/10, 2);
// 		}
// 		else{
// 			// calculate average power from arrays of voltages and currents
// 			power_average= calculate_average(voltage_package, current_package);
// 			power_average_array[k] = power_average;
// 			k++;
// 			/*** Transmit array contents to PuTTy ***/
// 			/*
// 			uint8_t i;
// 			uint8_t j;
// 			for (i = 0; i < 24; i++){
// 				usart_transmit('V');
// 				usart_transmit(i + ASCII_CONVERT);
// 				usart_transmit(':');
// 				usart_decompose(target, voltage_package[i]);
// 
// 				for(j = 0; j < 3; j++){
// 					usart_transmit(target[j] + ASCII_CONVERT);
// 				}
// 				usart_transmit(' ');
// 				usart_transmit('I');
// 				usart_transmit(i + ASCII_CONVERT);
// 				usart_transmit(':');
// 				usart_decompose(target, current_package[i]);
// 				for(j = 0; j < 3; j++){
// 					usart_transmit(current_package[i] + ASCII_CONVERT);
// 				}
// 				usart_transmit(' ');
// 			}
// 			*/
// 
// 			// reset iteration count
// 			voltage_package_counter = 0;
//  			current_package_counter = 0;
// 			// encode voltage value and transmit
// 			
// 			/*** Sending Vin_voltage to PuTTy ***/
// 			/*			
//  			usart_decompose(target, voltage);
//  			uint8_t i;
//  			for (i = 3; i > 0; i--){
//  				usart_transmit(target[i-1] + ASCII_CONVERT);
//  			}
//  			usart_transmit(' ');
// 			*/
// 			
// 			//After we calculate the average power for 10 cycles, find the average of those values
// 			// then encode and transmit the power value
// 			if(k==5){
// 			uint8_t x;
// 			k=0;
// 				for (x=0; x<5;x++){
// 					power_average+= power_average_array[x];
// 				}
// 			power_average = power_average/5; 
// 			usart_prepare_data(power_average, 1);
// 			}
// 			/*** Sending Vin_current to PuTTy ***/
// 			/*
// //  			usart_decompose(target, Vin_current);
// //  			uint8_t i;
// //  			for (i = 3; i > 0; i--){
// //  				usart_transmit(target[i-1] + ASCII_CONVERT);
// //  			}
// //  			usart_transmit(' ');
// 			*/
// 			// encode power value and transmit
// 			if (samples_counter >= 1200){//25 cycles of V and I
// 				samples_counter= 0;
// 
// 
// 				usart_prepare_data(maximum_current, 2);
// 				usart_prepare_data((maximum_voltage / ROOT2), 0);
// 					
// 				maximum_voltage = 0;
// 				maximum_current = 0;
// 			}
// 			
// 		}
// 		usart_setGroup();

	}
	
	cli();
	return 0;
}