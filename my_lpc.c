/*
 * This file is part of the frser-atmega644 project.
 *
 * Copyright (C) 2010 Urja Rannikko <urjaman@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
 */

#include "main.h"
#include "flash.h"
#include "parallel.h"
#include "lpc.h"
#include "uart.h"

static uint8_t lpc_databus_read(void) {
	uint8_t rv;
	rv = (PINC & 0x03)<<2;
	rv |= (PIND & 0xC0)>>6;
	return rv;
}

static void lpc_databus_tristate(void) {
	DDRC &= ~(0x03);
	DDRD &= ~(0xC0);
	PORTC &= ~(0x03);
	PORTD &= ~(0xC0);
}

static void lpc_databus_output(uint8_t data) {
	PORTC = ((PORTC & 0xFC) | ((data>>2) & 0x03));
	PORTD = ((PORTD & 0x3F) | ((data<<6) & 0xC0));
	DDRC |= 0x03;
	DDRD |= 0xC0;
}

static void lpc_init(void) {
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
	lpc_databus_output(0x0F);
}
// BIT 5 of data = do LFRAME
static void lpcbus_cycleout(uint8_t data) {
	lpc_databus_output(data&0x0F);
	if (data&0x10)
		PORTC &= ~_BV(7);
	PORTA &= ~_BV(0);
	asm volatile(
	"nop" "\n\t"
	"nop" "\n\t"
	:: );
	PORTA |= _BV(0);
	PORTC |= _BV(7); // This is a NOP if it was not taken down, thus no if () needed
	asm volatile(
	"nop" "\n\t"
	"nop" "\n\t"
	:: );
	return;
	}

static void lpcbus_cyclegivebus(void) {
	lpc_databus_output(0x0F);
	PORTA &= _BV(0);
	asm volatile( "nop" "\n\t" :: );
	lpc_databus_tristate();
	asm volatile( "nop" "\n\t" :: );
	PORTA |= _BV(0);
	asm volatile(
	"nop" "\n\t"
	"nop" "\n\t"
	:: );
	return;
	}

static void lpcbus_cyclenop(void) {
	PORTA &= _BV(0);
	asm volatile(
	"nop" "\n\t"
	"nop" "\n\t"
	:: );
	PORTA |= _BV(0);
	asm volatile(
	"nop" "\n\t"
	"nop" "\n\t"
	:: );
	return;
	}

static void lpcbus_cycletakebus(void) {
	PORTA &= _BV(0);
	asm volatile( "nop" "\n\t" :: );
	lpc_databus_output(0x0F);
	asm volatile( "nop" "\n\t" :: );
	PORTA |= _BV(0);
	asm volatile(
	"nop" "\n\t"
	"nop" "\n\t"
	:: );
	return;
	}

static uint8_t lpcbus_cyclein(void) {
	lpcbus_cyclenop();
	return lpc_databus_read();
	}

static uint16_t lpc_readcycle(uint32_t addr) {
	uint16_t d;
	lpcbus_cycleout(0x10);					//  1.LFRAME
	lpcbus_cycleout(0b0100);				//  2.0b0100 (0b010X)
	lpcbus_cycleout(0x0F);					//  3.A31..A28
	lpcbus_cyclenop();					//  4.A27..A24
	lpcbus_cycleout(((uint8_t)(addr>>20))&(uint8_t)0x0F);	//  5.A23..A20
	lpcbus_cycleout(((uint8_t)(addr>>16))&(uint8_t)0x0F);	//  6.A19..A16
	lpcbus_cycleout(((uint8_t)(addr>>12))&(uint8_t)0x0F);	//  7.A15..A12
	lpcbus_cycleout(((uint8_t)(addr>> 8))&(uint8_t)0x0F);	//  8.A11..A08
	lpcbus_cycleout(((uint8_t)(addr>> 4))&(uint8_t)0x0F);	//  9.A07..A04
	lpcbus_cycleout(((uint8_t)(addr>> 0))&(uint8_t)0x0F);	// 10.A03..A00
	lpcbus_cyclegivebus();					// 11. TAR0
	lpcbus_cyclenop();					// 12. TAR1
	d = (lpcbus_cyclein()<<8);				// 13. SYNC (included as data for test func, should be 0000)
	d |= lpcbus_cyclein();					// 14. D3..0
	d |= (lpcbus_cyclein()<<4);				// 15. D7..4
	lpcbus_cyclenop();					// 16. TAR0
	lpcbus_cycletakebus();					// 17. TAR1
	return d;
	}

uint8_t lpc_test(void) {
	uint16_t testdata;
	if (parallel_test()) return 0; // If parallel test succeeds, not FWH/LPC
	lpc_init();
	PORTB &= ~_BV(1); //!RST
	PORTA &= ~_BV(7); //!INIT
	_delay_us(1);
	PORTB |= _BV(1);
	PORTA |= _BV(7);
	_delay_us(1);
	testdata = lpc_readcycle(0xFFFFF0L);
	if (testdata&0x0F0000) return 0; // nonzero SYNC
 	return 1; // all seems ok, go for it  :)
	}

void lpc_readn(uint32_t addr, uint32_t len) {
	lpc_init();
	do {
	SEND(lpc_readcycle(addr++));
	} while(--len);
}

// assume nothing, and perform single cycle
uint8_t lpc_read(uint32_t addr) {
	lpc_init();
	return lpc_readcycle(addr);
}

// assume nothing, perform single cycle
void lpc_write(uint32_t addr, uint8_t data) {
	lpc_init();
	lpcbus_cycleout(0x10);					//  1.LFRAME
	lpcbus_cycleout(0b0110);				//  2.0b0110 (0b011X)
	lpcbus_cycleout(0x0F);					//  3.A31..A28
	lpcbus_cyclenop();					//  4.A27..A24
	lpcbus_cycleout(((uint8_t)(addr>>20))&(uint8_t)0x0F);	//  5.A23..A20
	lpcbus_cycleout(((uint8_t)(addr>>16))&(uint8_t)0x0F);	//  6.A19..A16
	lpcbus_cycleout(((uint8_t)(addr>>12))&(uint8_t)0x0F);	//  7.A15..A12
	lpcbus_cycleout(((uint8_t)(addr>> 8))&(uint8_t)0x0F);	//  8.A11..A08
	lpcbus_cycleout(((uint8_t)(addr>> 4))&(uint8_t)0x0F);	//  9.A07..A04
	lpcbus_cycleout(((uint8_t)(addr>> 0))&(uint8_t)0x0F);	// 10.A03..A00
	lpcbus_cycleout(data&0x0F);				// 11. D3..0
	lpcbus_cycleout((data>>4)&(uint8_t)0x0F);		// 12. D7..4
	lpcbus_cyclegivebus();					// 13. TAR0
	lpcbus_cyclenop();					// 14. TAR1
	lpcbus_cyclenop();					// 15. SYNC
	lpcbus_cyclenop();					// 16. TAR0
	lpcbus_cycletakebus();					// 17. TAR1
}
