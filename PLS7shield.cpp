#include "Arduino.h"
#include "PLS7shield.h"

#define SCL_HI	(PORTC |= (1<<5))
#define SCL_LO	(PORTC &= ~(1<<5))
#define SDA	(PINC & (1 << 4))
#define SHLD_HI	(PORTB |= (1<<5))
#define SHLD_LO	(PORTB &= ~(1<<5))

volatile byte displayBuffer[5];
volatile byte disp = 4;

ISR(TIMER2_COMPA_vect)
{
	//timer2 interrupt routine
	if (++disp > 4)
		disp = 0;
	PORTB = ~(1 << (4 - disp));	//turn transistor on
	PORTD = ~displayBuffer[disp];	//refresh display
}

PLS7shield::PLS7shield()
{
	DDRD = 0xff;	//port D output
	DDRC = 0xe0;	//PC5 outpir, PC4-PC0 input
	DDRB = 0x3f;	//PB5 - PB0 output


	TCCR2A = 0x02; //timer2: CTC mode
	TCCR2B = 0x04; //timer2: fclk = fosc/64
	OCR2A = 249;	//timer2 period: 250 Tclk (OCR0A + 1 = 250)
	TIMSK2 = 0x02;	//timer2 output compare match A interrupt enable
	sei(); 	//I = 1 (interrupt enable)
}

void PLS7shield::writeDisplay(byte d, byte value)
{
	displayBuffer[d] = value;
}

byte PLS7shield::readDisplay(byte d)
{
	return displayBuffer[d];
}

byte PLS7shield::readSwitches()
{
	//bit-banged serial read
	byte i, tmp = 0, mask = 0x80;

	SHLD_HI;
	SHLD_LO;
	SHLD_HI;	//load buffer

	for (i=0; i<8; i++)
	{
		//read data from serial buffer
		SCL_LO;
		SCL_HI;

		if (SDA)
			tmp |= mask;
		mask >>= 1;
	}

	return ~tmp;
}

byte PLS7shield::switchState(byte s)
{
	byte sw = readSwitches();
	
	if (sw & (1 << (7 - s)))
		return HIGH;
	else
		return LOW;
}

byte PLS7shield::buttonState(byte b)
{
	switch(b)
	{
		case LEFT:
			return !digitalRead(A0);
			break;
		case DOWN:
			return !digitalRead(A1);
			break;
		case RIGHT:
			return !digitalRead(A2);
			break;
		case UP:
			return !digitalRead(A3);
			break;
	}
	return 0;
}
