/*
 * This file is part of the frser-atmega644 project.
 *
 * Copyright (C) 2013 Urja Rannikko <urjaman@gmail.com>
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
#include "console.h"
#include "appdb.h"
#include "lib.h"
#include "ciface.h"

#define RECVBUFLEN 64

const unsigned char prompt[] PROGMEM = "\x0D\x0A >";
unsigned char recvbuf[RECVBUFLEN];
unsigned char token_count;
unsigned char* tokenptrs[MAXTOKENS] __attribute__((aligned(16)));


void ciface_main(void) {
	void(*func)(void);
	for (;;) {
		sendstr_P((PGM_P)prompt);
		if (getline(recvbuf,RECVBUFLEN)) return;
		tokenize(recvbuf,tokenptrs, &token_count);
		if (token_count) {
			func = find_appdb(tokenptrs[0]);
			func();
		}
	}
}
