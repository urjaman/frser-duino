/*
	This file was part of bbflash, now frser-atmega644.
	Copyright (C) 2013, Hao Liu and Robert L. Thompson
	Copyright (C) 2013, Urja Rannikko <urjaman@gmail.com>

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
#include "fwh.h"
#include "parallel.h"

#define FWH_START_READ 0b1101
#define FWH_START_WRITE 0b1110
#define FWH_ABORT 0b1111

#define FWH_BL_ADDR 0xff000000


bool fwh_init(void)
{
	return nibble_init();
}

void fwh_cleanup(void)
{
	nibble_cleanup();
}

static void fwh_nibble_write(uint8_t value)
{
	clocked_nibble_write(value);
}

static void fwh_start(uint8_t start)
{
	nibble_start(start);
}

static void fwh_send_imaddr(uint32_t addr)
{
	int8_t i;
	addr |= FWH_BL_ADDR;
	for (i = 24; i >= 0; i -= 4)
		fwh_nibble_write((addr >> i) & 0xf); /* That shift is evil. Fix later. */
}

int fwh_read_address(uint32_t addr)
{
	fwh_start(FWH_START_READ);
	fwh_nibble_write(0);	/* IDSEL hardwired */
	fwh_send_imaddr(addr);
	fwh_nibble_write(0);	/* IMSIZE single byte */
	nibble_set_dir(INPUT);
	clock_cycle();
	if (!nibble_ready_sync())
		return -1;
	uint8_t byte = byte_read();
	clock_cycle();
	nibble_set_dir(OUTPUT);
	fwh_nibble_write(0xf);
	clock_cycle();
	return byte;
}

bool fwh_write_address(uint32_t addr, uint8_t byte)
{
	fwh_start(FWH_START_WRITE);
	fwh_nibble_write(0);	/* IDSEL hardwired */
	fwh_send_imaddr(addr);
	fwh_nibble_write(0);	/* IMSIZE single byte */
	byte_write(byte);
	nibble_write(0xf);
	nibble_set_dir(INPUT);
	clock_cycle();
	clock_cycle();
	if (!nibble_ready_sync())
		return false;
	clock_cycle();
	return true;
}

uint8_t fwh_test(void) {
	if (parallel_test()) return 0; // If parallel test succeeds, not FWH/LPC
	nibble_hw_init();
	PORTB &= ~_BV(1); //!RST
	_delay_us(1);
	PORTB |= _BV(1);
	fwh_init();
	if (fwh_read_address(0xFFFFFFFF)==-1) return 0;
	return 1;
}
