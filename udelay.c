/*
 * This file is part of the frser-duino project.
 *
 * Copyright (C) 2009 Urja Rannikko <urjaman@gmail.com>
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
#include "udelay.h"

void udelay(uint32_t usecs) {
	if (usecs < 16) {
		uint8_t i=usecs;
		do { _delay_us(1); } while(--i);
		return;
	}
	usecs >>= 4; // div 16;
	do { _delay_us(16); } while(--usecs);
	return;
}
