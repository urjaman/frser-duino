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

void uint2str(unsigned char *buf, unsigned int val);
void uchar2str(unsigned char *buf, unsigned char val);
void uchar2xstr(unsigned char *buf, unsigned char val);
unsigned char str2uchar(unsigned char *buf);
unsigned char xstr2uchar(unsigned char *buf);
unsigned long int astr2luint(unsigned char *buf);
void uint2xstr(unsigned char *buf, unsigned int val);
unsigned char bcd2bin(unsigned char bcd);
unsigned char bin2bcd(unsigned char bin);
unsigned long int astr2luint(unsigned char *buf);
void luint2str(unsigned char *buf, unsigned long int val);
void luint2xstr(unsigned char*buf, unsigned long int val);
