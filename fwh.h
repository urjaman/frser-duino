/*
	This file was part of bbflash, now part of frser-atmega644.
	Copyright (C) 2013, Hao Liu and Robert L. Thompson
	Copyright (C) 2013 Urja Rannikko <urjaman@gmail.com>

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef FWH_H_
#define FWH_H_

#include "mybool.h"

uint8_t fwh_test(void);
bool fwh_init(void);
void fwh_cleanup(void);
int fwh_read_address(uint32_t addr);
bool fwh_write_address(uint32_t addr, uint8_t byte);

#endif /* FWH_H_ */
