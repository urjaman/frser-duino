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

#define lpc_nibble_write(v) clocked_nibble_write(v)
#define lpc_nibble_write_hi(v) clocked_nibble_write_hi(v)


static void lpc_send_addr(uint32_t addr)
{
#if 0
	int8_t i;
	addr |= LPC_BL_ADDR;
	for (i = 28; i >= 0; i -= 4)
		lpc_nibble_write((addr >> i) & 0xf);
#else
	u32_u a;
	a.l = addr;
	/* NOTE: revise this if LPC_BL_ADDR changes. */
	lpc_nibble_write(0xF);
	//lpc_nibble_write(0xF);
	clock_cycle();
	lpc_nibble_write_hi(a.b[2]);
	lpc_nibble_write(a.b[2]);
	lpc_nibble_write_hi(a.b[1]);
	lpc_nibble_write(a.b[1]);
	lpc_nibble_write_hi(a.b[0]);
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
