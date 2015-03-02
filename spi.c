/*
 * This file is part of the frser-atmega644 project.
 *
 * Copyright (C) 2009,2011 Urja Rannikko <urjaman@gmail.com>
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

static uint8_t spi_set_ubrr = 0; /* Max speed - F_CPU/2 */

uint32_t spi_set_speed(uint32_t hz) {
	/* Range check. */
	if (hz>(F_CPU/2)) {
		spi_set_ubrr = 0;
		return F_CPU/2;
	}
	if (hz<(F_CPU/512)) {
		spi_set_ubrr = 255;
		return F_CPU/512;
	}

	uint32_t bdiv = hz*2;
	uint32_t ubrr_vp = (F_CPU / bdiv)-1;
	// If the division is not perfect, increase the result (round down).
	if (F_CPU%bdiv) ubrr_vp++;

	spi_set_ubrr = ubrr_vp;

	uint32_t new_hz = F_CPU / ((ubrr_vp+1)*2);
	return new_hz;
}

static void spi_select(void) {
	UBRR1 = 0;
	UCSR1B = _BV(TXEN1)|_BV(RXEN1);
	UBRR1 = (uint16_t)spi_set_ubrr;
	PORTB &= ~_BV(4);
	DDRB |= _BV(4);
}

static void spi_deselect(void) {
	PORTB |= _BV(4);
	UCSR1B = 0;
}

static uint8_t spi_txrx(const uint8_t c) {
	UDR1 = c;
	while (!(UCSR1A&_BV(RXC1)));
	return UDR1;
}


static void spi_init(void) {
	PORTB |= _BV(3)|_BV(4);
	DDRB |= _BV(3)|_BV(4); // !WP&!CS high
	PORTA |= _BV(1);
	DDRA |= _BV(1); // !HOLD high
	DDRD = ( DDRD & ~_BV(2)) | _BV(3) | _BV(4);
	PORTD =  ( PORTD &  ~( _BV(3) | _BV(4) ) ) | _BV(2);
	// DO/RXD (2), DI/TXD (3) and CLK (4) correctly. Pullup on RXD for 0xFF.
	UCSR1C = _BV(UMSEL11)|_BV(UMSEL10);
}




void spi_init_cond(void) {
	if (!spi_initialized) {
		spi_init();
		spi_initialized = 1;
	}
}

uint8_t spi_uninit(void) {
	if (spi_initialized) {
		UCSR1C = 0;
		DDRB &= ~_BV(3); // !WP
		DDRB &= ~_BV(4); // !CS
		DDRA &= ~_BV(1); // !HOLD
		DDRD &= ~_BV(3); // MOSI
		DDRD &= ~_BV(4); // SCLK
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
