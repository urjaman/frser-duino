/*!
 * This file is part of the frser-avr-lpc project.
 *
 * Copyright (C) 2010 Mike Stirling
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
 * 
 * \file lpc.c
 * \brief LPC bus emulation state machine
 * 
 * \author	(C) 2010 Mike Stirling 
 * 			($Author$)
 * \version	$Revision$
 * \date	$Date$
 */
#ifndef LPC_H_
#define LPC_H_

#include <stdint.h>

uint8_t lpc_test(void);
uint8_t lpc_read(uint32_t address);
void lpc_readn(uint32_t addr, uint32_t len);
void lpc_write(uint32_t address,uint8_t data);

#endif



