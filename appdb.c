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
#include "ciface.h"
#include "console.h"
#include "appdb.h"

const unsigned char echostr[] PROGMEM = "ECHO";
const unsigned char readsectstr[] PROGMEM = "READSECT";
const unsigned char flashidstr[] PROGMEM = "IDCHIP";
const unsigned char qprotostr[] PROGMEM = "QPROTO";
const unsigned char sprotostr[] PROGMEM = "SPROTO";
const unsigned char spiidstr[] PROGMEM = "SPI-ID";
const unsigned char spitststr[] PROGMEM = "SPITEST";
const unsigned char partststr[] PROGMEM = "PARTEST";
const unsigned char lpctststr[] PROGMEM = "LPCTEST";
const unsigned char fwhtststr[] PROGMEM = "FWHTEST";
const unsigned char frlaopstr[] PROGMEM = "FRSLAOP";
const unsigned char bljumpstr[] PROGMEM = "BLJUMP";
const unsigned char calcstr[] PROGMEM = "CALC";
const unsigned char helpstr[] PROGMEM = "?";

const struct command_t appdb[] PROGMEM = {
	{(PGM_P)echostr, &(echo_cmd)},
	{(PGM_P)readsectstr, &(flash_readsect_cmd)},
	{(PGM_P)flashidstr, &(flash_idchip_cmd)},
	{(PGM_P)qprotostr, &(flash_proto_cmd)},
	{(PGM_P)sprotostr, &(flash_sproto_cmd)},
	{(PGM_P)spiidstr, &(spi_id_cmd)},
	{(PGM_P)spitststr, &(spi_test_cmd)},
	{(PGM_P)partststr, &(par_test_cmd)},
	{(PGM_P)lpctststr, &(lpc_test_cmd)},
	{(PGM_P)fwhtststr, &(fwh_test_cmd)},
	{(PGM_P)frlaopstr, &(frser_last_op_cmd)},
	{(PGM_P)bljumpstr, &(bljump_cmd)},
	{(PGM_P)calcstr, &(calc_cmd)},
	{(PGM_P)helpstr, &(help_cmd)},
	{NULL,NULL}
};

void invalid_command(void) {
	sendstr(tokenptrs[0]);
	sendstr_P(PSTR(": not found"));
	}

void *find_appdb(unsigned char* cmd) {
	unsigned char i;
	const struct command_t * ctptr;
	PGM_P name;
	void* fp;
	for(i=0;;i++) {
		ctptr = &(appdb[i]);
		name = (PGM_P)pgm_read_word(&(ctptr->name));
		fp = (void*)pgm_read_word(&(ctptr->function));
		if (!name) break;
		if (strcmp_P((char*)cmd,name) == 0) {
			return fp;
			}
	}
	return &(invalid_command);
}


