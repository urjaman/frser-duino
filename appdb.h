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

struct command_t {
	PGM_P name;
	void(*function)(void);
};

extern const struct command_t appdb[] PROGMEM;

void *find_appdb(unsigned char* cmd);
void echo_cmd(void);
void help_cmd(void);
void flash_readsect_cmd(void);
void flash_idchip_cmd(void);
void calc_cmd(void);
void flash_proto_cmd(void);
void bljump_cmd(void);
void spi_id_cmd(void);
void spi_test_cmd(void);
void par_test_cmd(void);
void lpc_test_cmd(void);
void fwh_test_cmd(void);
