/*
 * EE209 Project.c
 *
 * Created: 9/5/2017 11:33:58 AM
 * Author : Group 20
 */ 

// Prototype Functions...
void usart_init(uint16_t UBRR);
void usart_prepare_data(double quantized_value, uint8_t parameter_type);
void usart_decompose(uint8_t *target, double quantized_value, uint8_t parameter_type);
void usart_setUnits(uint8_t parameter_type);
void usart_setGroup();
void usart_transmit(uint8_t data);
//uint8_t usart_receive();