/*
 * btl.cpp
 *
 * Created: 27-Feb-25 4:37:17 PM
 * Author : acer
 */ 
#define F_CPU 8E6

#include <avr/io.h>
#include <stdio.h>
#include <stdlib.h>
#include <util/delay.h>
#include <string.h>
#include <avr/interrupt.h>

#include "led_7_seg.h"

double voltage_mode = 5;
double current_mode = 1;
const double voltage_coefficient = 50 / 4.95882;


double ADC_Read(uint8_t channel) {
	ADMUX = (ADMUX & 0xF8) | (channel & 0x07); //choosing ADC channel
	ADCSRA |= (1 << ADSC); // begin convertign 
	while (ADCSRA & (1 << ADSC)); //while not done yet
	return ADC; //return ADCL + ADCH
}

double ADC_Read(uint8_t channel, uint8_t gain) {
	ADMUX |= 0b11100000 ;
	ADMUX |= (channel & 0x0F);//choosing ADC channel
	ADCSRA |= (1 << ADSC); // begin convertign
	while (ADCSRA & (1 << ADSC)); //while not done yet
	return ADC; //return ADCL + ADCH
}


ISR (INT0_vect)
{
	if(voltage_mode == 50)
	{
		voltage_mode = 5;
		PORTB = 1<<PB0;	
	}
	else if(voltage_mode == 25)
	{
		voltage_mode = 50;
		PORTB = 1<<PB2;
	}
	else 
	{
		voltage_mode = 25;
		PORTB = 1<<PB1;
	}
		
};

int main(void)
{
	_LED_init();
	DDRB = (1<<PB0) | (1<<PB1) | (1<<PB2);
	PORTB = (1<<PB0) | (1<<PB1) | (1<<PB2) | (1<<PB3);
	//setting up ADC
	//set to using external VREF
	ADMUX = (1<<REFS0);
	//turn on ADC function
	ADCSRA = (1<<ADEN) ;
	//set prescaler to 128 (page 219)
	ADCSRA |= (1<<ADPS0) | (1<<ADPS1) | (1<<ADPS2);
	


	//set interrupt mode (page 49)
	GICR = (1<<INT0);
	MCUCR = (1<<ISC11) | (1<<ISC01);
	
	//button pull up res
	PORTD |= 1<<PD2;
	sei();
	
	//string to store voltage value
	
	char string[10];
	bool reading_voltage= true;
    while (1) 
    {
		//if switch close -> change to take current 
		if( (PINB & (1<<PB3)) == 0){
			reading_voltage = false;
		}
		else
		{
			reading_voltage = true;
		}
		
		if(reading_voltage)
		{
			//read voltage 
			//read the POSITIVE probe
			double adc_value_pos = ADC_Read(3);
			double voltage_plus =  adc_value_pos * 5.0/1024.0;
			voltage_plus = voltage_plus * voltage_coefficient;
			
			//read the NEGATIVE probe
			double adc_value_neg = ADC_Read(4);
			double voltage_neg =  adc_value_neg * 5.0/1024.0;
			voltage_neg = voltage_neg * voltage_coefficient;
			
			//get voltage from pos and neg terminal
			double voltage = voltage_plus - voltage_neg;
			if(voltage >= voltage_mode)
			{
				//if differential is >> set mode, cap it
				voltage = voltage_mode;
			}
			//change a double to a char[] and store it to string
			//number,length of char, decimal point, srting
			dtostrf(voltage,5, 3, string);
		}
		else {
			//read current
			double adc_value_pos = ADC_Read(0,200);
			double amp_plus =  adc_value_pos * 5.0/1024.0;
			//amp_plus = amp_plus * voltage_coefficient;
			
			//read the NEGATIVE probe
			double adc_value_neg = ADC_Read(1,200);
			double amp_neg =  adc_value_neg * 5.0/1024.0;
			//amp_neg = amp_neg * voltage_coefficient;
			
			//get voltage from pos and neg terminal
			double current = amp_plus - amp_neg;
			if(current >= current_mode)
			{
				//if differential is >> set mode, cap it
				current = current_mode;
			}
			
			//change a double to a char[] and store it to string
			//number,length of char, decimal point, srting
			dtostrf(current,5, 3, string);			
		}
		printString(string);

		

			
    }
}




