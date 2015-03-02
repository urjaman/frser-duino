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

void frser_main(void) __attribute__((noreturn));

/* Flashrom serial interface AVR implementation */
#define S_ACK 0x06
#define S_NAK 0x15

#define S_CMD_NOP		0x00            /* No operation                                 */
#define S_CMD_Q_IFACE           0x01            /* Query interface version                      */
#define S_CMD_Q_CMDMAP		0x02		/* Query supported commands bitmap		*/
#define S_CMD_Q_PGMNAME         0x03            /* Query programmer name                        */
#define S_CMD_Q_SERBUF          0x04            /* Query Serial Buffer Size                     */
#define S_CMD_Q_BUSTYPE         0x05            /* Query supported bustypes                     */
#define S_CMD_Q_CHIPSIZE        0x06            /* Query supported chipsize (2^n format)        */
#define S_CMD_Q_OPBUF           0x07            /* Query operation buffer size                  */
#define S_CMD_Q_WRNMAXLEN	0x08		/* Query Write to opbuf: Write-N maximum lenght */
#define S_CMD_R_BYTE            0x09            /* Read a single byte                           */
#define S_CMD_R_NBYTES          0x0A            /* Read n bytes                                 */
#define S_CMD_O_INIT            0x0B            /* Initialize operation buffer                  */
#define S_CMD_O_WRITEB          0x0C            /* Write opbuf: Write byte with address         */
#define S_CMD_O_WRITEN		0x0D		/* Write to opbuf: Write-N			*/
#define S_CMD_O_DELAY           0x0E            /* Write opbuf: udelay                          */
#define S_CMD_O_EXEC            0x0F            /* Execute operation buffer                     */
#define S_CMD_SYNCNOP		0x10		/* Special no-operation that returns NAK+ACK	*/
#define S_CMD_Q_RDNMAXLEN	0x11		/* Query read-n maximum length			*/
#define S_CMD_S_BUSTYPE		0x12		/* Set used bustype(s).				*/
#define S_CMD_O_SPIOP		0x13		/* Perform SPI operation.			*/
#define S_CMD_S_SPI_FREQ	0x14		/* Set SPI clock frequency			*/
#define S_CMD_S_PIN_STATE	0x15		/* Enable/disable output drivers		*/

/* The biggest valid command value */
#define S_MAXCMD 0x15
/* The maximum static length of parameters (poll_dly)) */
#define S_MAXLEN 0x08
