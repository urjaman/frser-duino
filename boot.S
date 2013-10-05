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

#include <avr/io.h>
#define RAMSTART 0x100
#if SPM_PAGESIZE > 256
#error Too big SPM pagesize for the protocol
#endif
Xlo = 26
Xhi = 27
Ylo = 28
Yhi = 29
Zlo = 30
Zhi = 31

#define UART_UBRRL 12
#define UART_UBRRH 0
// 115384 baud at 12 Mhz
#define TAG_CHAR 0x55
#define INIT_CMD 0xAA
#define EXIT_CMD 0xFF

.section .bootloader, "ax"
	.global boot
boot:
	cli
	rcall uart_init
	rcall uart_get
	cpi r16, INIT_CMD
	brne out
	ldi r16, TAG_CHAR
	rcall uart_send
#if SPM_PAGESIZE == 256
	clr r16
#else
	ldi r16, SPM_PAGESIZE
#endif
	rcall uart_send

bootlp:
	rcall uart_get
	cpi r16, EXIT_CMD
	breq out

	mov Zlo, r16

#if SPM_PAGESIZE == 64
	mov Zhi, Zlo
	swap Zlo
	lsl Zlo
	lsl Zlo
	andi Zlo, 0xC0
	lsr Zhi
	lsr Zhi
#elif SPM_PAGESIZE == 128
	mov Zhi, Zlo
	clr Zlo
	lsr Zhi
	ror Zlo
#elif SPM_PAGESIZE == 256
	mov Zhi, Zlo
	clr Zlo
#else
#error unexpected SPM_PAGESIZE
#endif

	movw Xlo, Zlo // Save Z in X
#if SPM_PAGESIZE == 256
	clr r20
#else
	ldi r20, SPM_PAGESIZE
#endif
	ldi Ylo, lo8(RAMSTART)
	ldi Yhi, hi8(RAMSTART)
dataloop:
	rcall uart_get
	st Y+, r16
	dec r20
	brne dataloop
	rcall wait_spm_ready
	ldi r16, 0x03 // erase
	out _SFR_IO_ADDR(SPMCSR), r16
	spm
	rcall wait_spm_ready
	ldi r20, (SPM_PAGESIZE/2)
	ldi Ylo, lo8(RAMSTART)
	ldi Yhi, hi8(RAMSTART)
	ldi r16, 0x01
pageload:
	ld r0, Y+
	ld r1, Y+
	out _SFR_IO_ADDR(SPMCSR), r16
	spm
	adiw r30, 2
	dec r20
	brne pageload
	movw r30, r26 // Restore Z from X
	ldi r16, 0x05
	out _SFR_IO_ADDR(SPMCSR), r16
	spm
	ldi r16, TAG_CHAR
	rcall uart_send
	rjmp bootlp
out:
	rcall wait_spm_ready
	ldi r16, 0x11
	out _SFR_IO_ADDR(SPMCSR), r16
	spm
	rcall wait_spm_ready
	jmp entry
.set entry,0

wait_spm_ready:
	in r16, _SFR_IO_ADDR(SPMCSR)
	sbrc r16, SPMEN
	rjmp wait_spm_ready
	ret

uart_get:
	lds r16, _SFR_MEM_ADDR(UCSR0A)
	sbrs r16, RXC0
	rjmp uart_get
	lds r16,  _SFR_MEM_ADDR(UDR0)
	ret

uart_send:
	lds r17, _SFR_MEM_ADDR(UCSR0A)
	sbrs r17, UDRE0
	rjmp uart_send
	sts _SFR_MEM_ADDR(UDR0), r16
	ret

uart_init:
	ldi r16, UART_UBRRH
	sts _SFR_MEM_ADDR(UBRR0H), r16
	ldi r16, UART_UBRRL
	sts _SFR_MEM_ADDR(UBRR0L), r16
	ldi r16, 0x02 // U2X mode
	sts _SFR_MEM_ADDR(UCSR0A), r16
	ldi r16, 0x06
	sts _SFR_MEM_ADDR(UCSR0C), r16
	ldi r16, 0x18
	sts _SFR_MEM_ADDR(UCSR0B), r16 // uart enabled
	ret

