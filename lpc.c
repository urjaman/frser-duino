/*
 * This file is part of the frser-avr-lpc project.
 *
 * Copyright (C) 2010 Mike Stirling
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
 * 
 * \file lpc.c
 * \brief LPC bus emulation state machine
 * 
 * \author	(C) 2010 Mike Stirling 
 * 			($Author$)
 * \version	$Revision$
 * \date	$Date$
 */

#include "main.h"
#include "flash.h"
#include "parallel.h"
#include "lpc.h"
#include "uart.h"

#define LPC_FRAME_PORT			PORTC
#define LPC_nLFRAM			PC7

#define LPC_CLK_PORT			PORTA
#define LPC_CLK				PA0

#define LPC_RESET_PORT			PORTB
#define LPC_nRESET			PB1


#define LPC_BL_ADDR 0xff000000

typedef enum {
	lpcReset = 0,
	lpcResetWait,
	lpcInit,
	lpcInitWait,
	lpcIdle,
	lpcStart,
	lpcCommand,
	lpcAddress,
	lpcDataOut,
	lpcTARtoInput,
	lpcSync,
	lpcDataIn,
	lpcTARtoOutput,
} lpc_state_t;

typedef struct {
	lpc_state_t		state;
	uint8_t			command;
	uint32_t		address;
	uint8_t			data;
	uint8_t			count;
	uint8_t			clock:1;
} lpc_t;
static lpc_t lpc;

#define LPC_MEMORY_READ		0x4
#define LPC_MEMORY_WRITE	0x6

#define LPC_RESET_COUNT		10
#define LPC_INIT_COUNT		10

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

static void lpc_databus_setdata(uint8_t data) {
	PORTC = ((PORTC & 0xFC) | ((data>>2) & 0x03));
	PORTD = ((PORTD & 0x3F) | ((data<<6) & 0xC0));
}

static void lpc_databus_out(void) {
	DDRC |= 0x03;
	DDRD |= 0xC0;
}

static void lpc_databus_output(uint8_t data) {
	lpc_databus_setdata(data);
	lpc_databus_out();
}

static void lpc_state_machine(void)
{	
	lpc.clock = !lpc.clock;
	if (lpc.clock) {
		LPC_CLK_PORT |= _BV(LPC_CLK);
	} else {
		LPC_CLK_PORT &= ~_BV(LPC_CLK);
	}
	
	// Data changes occur on the falling clock edges
	if (lpc.clock)
		return;
	
	switch (lpc.state) {
	case lpcReset:
		//TRACE("LPC reset started\n");
		LPC_RESET_PORT &= ~_BV(LPC_nRESET);
		lpc_databus_output(0);
		LPC_FRAME_PORT |= _BV(LPC_nLFRAM);
		lpc.count = LPC_RESET_COUNT;
		lpc.state = lpcResetWait;
		break;
	case lpcResetWait:
		lpc.count--;
		if (lpc.count == 0)
			lpc.state = lpcInit;
		break;
	case lpcInit:
		LPC_RESET_PORT |= _BV(LPC_nRESET);
		lpc.count = LPC_INIT_COUNT;
		lpc.state = lpcInitWait;
		break;
	case lpcInitWait:
		lpc.count--;
		if (lpc.count == 0) {
			//TRACE("LPC reset complete\n");
			lpc.state = lpcIdle;
		}
			
	case lpcIdle:
		lpc_databus_setdata(0);
		LPC_FRAME_PORT |= _BV(LPC_nLFRAM);
		break;
	case lpcStart:
		//TRACE("LPC transaction starting\n");
		lpc_databus_setdata(0);
		LPC_FRAME_PORT &= ~_BV(LPC_nLFRAM);
		lpc.state = lpcCommand;
		break;
	case lpcCommand:
		//TRACE("Command = %x\n",lpc.command);
		lpc_databus_setdata(lpc.command&0xf);
		LPC_FRAME_PORT |= _BV(LPC_nLFRAM);
		lpc.state = lpcAddress;
		lpc.count = 8; // address clocked out over 8 cycles
		break;
	case lpcAddress:
		//TRACE("Address byte = %x\n",lpc.address >> 28);
		lpc_databus_setdata((lpc.address >> 28)&0xf);
		lpc.address <<= 4;
		lpc.count--;
		if (lpc.count == 0) {
			lpc.state = (lpc.command == LPC_MEMORY_READ) ? lpcTARtoInput : lpcDataOut;
			lpc.count = 2;
		}
		break;
	case lpcDataOut:
		lpc_databus_setdata(lpc.data&0xf);
		lpc.data >>= 4;
		//TRACE("Data out = %x\n",LPC_PORT & 0xf);
		lpc.count--;
		if (lpc.count == 0) {
			lpc.state = lpcTARtoInput;
			lpc.count = 2;
		}
		break;
	case lpcTARtoInput:
		lpc_databus_setdata(0xf);
		lpc.count--;
		if (lpc.count == 1)
			lpc_databus_tristate();
		if (lpc.count == 0)
			lpc.state = lpcSync;
		break;
	case lpcSync:
		if ((lpc_databus_read()) == 5) {
			//TRACE("Device inserted wait state\n");
			break;
		}
		if ((lpc_databus_read()) != 0) {
			//ERROR("Bad sync word received 0x%x != 0\n",LPC_PIN & 0xf);
			lpc.state = lpcTARtoOutput; // must restore port to output mode
			lpc.count = 2;
			break;
		}
		lpc.state = (lpc.command == LPC_MEMORY_READ) ? lpcDataIn : lpcTARtoOutput;
		lpc.count = 2; // 2 nibbles
		break;
	case lpcDataIn:
		//TRACE("Data in = %x\n",LPC_PIN & 0xf);
		lpc.count--;
		if (lpc.count == 1)
			lpc.data = lpc_databus_read() & 0xf;
		if (lpc.count == 0) {
			lpc.data |= (lpc_databus_read() & 0xf) << 4;
			lpc.state = lpcTARtoOutput;
			lpc.count = 2;
		}
		break;
	case lpcTARtoOutput:
		lpc_databus_setdata(0xf);
		lpc.count--;
		if (lpc.count == 1)
			lpc_databus_out();
		if (lpc.count == 0)
			lpc.state = lpcIdle;
		break;
	default:
		lpc.state = lpcIdle;
		break;
	}
	
}

static void lpc_reset(void)
{
	lpc.state = lpcReset;
	while (lpc.state != lpcIdle)
		lpc_state_machine();
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

uint8_t lpc_test(void) {
	//uint16_t testdata;
//	if (parallel_test()) return 0; // If parallel test succeeds, not FWH/LPC
	lpc_init();
	PORTB &= ~_BV(1); //!RST
	PORTA &= ~_BV(7); //!INIT
	_delay_us(1);
	PORTB |= _BV(1);
	PORTA |= _BV(7);
	_delay_us(1);
	lpc_reset();
	//TODO: test LPC properly
	return 1;
}

uint8_t lpc_read(uint32_t address)
{
	address |= LPC_BL_ADDR;
	lpc.command = LPC_MEMORY_READ;
	lpc.address = address;
	lpc.state = lpcStart;
	while (lpc.state != lpcIdle)
		lpc_state_machine();
	return lpc.data;
}

void lpc_write(uint32_t address,uint8_t data)
{
	address |= LPC_BL_ADDR;
	lpc.command = LPC_MEMORY_WRITE;
	lpc.address = address;
	lpc.data = data;
	lpc.state = lpcStart;
	while (lpc.state != lpcIdle)
		lpc_state_machine();	
}

void lpc_readn(uint32_t addr, uint32_t len) {
	do {
	SEND(lpc_read(addr++));
	} while(--len);
}