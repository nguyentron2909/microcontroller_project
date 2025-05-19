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

//7seg led display functions
void _LED_init(){
	//port b is light controller
	DDRB = 0xFF;	//set all of port b to input mode (to let it sink current into the IC)
	PORTB = 0xFF;	//set all port b to HIGH pull-up
	
	//port c is for choosing which number to show
	DDRC = 0b11000011;	// set half of port c to output mode
	PORTC = 0b11000011;
	_delay_ms(100);
}
//print single characrrer at a position
void _print_7Segment(char character, int position){
	//convert string number to bit to display
	_delay_ms(2);
	//print in which position
	switch (position)
	{
		case 0: PORTC = 0b00000001;	break;
		case 1: PORTC = 0b00000010;	break;
		case 2: PORTC = 0b01000000;	break;
		case 3: PORTC = 0b10000000;	break;
		default:	return;
	}
	
	//print what character
	switch(character)
	{
		case '0': PORTB = 0b11000000;	break;
		case '1': PORTB = 0b11111001;	break;
		case '2': PORTB = 0b10100100;	break;
		case '3': PORTB = 0b10110000;	break;
		case '4': PORTB = 0b10011001;	break;
		case '5': PORTB = 0b10010010;	break;
		case '6': PORTB = 0b10000010;	break;
		case '7': PORTB = 0b11111000;	break;
		case '8': PORTB = 0b10000000;	break;
		case '9': PORTB = ~0b01101111;	break;
		case '.': PORTB = 0b01111111;	break;
		case '-': PORTB = ~0b01000000;	break;
		case 'H': PORTB = 0b10001001;	break;
		case 'E': PORTB = 0b10000110;	break;
		case 'L': PORTB = 0b11000111;	break;
		default: return;
	}
	
}
//peint a string of char starting from pos 0
void printString(char* stringg)
{
	//print starting in position 0
	unsigned int pos = 0;
	bool decimal_toggle = false;
	
	//while pos is less than the length
	while (pos < strlen(stringg)){
		//if there is a decimal point, print at the same pos as previous
		if(stringg[pos+1] == '.'){
			_print_7Segment(stringg[pos],pos);
			_print_7Segment('.',pos);
			pos += 2;
			decimal_toggle = true;
		}
		else {
			//overly complicated? yea
			//can i come up with anything else? nope
			//but hey it works, any genius can rewrite this anytime!
			if(!decimal_toggle){
				_print_7Segment(stringg[pos],pos);
			}
			else{
				_print_7Segment(stringg[pos],pos-1);
			}
			pos++;
		}
	}
}

//ADC functions
bool reading_voltage= true;
double voltage_mode = 5; //change range from 0-5 -25 -50 
//for smooth ADC reading
int stableCount = 0;
const int stabilityThreshold = 20; // number of stable readings needed
//from real measurement of supply voltage, change calibration factor (if needed)
double voltage_coefficient_1 = 50.0 / 5.0;
volatile double preV = 0.0;
volatile double preVV = 0.0;
void _ADC_setup() {
	//set to using external VREF
	ADMUX = (0<<REFS1) | (1<<REFS0);
	//turn on ADC function
	ADCSRA = (1<<ADEN) ;
	//set prescaler to 128 (page 219)
	ADCSRA |= (1<<ADPS0) | (1<<ADPS1) | (1<<ADPS2);
}
double ADC_Read(uint8_t channel) {
	ADMUX = (ADMUX & 0xF8) | (channel & 0x07); //choosing ADC channel
	ADCSRA |= (1 << ADSC); // begin convertign 
	while (ADCSRA & (1 << ADSC)); //while not done yet
	return ADC; //return ADCL + ADCH
}
double readAvgADC(uint8_t channel) {
	double sum = 0;
	const int samples = 10; // read 10 times for avg
	for (int i = 0; i < samples; i++) {
		sum += ADC_Read(channel);
		_delay_us(100); // small delay between reads
	}
	return sum / samples;
}

//interupt
void interrupt_setup() {
	//set interrupt modes
	GICR = (1<<INT0) | (1<<INT1);
	MCUCR = (1<<ISC11) | (1<<ISC01);
	//button pull up res
	PORTD |= 1<<PD2;
	PORTD |= 1<<PD3;
	//set external interupt
	sei();
	
	//led indicators for modes 
	DDRA = 0b11111000; //outputmode
	PORTA = 0b00000000;  
}
//int0 <=> change range
ISR (INT0_vect)
{
	if(voltage_mode == 50)
	{
		voltage_mode = 5;
		PORTA = (0<<PA7) | (0<<PA6) | (1<<PA5);
	}
	else if(voltage_mode == 25)
	{
		voltage_mode = 50;
		PORTA = (1<<PA7) | (0<<PA6) | (0<<PA5);
	}
	else
	{
		voltage_mode = 25;
		PORTA = (0<<PA7) | (1<<PA6) | (0<<PA5);
	}
	
	if(reading_voltage) PORTA |= (1<<PA3) | (0<<PA4) ;
	else PORTA |= (0<<PA3) | (1<<PA4);
	preV = 0.1;
	preVV = 0.1;
	_delay_ms(10);
		
};
//int1 <=> change mode
ISR (INT1_vect){
	
	if(reading_voltage)
	{
		reading_voltage = false;
		 PORTA = (0<<PA3) | (1<<PA4) ;
	}
	else
	{
		reading_voltage = true;
		 PORTA = (1<<PA3) | (0<<PA4);
	}
	
	if(voltage_mode == 50)
	{
		PORTA |= (1<<PA7) | (0<<PA6) | (0<<PA5);
	}
	else if(voltage_mode == 25)
	{
		PORTA |= (0<<PA7) | (1<<PA6) | (0<<PA5);
	}
	else
	{
		PORTA |= (0<<PA7) | (0<<PA6) | (1<<PA5);
	}
	preV =0.1;
	preVV = 0.1;
	_delay_ms(10);
}

int main(void)
{
	//setting up led 7 seg
	_LED_init();
	printString("0.000");
	//setting up ADC
	_ADC_setup();
	//setting up interupt pins
	interrupt_setup();
	
	if(voltage_mode == 50)		PORTA |= (1<<PA7) | (0<<PA6) | (0<<PA5);
	else if(voltage_mode == 25) PORTA |= (0<<PA7) | (1<<PA6) | (0<<PA5);
	else if(voltage_mode == 5) 	PORTA |= (0<<PA7) | (0<<PA6) | (1<<PA5);
	
	if(reading_voltage) PORTA |= (1<<PA3) | (0<<PA4) ;
	else				PORTA |= (0<<PA3) | (1<<PA4);
	
	//string to store voltage value
	char string[10] = "0.000";
	
    while (1) 
    {
		if(reading_voltage)
		{
			//read voltage
			double adc_value_pos = readAvgADC(0);
			double voltage_plus =  adc_value_pos * 5.0 /1023.0;
			voltage_plus = voltage_plus * voltage_coefficient_1;
			
			double voltage = -0.0037*voltage_plus *voltage_plus + 1.0974*voltage_plus    -0.1087 ;
			if(voltage >= voltage_mode) {
				//if differential is >> set mode, cap it
				voltage = voltage_mode;
			}
			if (voltage<=0)
			{
				voltage = 0;
			}
			
			double diff = fabs(preV - voltage);
			if (diff >= 0.001) 
			{
				stableCount++;
				if (stableCount >= stabilityThreshold) 
				{
					dtostrf(voltage , 5 , 3, string);
					preV = voltage;
					stableCount = 0;
					}
			} 
			else 
			{
				stableCount = 0;
			}
		}
		else {
			double adc_value = readAvgADC(1);
			adc_value =  adc_value * 5.0/1023.0;// -0.047 + 0.145; 
			double dual_vol =1e2 * (0.000648879162909*adc_value*adc_value-1.069188345144220*adc_value+3.148559017653335);
			//double dual_vol = 300 - 100 * adc_value;
			
			//account for non linearity in the circuit
			
			/*if(adc_value >= 3.0) {
				//negative voltage 
				dual_vol =   1.068015903394052*dual_vol+ 0.526509183078661;
			}
			else {
				//positive voltage 
				dual_vol = 1.065760411697356*dual_vol + 0.384601488399091;
			}*/
			
			//if the difference between current and shit doesnt pass a certain thresh
			// dont change the value
			double diff;
			if(
			(dual_vol <= 2.5 && dual_vol >= 0) ||
			(dual_vol >= -2.5 && dual_vol < 0)
			)
			{
				//cap small values so that it wouldnt flicker as much
				diff = fabs(preVV*100 - dual_vol*100);
			}
			else {
				diff = fabs(preVV- dual_vol);
			}
			
			// stablization 
			if (diff >= 0.001) {
				stableCount++;
				if (stableCount >= stabilityThreshold) {
					dtostrf(dual_vol , 5 , 3, string);
					preVV = dual_vol;
					stableCount = 0;
				}
			}
			else {
				stableCount = 0;
			}
			
			//dtostrf(dual_vol,5, 3, string);
		}
		printString(string);
    }
}
