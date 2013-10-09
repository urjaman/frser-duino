/*
	This file is part of bbflash.
	Copyright (C) 2013, Hao Liu and Robert L. Thompson

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
#include "lpc.h"
#include "parallel.h"
#include "typeu.h"

#define LPC_START 0b0000
#define LPC_CYCTYPE_READ 0b0100
#define LPC_CYCTYPE_WRITE 0b0110

#define LPC_BL_ADDR 0xff000000

bool lpc_init(void)
{
	return nibble_init();
}

void lpc_cleanup(void)
{
	nibble_cleanup();
}

static void lpc_start(void)
{
	nibble_start(LPC_START);
}

static void lpc_nibble_write(uint8_t value)
{
	clocked_nibble_write(value);
}

#define swap(x) do { asm volatile("swap %0" : "=r" (x) : "0" (x)); } while(0)

static void lpc_send_addr(uint32_t addr)
{
#if 0
	int8_t i;
	addr |= LPC_BL_ADDR;
	for (i = 28; i >= 0; i -= 4)
		lpc_nibble_write((addr >> i) & 0xf);
#else
	uint8_t tmp;
	u32_u a;
	a.l = addr;
	/* NOTE: revise this if LPC_BL_ADDR changes. */
	lpc_nibble_write(0xF);
	lpc_nibble_write(0xF);
	tmp = a.b[2];
	swap(tmp);
	lpc_nibble_write(tmp);
	//swap(a.b[2]);
	lpc_nibble_write(a.b[2]);
	tmp = a.b[1];
	swap(tmp);
	lpc_nibble_write(tmp);
	//swap(a.b[1]);
	lpc_nibble_write(a.b[1]);
	tmp = a.b[0];
	swap(tmp);
	lpc_nibble_write(tmp);
//	swap(a.b[0]);
	lpc_nibble_write(a.b[0]);
#endif
}

int lpc_read_address(uint32_t addr)
{
	lpc_start();
	lpc_nibble_write(LPC_CYCTYPE_READ);
	lpc_send_addr(addr);
	nibble_set_dir(INPUT);
	clock_cycle();
	if (!nibble_ready_sync())
		return -1;
	uint8_t byte = byte_read();
	clock_cycle();
	clock_cycle();
	clock_cycle();
	return byte;
}

bool lpc_write_address(uint32_t addr, uint8_t byte)
{
	lpc_start();
	lpc_nibble_write(LPC_CYCTYPE_WRITE);
	lpc_send_addr(addr);
	byte_write(byte);
	nibble_set_dir(INPUT);
	clock_cycle();
	clock_cycle();
	if (!nibble_ready_sync())
		return false;
	clock_cycle();
//	clock_cycle(lpc->clock);
	return true;
}



uint8_t lpc_test(void) {
	if (parallel_test()) return 0; // If parallel test succeeds, not FWH/LPC
	nibble_hw_init();
	PORTB &= ~_BV(1); //!RST
	_delay_us(1);
	PORTB |= _BV(1);
	lpc_init();
	if (lpc_read_address(0xFFFFFFFF)==-1) return 0;
	return 1;
}
