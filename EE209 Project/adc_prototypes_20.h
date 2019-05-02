/*
 * EE209 Project.c
 *
 * Created: 9/5/2017 11:43:58 AM
 * Author : Group 20
 */ 

volatile uint8_t ADC_toggle; //global variable that will toggle every time an ADC channel is read
uint16_t voltage_package[24];
uint16_t current_package[24];
double Vin_voltage[24];
double Vin_current[24];
double voltage[24]; // voltage across load
double current[24];	// current across load

// Prototype Functions...
void adc_init();
void adc_read();
void adc_calculate();
void approximate(double *values, double *target, int target_length);
void combine(double *values_one, double *values_two, double *target, int target_length);
double adc_Vin_magnitude(double Vin_parameter);
double calculate_average(double *voltage_values, double *current_values);
double adc_calculate_max(uint16_t input);
int smallerArray(int array_one, int array_two);