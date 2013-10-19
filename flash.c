/*
 * This file is part of the frser-atmega644 project.
 *
 * Copyright (C) 2010,2011 Urja Rannikko <urjaman@gmail.com>
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
#include "uart.h"
#include "parallel.h"
#include "lpc.h"
#include "spi.h"
#include "frser.h"
#include "fwh.h"
#include "nibble.h"

static uint8_t flash_prot_in_use=0;

static void flash_portclear(void) {
// So that every test function can assume this reset/safe setup.
	DDRA=0;
	DDRB=0;
	DDRC=0;
	DDRD=0;
	PORTA=0;
	PORTB=0;
	PORTC=0;
	PORTD=0;
}

void flash_set_safe(void) {
	flash_portclear();
}

//#define SD(x) SEND(X)
#define SD(x)

uint8_t flash_get_proto(void) {
	return flash_prot_in_use;
}

uint8_t flash_idle_clock(void) {
#if 1
	if (flash_prot_in_use&(CHIP_BUSTYPE_LPC|CHIP_BUSTYPE_FWH)) {
		clock_cycle();
		return 1;
	}
#endif
	return 0;
}

uint8_t flash_plausible_protocols(void) {
	uint8_t protocols = SUPPORTED_BUSTYPES;
	flash_portclear();
	if (spi_test()) {
		// 0 or SPI, because using parallel while SPI chip is attached is potentially dangerous.
		protocols &= CHIP_BUSTYPE_SPI;
		return protocols;
	}
	flash_portclear();
	if ((protocols&CHIP_BUSTYPE_PARALLEL)&&(!parallel_test())) {
		protocols &= ~CHIP_BUSTYPE_PARALLEL;
	}
	flash_portclear();
	if ((protocols&CHIP_BUSTYPE_LPC)&&(!lpc_test())) {
		protocols &= ~CHIP_BUSTYPE_LPC;
	}
	flash_portclear();
	if ((protocols&CHIP_BUSTYPE_FWH)&&(!fwh_test())) {
		protocols &= ~CHIP_BUSTYPE_FWH;
	}
	flash_select_protocol(flash_prot_in_use);
	return protocols;
}

void flash_select_protocol(uint8_t allowed_protocols) {
	allowed_protocols &= SUPPORTED_BUSTYPES;
	flash_portclear();
	if (spi_test()) {
		// 0 or SPI, because using parallel while SPI chip is attached is potentially dangerous.
		flash_prot_in_use = allowed_protocols&CHIP_BUSTYPE_SPI;
		return;
	}
	flash_portclear();
	if ((allowed_protocols&CHIP_BUSTYPE_PARALLEL)&&(parallel_test())) {
		flash_prot_in_use = CHIP_BUSTYPE_PARALLEL;
		return;
	}
	flash_portclear();
	if ((allowed_protocols&CHIP_BUSTYPE_LPC)&&(lpc_test())) {
		flash_prot_in_use = CHIP_BUSTYPE_LPC;
		return;
	}
	flash_portclear();
	if ((allowed_protocols&CHIP_BUSTYPE_FWH)&&(fwh_test())) {
		flash_prot_in_use = CHIP_BUSTYPE_FWH;
		return;
	}
	flash_prot_in_use = 0;
	return;
}

static void spi_uninit_check(void)
{
	if ((flash_prot_in_use!=CHIP_BUSTYPE_SPI)&&((SUPPORTED_BUSTYPES) & CHIP_BUSTYPE_SPI)) {
		if (spi_uninit()) {
			flash_portclear();
			switch (flash_prot_in_use) {
				case 0:
				default:
					break;
				case CHIP_BUSTYPE_PARALLEL:
					parallel_test();
					break;
				
				case CHIP_BUSTYPE_LPC:
					lpc_test();
					break;

				case CHIP_BUSTYPE_FWH:
					fwh_test();
					break;
			}
		}
	}
}

uint8_t flash_read(uint32_t addr) {
	spi_uninit_check();
	switch (flash_prot_in_use) {
		case 0:
		default:
			return 0xFF;
		case CHIP_BUSTYPE_PARALLEL:
			return parallel_read(addr);
		case CHIP_BUSTYPE_LPC:
			return lpc_read_address(addr);
		case CHIP_BUSTYPE_FWH:
			return fwh_read_address(addr);
		case CHIP_BUSTYPE_SPI:
			return spi_read(addr);
	}
}

void flash_readn(uint32_t addr, uint32_t len) {
	spi_uninit_check();
	if (len==0) len = ((uint32_t)1<<24);
	switch (flash_prot_in_use) {
		case 0:
		default:
			while (len--) SEND(0xFF);
			return;
		case CHIP_BUSTYPE_PARALLEL:
			parallel_readn(addr,len);
			return;
		case CHIP_BUSTYPE_LPC:
			while (len--) SEND(lpc_read_address(addr++));
			return;
		case CHIP_BUSTYPE_FWH:
			while (len--) SEND(fwh_read_address(addr++));
			return;
		case CHIP_BUSTYPE_SPI:
			spi_readn(addr,len);
			return;
	}
}

void flash_write(uint32_t addr, uint8_t data) {
	spi_uninit_check();
	switch (flash_prot_in_use) {
		case 0:
		default:
			return;
		case CHIP_BUSTYPE_PARALLEL:
			parallel_write(addr,data);
			return;
		case CHIP_BUSTYPE_LPC:
			lpc_write_address(addr,data);
			return;
		case CHIP_BUSTYPE_FWH:
			fwh_write_address(addr,data);
			return;
	}
}

	
void flash_spiop(uint32_t sbytes, uint32_t rbytes) {
	if ((SUPPORTED_BUSTYPES) & CHIP_BUSTYPE_SPI) {
		spi_init_cond();
		spi_spiop(sbytes,rbytes);
		return;
	} else {
		while (sbytes--) RECEIVE();
		SEND(S_ACK);
		while (rbytes--) SEND(0xFF);
		return;
	}
}
