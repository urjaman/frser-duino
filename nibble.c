/*
	This file was part of bbflash, now frser-atmega644.
	Copyright (C) 2013, Hao Liu and Robert L. Thompson
	Copyright (C) 2013 Urja Rannikko <urjaman@gmail.com>

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "main.h"
#include "nibble.h"

// 1 instruction is 83 ns, i think we wouldnt even need a single nop ever, but for safety, a nop.
#define delay() asm("nop")

#define FRAME_DDR			DDRC
#define FRAME_PORT			PORTC
#define FRAME				PC7

#define CLK_DDR				DDRA
#define CLK_PORT			PORTA
#define CLK				PA0

#define INIT_DDR			DDRA
#define INIT_PORT			PORTA
#define INIT				PA7

void nibble_set_dir(uint8_t dir)
{
	if (dir) {
		DDRC |= 0x03;
		DDRD |= 0xC0;
	} else {
		DDRC &= ~(0x03);
		DDRD &= ~(0xC0);
		PORTC &= ~(0x03);
		PORTD &= ~(0xC0);
	}	
}

uint8_t nibble_read(void)
{
	uint8_t rv;
	rv = (PINC & 0x03)<<2;
	rv |= (PIND & 0xC0)>>6;
	return rv;
}

void nibble_write(uint8_t data)
{
	PORTC = ((PORTC & 0xFC) | ((data>>2) & 0x03));
	PORTD = ((PORTD & 0x3F) | ((data<<6) & 0xC0));
}

void clock_low(void)
{
	CLK_PORT &= ~_BV(CLK);
	delay();
}

void clock_high(void)
{
	CLK_PORT |= _BV(CLK);
	delay();
}

void clock_cycle(void)
{
	clock_low();
	clock_high();
}


bool nibble_init(void)
{
	uint8_t i;

	INIT_DDR |= _BV(INIT);
	INIT_PORT &= ~_BV(INIT);

	CLK_DDR |= _BV(CLK);
	CLK_PORT |= _BV(CLK);

	FRAME_DDR |= _BV(FRAME);
	FRAME_PORT |=  _BV(FRAME);	

	nibble_set_dir(OUTPUT);
	nibble_write(0);

	for (i = 0; i < 24; i++)
		clock_cycle();
	INIT_PORT |= _BV(INIT);
	for (i = 0; i < 42; i++)
		clock_cycle();

	return true;
}

void nibble_cleanup(void)
{
	CLK_DDR &= ~_BV(CLK);
	FRAME_DDR &= ~_BV(FRAME);
	nibble_set_dir(INPUT);
}

void clocked_nibble_write(uint8_t value)
{
	clock_low();
	nibble_write(value);
	delay();
	clock_high();
	delay();
}

uint8_t clocked_nibble_read(void)
{
	clock_cycle();
	return nibble_read();
}

void nibble_start(uint8_t start)
{
	FRAME_PORT |= _BV(FRAME);
	nibble_set_dir(OUTPUT);
	clock_high();
	FRAME_PORT &= ~_BV(FRAME);
	nibble_write(start);
	clock_cycle();
	FRAME_PORT |= _BV(FRAME);
	delay();
}

bool nibble_ready_sync(void)
{
	uint8_t nib;
	uint8_t x=32;
	do {
		
		nib = clocked_nibble_read();
		if (!(x--)) return false;
	} while (nib != 0);
	return true;
}

uint8_t byte_read(void)
{
	return clocked_nibble_read()
	| (clocked_nibble_read() << 4);
}

void byte_write(uint8_t byte)
{
	clocked_nibble_write(byte & 0xf);
	clocked_nibble_write(byte >> 4);
}

void nibble_hw_init(void) {
	uint8_t x;
	// !RST = A16 = PB1 => 1
	// !INIT =  !OE = PA7 => 1
	// A6..A9 = PA4..5 + PB4..5  => pullup
	// A17 = GPI4 = PA1 => pullup
	// IC = A14 = PA2 => 0
	// !WP = A5 = PB6 => 1
	// !TBL = A4 = PB7 => 1
	// CLK = !WE = PA0 => 1
	// !LFRAME = A10 = PC7 => 1
	// PORTA:
	// 01457=1, 2=0
	// DDRA:
	// 027=1,45=0
	// PORTB:
	// 1456 => 1
	// DDRB:
	// 16 => 1
	// PORTC:
	// 7 => 1
	// DDRC:
	// 7 => 1
	x = PORTA;
	x |= (_BV(0)||_BV(1)|_BV(4)|_BV(5)|_BV(7));
	x &= ~_BV(2);
	PORTA = x;
	x = DDRA;
	x |= (_BV(0)|_BV(2)|_BV(7));
	x &= ~(_BV(4)|_BV(5));
	DDRA = x;
	PORTB |= (_BV(1)|_BV(4)|_BV(5)|_BV(6));
	DDRB |= (_BV(1)|_BV(6));
	PORTC |= _BV(7);
	DDRC |= _BV(7);
}
