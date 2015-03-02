/*
 * This file is part of the frser-duino project.
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

/* UART MODULE HEADER */
uint8_t uart_isdata(void);
uint8_t uart_recv(void);
void uart_send(uint8_t val);
void uart_init(void);
void uart_wait_txdone(void);

/* We assume to other half to be perfect.... */
#define BAUD_TOL 3

#define RECEIVE() uart_recv()
#define SEND(n) uart_send(n)
#define UART_BUFLEN 1024
/* Compat; Int Tx support was stripped. */
#define UART_POLLED_TX
#define UARTTX_BUFLEN 0
