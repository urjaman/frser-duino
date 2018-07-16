##
## This file is part of the frser-duino project.
##
## Copyright (C) 2010,2011,2015 Urja Rannikko <urjaman@gmail.com>
##
## This program is free software; you can redistribute it and/or modify
## it under the terms of the GNU General Public License as published by
## the Free Software Foundation; either version 2 of the License, or
## (at your option) any later version.
##
## This program is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
## GNU General Public License for more details.
##
## You should have received a copy of the GNU General Public License
## along with this program; if not, write to the Free Software
## Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
##

PROJECT=frser-duino
DEPS=uart.h main.h Makefile
SOURCES=main.c uart.c spihw.c
CC=avr-gcc
LD=avr-ld
OBJCOPY=avr-objcopy
MMCU ?= atmega328p
AVRDUDE_MCU ?= m328p
AVRDUDE_PROGRAMMER ?= arduino
# These defaults are for the U2-equipped arduino (m328p),
# feel free to change.

#Device
SERIAL_DEV ?= /dev/ttyACM0
# Bootloader
BLBAUD ?= 115200
# Flashrom serial (=serprog)
FRBAUD ?= 115200
#Additional defines (used by make ftdi)
DFLAGS ?=

AVRDUDECMD=avrdude -c $(AVRDUDE_PROGRAMMER) -p $(AVRDUDE_MCU) -P $(SERIAL_DEV) -b $(BLBAUD)

#AVRBINDIR=/usr/avr/bin/

CFLAGS=-mmcu=$(MMCU) -DBAUD=$(FRBAUD) -Os -fno-inline-small-functions -fno-tree-switch-conversion -frename-registers -g -Wall -W -pipe -flto -fwhole-program -std=gnu99 $(DFLAGS)

include libfrser/Makefile.frser
include libfrser/Makefile.spilib
include libfrser/Makefile.spihw_avrspi


all: $(PROJECT).out

$(PROJECT).hex: $(PROJECT).out
	$(AVRBINDIR)$(OBJCOPY) -j .text -j .data -O ihex $(PROJECT).out $(PROJECT).hex

$(PROJECT).bin: $(PROJECT).out
	$(AVRBINDIR)$(OBJCOPY) -j .text -j .data -O binary $(PROJECT).out $(PROJECT).bin

$(PROJECT).out: $(SOURCES) $(DEPS)
	$(AVRBINDIR)$(CC) $(CFLAGS) -I. -o $(PROJECT).out $(SOURCES)
	$(AVRBINDIR)avr-size $(PROJECT).out

asm: $(SOURCES) $(DEPS)
	$(AVRBINDIR)$(CC) $(CFLAGS) -S  -I. -o $(PROJECT).s $(SOURCES)


program: $(PROJECT).hex
	$(AVRBINDIR)$(AVRDUDECMD) -U flash:w:$(PROJECT).hex


clean:
	rm -f $(PROJECT).bin
	rm -f $(PROJECT).out
	rm -f $(PROJECT).hex
	rm -f $(PROJECT).s
	rm -f *.o


objdump: $(PROJECT).out
	$(AVRBINDIR)avr-objdump -xdC $(PROJECT).out | less

# Compatibility with serprog-duino / User Friendlyness helpers
u2:
	$(MAKE) clean
	DFLAGS= FRBAUD=115200 $(MAKE) all

flash-u2:
	BLBAUD=115200 SERIAL_DEV=/dev/ttyACM0 $(MAKE) program
	
flash-ch341:
	BLBAUD=115200 SERIAL_DEV=/dev/ttyUSB0 $(MAKE) program

ftdi:
	$(MAKE) clean
	DFLAGS=-DFTDI FRBAUD=2000000 $(MAKE) all

flash-ftdi:
	BLBAUD=57600 SERIAL_DEV=/dev/ttyUSB0 $(MAKE) program

#A few Arduino Mega (old 1280 FTDI based or 2560 U2 based for now) helpers
mega1280:
	$(MAKE) clean
	DFLAGS=-DFTDI MMCU=atmega1280 $(MAKE) all

flash-mega1280:
	BLBAUD=57600 SERIAL_DEV=/dev/ttyUSB0 AVRDUDE_MCU=m1280 $(MAKE) program

mega2560:
	$(MAKE) clean
	DFLAGS= FRBAUD=115200 MMCU=atmega2560 $(MAKE) all

flash-mega2560:
	BLBAUD=115200 AVRDUDE_PROGRAMMER="wiring" SERIAL_DEV=/dev/ttyACM0 AVRDUDE_MCU=m2560 $(MAKE) program

