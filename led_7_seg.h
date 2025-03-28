#ifndef __LED_7_SEG
#define __LED_7_SEG

#include <avr/io.h>
#include <stdio.h>
#include <stdlib.h>
#include <util/delay.h>
#include <string.h>

#include "settings.h"

void _LED_init();
void _print_7Segment(char, int);

void printString(char[]);

void _LED_init()
{
	//port c is gonna be light controller
	DDRC = 0xFF;	//set all of port C to input mode (to let it sink current into the IC)
	PORTC = 0xFF;	//set all portC to HIGH pull-up
	
	//port D is for choosing which number to show
	DDRD = 0xF0;	// set half of port D to output mode
}

void printString(char* stringg)
{
	//print starting in position 0
	unsigned int pos = 0;
	bool decimal_toggle = false;
	
	//while pos is less than the length
	while (pos < strlen(stringg))
	{
		//if there is a decimal point, print at the same pos as previous
		if(stringg[pos+1] == '.')
		{
			_print_7Segment(stringg[pos],pos);
			_print_7Segment('.',pos);
			pos += 2;
			decimal_toggle = true;
		}
		else {
			//overly complicated? yea
			//can i come up with anything else? nope
			//but hey it works, any genius can rewrite this anytime!
			if(!decimal_toggle)
			{
				_print_7Segment(stringg[pos],pos);
			}
			else
			{
				_print_7Segment(stringg[pos],pos-1);
			}
			
			pos++;
		}
	}
}

void _print_7Segment(char character, int position)
{
	//convert string number to bit to display
	
	#ifdef _DEBUG
	_delay_ms(100);
	#endif // _DEBUG
	
	//reset the 2 ports
	PORTC = 0xFF;
	PORTD = 0x00;
	//print in which position
	switch (position)
	{
		case 0:
		PORTD = 0b11100000;
		break;
		case 1:
		PORTD = 0b11010000;
		break;
		case 2:
		PORTD = 0b10110000;
		break;
		case 3:
		PORTD = 0b01110000;
		break;
		default:
		return;
	}
	
	//print what character
	switch(character)
	{
		case '0':
		PORTC = 0b11000000;
		break;
		
		case '1':
		PORTC = 0b11111001;
		break;
		
		case '2':
		PORTC = 0b10100100;
		break;
		
		case '3':
		PORTC = 0b10110000;
		break;
		
		case '4':
		PORTC = 0b10011001;
		break;
		
		case '5':
		PORTC = 0b10010010;
		break;
		
		case '6':
		PORTC = 0b10000010;
		break;
		
		case '7':
		PORTC = 0b11111000;
		break;
		
		case '8':
		PORTC = ~0b01111111;
		break;
		
		case '9':
		PORTC = ~0b01101111;
		break;
		
		case '.':
		PORTC = 0b01111111;
		break;
		
		case '-':
		PORTC = ~0b01000000;
		break;
		
		case 'H':
		PORTC = 0b10001001;
		break;
		
		case 'E':
		PORTC = 0b10000110;
		break;
		
		case 'L':
		PORTC = 0b11000111;
		break;
		
		default:
		return;
	}
	
	
}


#endif
