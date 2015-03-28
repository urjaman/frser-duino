/*
 * This file is part of the frser-duino project.
 *
 * Copyright (C) 2015 Urja Rannikko <urjaman@gmail.com>
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
#include "spilib.h"

/* This is just a bit of glue between parts of libfrser. */

void flash_set_safe(void) {
	spi_uninit();
	DDR_SPI &= ~_BV(MOSI);
	DDR_SPI &= ~_BV(SCK);
	DDR_SPI &= ~_BV(SS);
}

void flash_select_protocol(uint8_t allowed_protocols) {
	(void)allowed_protocols;
	SPI_PORT |= _BV(SS);
	SPI_PORT &= ~_BV(MOSI);
	SPI_PORT &= ~_BV(SCK);
	DDR_SPI = (1<<MOSI)|(1<<SCK)|(1<<SS);
	spi_init();
}

void flash_spiop(uint32_t s, uint32_t r) {
	spi_spiop(s,r);
}
