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

uint8_t spi_test(void);
uint8_t spi_read(uint32_t addr);
void spi_readn(uint32_t addr, uint32_t len);
void spi_spiop(uint32_t sbytes, uint32_t rbytes);
int spi_uninit(void);
void spi_init_cond(void);
uint32_t spi_set_speed(uint32_t hz);
