/*
 * This file is part of the frser-atmega644 project.
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

/* Define one to make a single bustype programmer. */
//#define FORCE_BUSTYPE CHIP_BUSTYPE_PARALLEL
//#define FORCE_BUSTYPE CHIP_BUSTYPE_LPC
//#define FORCE_BUSTYPE CHIP_BUSTYPE_FWH
//#define FORCE_BUSTYPE CHIP_BUSTYPE_SPI

#define CHIP_BUSTYPE_PARALLEL (1 << 0)
#define CHIP_BUSTYPE_LPC (1 << 1)
#define CHIP_BUSTYPE_FWH (1 << 2)
#define CHIP_BUSTYPE_SPI (1 << 3)

#ifdef FORCE_BUSTYPE
#define SUPPORTED_BUSTYPES FORCE_BUSTYPE
#else
#define SUPPORTED_BUSTYPES (CHIP_BUSTYPE_PARALLEL|CHIP_BUSTYPE_LPC|CHIP_BUSTYPE_FWH|CHIP_BUSTYPE_SPI)
#endif

void flash_set_safe(void);
void flash_select_protocol(uint8_t allowed_protocols);
uint8_t flash_read(uint32_t addr);
void flash_readn(uint32_t addr, uint32_t len);
void flash_write(uint32_t addr, uint8_t data);
void flash_spiop(uint32_t sbytes, uint32_t rbytes);
uint8_t flash_get_proto(void);
uint8_t flash_idle_clock(void);
