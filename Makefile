##
## This file is part of the frser-atmega644 project.
##
## Copyright (C) 2010,2011 Urja Rannikko <urjaman@gmail.com>
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

PROJECT=frser-atmega644
DEPS=uart.h frser.h udelay.h main.h parallel.h lpc.h flash.h
CIFACE_SOURCES=ciface.c console.c lib.c appdb.c commands.c
SOURCES=main.c uart.c flash.c udelay.c frser.c parallel.c lpc.c spi.c $(CIFACE_SOURCES)
CC=avr-gcc
LD=avr-ld
OBJCOPY=avr-objcopy
MMCU=atmega644p
BTLOADERADDR=0xFC00
SERIAL_DEV=/dev/ttyUSB0
UISP=uisp_bbpg

AVRBINDIR=/usr/avr/bin/
#AVRDUDECMD=avrdude -p m644p -c dt006 -E noreset
# If using avr-gcc < 4.6.0, replace -flto with -combine
CFLAGS=-mmcu=$(MMCU) -Os -mcall-prologues -Wl,--relax -fno-inline-small-functions -fno-tree-scev-cprop -frename-registers -g -Wall -W -pipe -flto -fwhole-program


all: $(PROJECT).out

$(PROJECT).hex: $(PROJECT).out
	$(AVRBINDIR)$(OBJCOPY) -j .text -j .data -O ihex $(PROJECT).out $(PROJECT).hex

$(PROJECT).bin: $(PROJECT).out
	$(AVRBINDIR)$(OBJCOPY) -j .text -j .data -O binary $(PROJECT).out $(PROJECT).bin
 
$(PROJECT).out: $(SOURCES) $(DEPS)
	$(AVRBINDIR)$(CC) -DBTLOADERADDR=$(BTLOADERADDR) $(CFLAGS) -I./ -o $(PROJECT).out $(SOURCES)
	$(AVRBINDIR)avr-size $(PROJECT).out
	
asm: $(SOURCES) $(DEPS)
	$(AVRBINDIR)$(CC) $(CFLAGS) -S  -I./ -o $(PROJECT).s $(SOURCES)
	

#program: $(PROJECT).hex
#	$(AVRBINDIR)$(AVRDUDECMD) -U flash:w:$(PROJECT).hex

program: $(PROJECT).bin serialprogrammer
	sudo make sr-program

clean:
	rm -f $(PROJECT).bin
	rm -f $(PROJECT).out
	rm -f $(PROJECT).hex
	rm -f $(PROJECT).s
	rm -f *.o
	rm -f boot.out boot.hex
	rm -f serialprogrammer
	
backup:
	$(AVRBINDIR)$(AVRDUDECMD) -U flash:r:backup.bin:r

bootloader-pgm: boot.hex
	sudo make bootloader-pgm-r

bootloader-pgm-r: boot.hex
	$(UISP) -v=3 -dprog=bbpg -dt_sck=5 --wr_fuse_e=0xFD --wr_fuse_h=0xDE --wr_fuse_l=0xC0 if=boot.hex --upload --verify

bootloader: boot.hex

boot.hex: boot.out
	$(AVRBINDIR)$(OBJCOPY) -j .bootloader -O ihex boot.out boot.hex

boot.out:  boot.S
	$(AVRBINDIR)$(CC) -mmcu=$(MMCU) -c -o boot.o boot.S
	$(AVRBINDIR)$(LD) --section-start=.bootloader=$(BTLOADERADDR) -o boot.out boot.o

# bbpg usage here is to reset the AVR, so that you dont need to re-plug the USB cable all the time
sr-program: $(PROJECT).bin serialprogrammer
	$(UISP) -dprog=bbpg -dt_sck=5
	modprobe ftdi_sio
	sleep 1s
	./serialprogrammer $(PROJECT).bin $(SERIAL_DEV)

blj-program: $(PROJECT).bin serialprogrammer
	./serialprogrammer --bljump=1500000 $(PROJECT).bin $(SERIAL_DEV)

serialprogrammer: serialprogrammer.c
	gcc -W -Wall -Os -o serialprogrammer serialprogrammer.c
