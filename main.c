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

#include "main.h"
#include "uart.h"
#include "frser.h"


int main(void) {
	cli();
	DDRA=0;
	DDRB=0;
	DDRC=0;
	DDRD=0;
	PORTA=0xFF;
	PORTB=0xFF;
	PORTC=0xFF;
	PORTD=0xFF;
	uart_init();
	power_adc_disable();
	power_timer0_disable();
	power_timer2_disable();
	power_twi_disable();
	frser_main();
}


