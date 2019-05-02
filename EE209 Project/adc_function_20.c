/*
 * EE209 Project.c
 *
 * Created: 9/5/2017 11:43:58 AM
 * Author : Group 20
 * Revision 8
 *
 */ 

/*** Header files ***/
#include <avr/io.h>
#include <stdint.h>

/*** Custom Header files ***/
#include "adc_prototypes_20.h"

/*** Custom Macro Definitions ***/
#define F_CPU 16000000UL					// Define CPU Operational Clock
#define ADC_RESOLUTION 1023					// 2^10 - 1
#define VREF 5								// External 5V reference voltage
#define ORIGINAL_ARRAY_LENGTH 24			// voltage array length
#define COMBINED_ARRAY_LENGTH 47			// voltage combined length
// Include delay header
#include <util/delay.h>

void adc_init(){
	
	/*** ADMUX: ADC Multiplexer Selection Register ***/
	/*** Voltage Reference Selection***/
	ADMUX &=~(1 << REFS1);											// Use Vcc as reference voltage
	ADMUX |= (1 << REFS0);											
	/*** ADC Left Adjust Result ***/
	ADMUX &=~(1 << ADLAR);											// Right Adjust ADC reading into ADCH
	/*** Analogue Channel Selection Bits ***/
// 	ADMUX &=~(1 << MUX3);
// 	ADMUX &=~(1 << MUX2);
// 	ADMUX &=~(1 << MUX1);
// 	ADMUX &=~(1 << MUX0);
		
 	ADMUX &=~(1 << MUX3);											// Channel 5: Input pin is PC5
	ADMUX |= (1 << MUX2);
	ADMUX &=~(1 << MUX1);
	ADMUX |= (1 << MUX0);
	// Set ADATE if more than single conversion mode...
	
	/*** ADCSRA: ADC Control and Status Register A ***/
	/*** ADC Enable ***/
	ADCSRA |= (1 << ADEN);											// ADC Power 'On'
	/*** ADC Auto Trigger ***/
	ADCSRA |= (1 << ADATE);											// Auto Trigger 'On'
	/*** ADC Interrupt Setting ***/
//	ADCSRA &=~(1 << ADIF);											
	ADCSRA |= (1 << ADIE);											// Interrupt 'On'
	/*** Pre-scaler Setup ***/
	ADCSRA |= (1 << ADPS2);											// Pre-scaler Division factor to 64
	ADCSRA |= (1 << ADPS1);
	ADCSRA &=~(1 << ADPS0);
	
	/***ADCSRB: ADC Control and Status Register B***/
	/*** ADC Auto Trigger Source ***/
	// NOTE: If ADATE is cleared, ADTS will have no effect
	ADCSRB &=~(1 << ADTS0);											// Timer / Counter 1 Overflow 
	ADCSRB |= (1 << ADTS1); 
	ADCSRB |= (1 << ADTS2);
			
}

void adc_read() {
	
	if (ADC_toggle == 0){
		ADMUX &= 0xf0;													//reset to channel 0
		ADMUX |= 0x05;													//select channel 5
		ADCSRA |= (1 << ADSC);											// Start new conversion
	}
	else if (ADC_toggle ==1){
		ADMUX &= 0xf0;													//select channel 0

//	ADMUX |= 0x02;
		ADCSRA |= (1 << ADSC);											// Start new conversion
	}
//	while (!(ADCSRA & (1 << ADIF)));								// Block until conversion is done
//	ADC_voltage = ADC;												// Receive value from ADC

//	ADMUX |= 0x02;													//select channel 2
//	ADCSRA |= (1 << ADSC);											// Start new conversion
//	while (!(ADCSRA & (1 << ADIF)));								// Block until conversion is done
//	ADC_current = ADC;												// Receive value from ADC

}

void adc_calculate() {
	
	uint8_t i;
	for(i = 0; i < 24; i++){
		Vin_voltage[i] = (double)voltage_package[i] / ADC_RESOLUTION * VREF;
		Vin_current[i] = (double)current_package[i] / ADC_RESOLUTION * VREF;
	}

}

double adc_calculate_max(uint16_t input){
	double output = ((double)input)/ADC_RESOLUTION * VREF;
	return output;
}

void approximate(double *values, double *target, int target_length){

	int i;
	for (i = 0; i < (target_length - 1); i++){
		target[i] = (values[i] + values[i + 1]) / 2;
	}
	
}

void combine(double *values_one, double *values_two, double *target, int target_length){
	/*This loop combines the 2 arrays storing voltage information together.
	The j counter is used to keep track of the position of values in the combined array.
	The i counter is used to go through the elements of the other two arrays.
	For each iteration of the loop, 2 positions are filled in the combined array.
	Thus the j counter starts at 0 (like the i counter) but it is incremented by 2 through every loop*/

	int i;
	int j = 0;
	for (i = 0; i < target_length; i++){
		target[j] = values_one[i];
		if(i < target_length){
			target[j + 1] = values_two[i];
		}
		j += 2;
	}

}

double adc_Vin_magnitude(double Vin_parameter){
	
	if(Vin_parameter < 2.5){
		Vin_parameter = 2.5 - Vin_parameter;
	}
	else{
		Vin_parameter = Vin_parameter - 2.5;
	}

	return Vin_parameter;

}

double calculate_average(double* voltage_values, double* current_values){
	
	double power_average = 0;
	
	// Approximation arrays
	double voltage_approxn[ORIGINAL_ARRAY_LENGTH - 1]; //this array will store a value of voltage values that were calculated using linear approximation.
	double current_approxn[ORIGINAL_ARRAY_LENGTH - 1]; //this array will store a value of current values that were calculated using linear approximation.

	// Combined arrays
	double voltage_combined[(ORIGINAL_ARRAY_LENGTH * 2) - 1]; //This array stores all the voltage values. (i.e. both the approximated and acquired ones).
	double current_combined[(ORIGINAL_ARRAY_LENGTH * 2) - 1]; //This array stores all the voltage values. (i.e. both the approximated and acquired ones).

	// Power array sizes
	double power[COMBINED_ARRAY_LENGTH]; //This array will store power values.
	double power_trapezoidal[COMBINED_ARRAY_LENGTH]; //size may need to be adjusted as above
	
	//values target length
	approximate(voltage_values, voltage_approxn, ORIGINAL_ARRAY_LENGTH);
	approximate(current_values, current_approxn, ORIGINAL_ARRAY_LENGTH);
	//one two target length
	combine(voltage_values, voltage_approxn, voltage_combined, ORIGINAL_ARRAY_LENGTH);
	combine(current_values, current_approxn, current_combined, ORIGINAL_ARRAY_LENGTH);
	
	/*power calculation code*/
	int i;
	for(i=0; i<(COMBINED_ARRAY_LENGTH); i++){ 
		power[i] = voltage_combined[i]*current_combined[i]; //p = vi 
	}

	for(i=0; i<(COMBINED_ARRAY_LENGTH - 1); i++){
		power_trapezoidal[i] = power[i]+power[i+1] / 2; //calculate the power using the trapezoidal method.
	}
	
	for (i=0; i<(COMBINED_ARRAY_LENGTH); i++) {
		power_average = power_average + power_trapezoidal[i]; 
	}
	power_average = (power_average / COMBINED_ARRAY_LENGTH);

	return power_average;
	
}
