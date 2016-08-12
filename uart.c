/*
 * This file is part of the frser-duino project.
 *
 * Copyright (C) 2009,2011,2013,2016 Urja Rannikko <urjaman@gmail.com>
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
#include "uart.h"

#if UART_BUFLEN > 256
typedef uint16_t urxbufoff_t;
#else
typedef uint8_t urxbufoff_t;
#endif

static uint8_t volatile uart_rcvbuf[UART_BUFLEN];
static urxbufoff_t volatile uart_rcvwptr;
static urxbufoff_t uart_rcvrptr;

#if ((UART_BUFLEN-1) & UART_BUFLEN) == 0
#define RLIM_RL(r) do { r &= (UART_BUFLEN-1); } while(0)
#else
#define RLIM_RL(r) do { if (r>=UART_BUFLEN) r=0; } while(0)
#endif

#if (defined __AVR_ATmega328P__)||(defined __AVR_ATmega328__)||(defined __AVR_ATmega168P__)||(defined __AVR_ATmega168__)||(__AVR_ATmega88P__)||(__AVR_ATmega88__)||(defined __AVR_ATmega48P__)||(defined __AVR_ATmega48__)
#define RX_ISR USART_RX_vect
#else
/* We assume this is a newer avr with fancier USART ISR names, fix if it warns :) */
#define RX_ISR USART0_RX_vect
#endif

ISR(RX_ISR) {
	urxbufoff_t reg = uart_rcvwptr;
	uart_rcvbuf[reg++] = UDR0;
	RLIM_RL(reg);
	uart_rcvwptr = reg;
}

uint8_t uart_isdata(void) {
	cli();
	urxbufoff_t rcvw = uart_rcvwptr;
	sei();
	if (rcvw != uart_rcvrptr) return 1;
	return 0;
}

static void uart_waiting(void) {
	cli();
	if (uart_rcvwptr == uart_rcvrptr) { /* Race condition check */
		sleep_enable();
		sei();
		sleep_cpu();
		sleep_disable();
	}
	sei();
}


uint8_t uart_recv(void) {
	urxbufoff_t reg;
	uint8_t val;
	while (!uart_isdata()) uart_waiting();
	reg = uart_rcvrptr;
	val = uart_rcvbuf[reg++];
	RLIM_RL(reg);
	uart_rcvrptr = reg;
	return val;
}

void uart_send(uint8_t val) {
	while (!(UCSR0A & _BV(UDRE0))); // wait for space in reg
	/* FTDI basically means a properly working USB-serial converter at whatever baudrate you chose. */
#ifndef FTDI
	_delay_us(10); // U2 hack
#endif
	UDR0 = val;
}

void uart_init(void) {
	cli();

#include <util/setbaud.h>
// Assuming uart.h defines BAUD
	uart_rcvwptr = 0;
	uart_rcvrptr = 0;
#ifndef UART_POLLED_TX
	uart_sndwptr = 0;
	uart_sndrptr = 0;
#endif
	UBRR0H = UBRRH_VALUE;
	UBRR0L = UBRRL_VALUE;
	UCSR0C = 0x06; // Async USART,No Parity,1 stop bit, 8 bit chars
	UCSR0A &= 0xFC;
#if USE_2X
	UCSR0A |= (1 << U2X0);
#endif
	UCSR0B = 0x98; // RXC int, receiver and transmitter
	sei();
}

void uart_wait_txdone(void) {
}
