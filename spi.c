/*
 * This file is part of the frser-duino project.
 *
 * Copyright (C) 2009,2011.2015 Urja Rannikko <urjaman@gmail.com>
 * Based on the SPI code that is:
 * Copyright (C) 2012  Denis 'GNUtoo' Carikli <GNUtoo@no-log.org>
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
#include "spi.h"
#include "uart.h"
#include "frser.h"

static int spi_initialized = 0;

static uint8_t spi_set_spd = 0; /* Max speed - F_CPU/2 */

const uint8_t PROGMEM spd_table[7] = {
	0x80, // SPI2X is 0x80 in this table because i say so. - div 2
	0, // div 4
	0x80 | _BV(SPR0), // div 8
	_BV(SPR0), // div 16
	0x80 | _BV(SPR1), // div 32
	_BV(SPR1), // div64
	_BV(SPR1) | _BV(SPR0), //div128
};

const uint32_t PROGMEM spd_hz_table[7] = {
	F_CPU/2, F_CPU/4, F_CPU/8, F_CPU/16, F_CPU/32, F_CPU/64, F_CPU/128
};

uint32_t spi_set_speed(uint32_t hz) {
	/* We can set F_CPU / : 2,4,8,16,32,64,128 */

	uint8_t spd;
	uint32_t hz_spd;

	/* Range check. */
	if (hz<=(F_CPU/128)) {
		spd = 6;
		hz_spd = F_CPU/128;
	} else {
		for (uint8_t spd=0;spd<7;spd++) {
			hz_spd = pgm_read_dword(&(spd_hz_table[spd]));
			if (hz >= hz_spd) break;
		}
	}
	spi_set_spd = spd;
	if (spi_initialized) { // change speed
		uint8_t spdv = pgm_read_byte(&(spd_table[spi_set_spd]));
		SPCR = (SPCR & 0xFC) | (spdv & 0x3);
		if (spdv&0x80) SPSR |= _BV(SPI2X);
		else SPSR &= ~_BV(SPI2X);
	}
	return hz_spd;
}

#define SPI_PORT	PORTB
#define SCK		PORTB5		/* port 13 */
#define MISO		PORTB4		/* port 12 */
#define MOSI 		PORTB3		/* port 11 */
#define SS		PORTB2		/* port 10 */
#define DDR_SPI		DDRB

static void spi_select(void) {
	SPI_PORT &= ~(1<<SS);
}

void spi_deselect(void) {
	SPI_PORT |= (1<<SS);
}

void spi_init(void) {
	/* set SS low */
	SPI_PORT &= ~(1<<SS);
	/* Enable MOSI,SCK,SS as output like on
	http://en.wikipedia.org/wiki/File:SPI_single_slave.svg */
	DDR_SPI = (1<<MOSI)|(1<<SCK)|(1<<SS);
	/* Enable SPI Master, set the clock what was asked. */
	/* CPOL and CPHA are 0 for SPI mode 0 (see wikipedia) */
	/* we use mode 0 like for the linux spi in flashrom*/
	uint8_t spdv = pgm_read_byte(&(spd_table[spi_set_spd]));
	SPCR = (1<<SPE)|(1<<MSTR) | (spdv & 0x03);
	if (spdv&0x80) SPSR |= _BV(SPI2X);
	else SPSR &= ~_BV(SPI2X);
}

static uint8_t spi_txrx(const uint8_t c) {
	uint8_t r;

	/* transmit c on the SPI bus */
	SPDR = c;
	/* Wait for the transmission to be completed */
	loop_until_bit_is_set(SPSR,SPIF);
	r = SPDR;
	return r;
}


void spi_init_cond(void) {
	if (!spi_initialized) {
		spi_init();
		spi_initialized = 1;
	}
}

uint8_t spi_uninit(void) {
	if (spi_initialized) {
		SPCR = 0;
		DDR_SPI &= ~_BV(MOSI);
		DDR_SPI &= ~_BV(SCK);
		DDR_SPI &= ~_BV(SS);
		spi_initialized = 0;
		return 1;
	}
	return 0;
}


void spi_spiop(uint32_t sbytes, uint32_t rbytes) {
	spi_init_cond();
	spi_select();
	while (sbytes--) spi_txrx(RECEIVE());
	SEND(S_ACK);
	while (rbytes--) SEND(spi_txrx(0xFF));
	spi_deselect();
}
