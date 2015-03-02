/*
 * This file is part of the frser-duino project.
 *
 * Copyright (C) 2010 Urja Rannikko <urjaman@gmail.com>
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

/* GENERIC DEFINITIONS FOR PROJECT */
#define F_CPU 16000000UL
#include <util/delay.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <avr/pgmspace.h>
#include <avr/eeprom.h>
#include <avr/power.h>
#include <avr/sleep.h>
#include <util/delay.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <alloca.h>
#include <inttypes.h>
#include <setjmp.h>
