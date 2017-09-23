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
MMCU=atmega1280
AVRDUDE_MCU ?= m1280
AVRDUDE_PROGRAMMER ?= wiring

# These defaults are hopefully OK for Arduino Mega (1280, old, with FTDI)
# feel free to change (and please make an issue if the defaults are blergh).

#Device
SERIAL_DEV ?= /dev/ttyUSB0
# Bootloader
BLBAUD ?= 57600
# Flashrom serial (=serprog)
FRBAUD ?= 115200
#Additional defines (used my make ftdi)
DFLAGS ?= -DFTDI

AVRDUDECMD=avrdude -c $(AVRDUDE_PROGRAMMER) -p $(AVRDUDE_MCU) -P $(SERIAL_DEV) -b $(BLBAUD)

#AVRBINDIR=/usr/avr/bin/

CFLAGS=-mmcu=$(MMCU) -DBAUD=$(FRBAUD) -Os -Wl,--relax -fno-inline-small-functions -fno-tree-switch-conversion -frename-registers -g -Wall -W -pipe -flto -fwhole-program -std=gnu99 $(DFLAGS)

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
usb:
	DFLAGS= FRBAUD=115200 $(MAKE) clean all

flash-usb:
	BLBAUD=115200 SERIAL_DEV=/dev/ttyACM0 $(MAKE) program

ftdi:
	DFLAGS=-DFTDI FRBAUD=2000000 $(MAKE) clean all

flash-ftdi:
	BLBAUD=57600 SERIAL_DEV=/dev/ttyUSB0 $(MAKE) program
