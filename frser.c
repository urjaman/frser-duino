/*
 * This file is part of the frser-duino project.
 *
 * Copyright (C) 2010,2011,2013,2015 Urja Rannikko <urjaman@gmail.com>
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
#include "udelay.h"
#include "spi.h"
#include "frser.h"
#include "typeu.h"

/* The length of the operation buffer - fake to support delays */
#define S_OPBUFLEN 12

struct constanswer {
	uint8_t len;
	PGM_P data;
} __attribute__ ((__packed__));

/* Calculate a nice read-n max value so that it doesnt hurt performance, but
   doesnt allow the device to be accidentally left in an "infini-tx" mode.
   This is the amount of data it can send based on baud rate in 2 seconds rounded to kB. */

#define BYTERATE (BAUD/5)
#define KBPSEC ((BYTERATE+512)/1024)
#define RDNMAX (KBPSEC*1024)

/* Report -4 for safety (atleast -1 because our buffer cant be filled 100%) */
#define UART_RBUFLEN (UART_BUFLEN-4)

const char PROGMEM ca_nop[1] = { S_ACK };
const char PROGMEM ca_iface[3] = { S_ACK,0x01,0x00 };
const char PROGMEM ca_bitmap[33] = { S_ACK, 0xBF, 0xC9, 0x3F };
const char PROGMEM ca_pgmname[17] = { S_ACK, 'f','r','s','e','r','-','d','u','i','n','o' }; /* Eww */
const char PROGMEM ca_serbuf[3] = { S_ACK, (UART_RBUFLEN)&0xFF, (UART_RBUFLEN>>8)&0xFF };
const char PROGMEM ca_bustypes[2] = { S_ACK, 0x08 };
const char PROGMEM ca_opbufsz[3] = { S_ACK, S_OPBUFLEN&0xFF, (S_OPBUFLEN>>8)&0xFF };
const char PROGMEM ca_wrnlen[4] = { S_ACK, 0x00, 0x01, 0x00 };
const char PROGMEM ca_syncnop[2] = { S_NAK, S_ACK };
const char PROGMEM ca_rdnmaxlen[4] = { S_ACK, RDNMAX&0xFF, (RDNMAX>>8)&0xFF, (RDNMAX>>16)&0xFF };

/* Commands with a const answer cannot have parameters */
const struct constanswer PROGMEM const_table[S_MAXCMD+1] = {
	{ 1, ca_nop },		// NOP
	{ 3, ca_iface },	// IFACE V
	{ 33, ca_bitmap },	// op bitmap
	{ 17, ca_pgmname },	// programmer name
	{ 3, ca_serbuf },	// serial buffer size
	{ 2, ca_bustypes },	// bustypes
	{ 0, NULL },		// chip size
	{ 3, ca_opbufsz },	// operation buffer size

	{ 4, ca_wrnlen },	// write-n max len
	{ 0, NULL },		// read byte
	{ 0, NULL },		// read n
	{ 0, NULL },		// init opbuf
	{ 0, NULL },		// opbuf, write-1
	{ 0, NULL },		// opbuf, write-n
	{ 0, NULL },		// opbuf, delay
	{ 0, NULL },		// exec opbuf

	{ 2, ca_syncnop },	// sync nop
	{ 4, ca_rdnmaxlen },	// Read-n maximum len
	{ 0, NULL },		// Set bustype
	{ 0, NULL },		// SPI operation
	{ 0, NULL },		// SPI speed
	{ 0, NULL },		// set output drivers
};

const uint8_t PROGMEM op2len[S_MAXCMD+1] = { /* A table to get  parameter length from opcode if possible (if not 0) */
		0x00, 0x00, 0x00,	/* NOP, iface, bitmap */
		0x00, 0x00, 0x00,	/* progname, serbufsize, bustypes */
		0x00, 0x00, 0x00,	/* chipsize, opbufsz, query-n maxlen */
		0x03, 0x06, 0x00,	/* read byte, read n, init opbuf */
		0x04, 0x00, 0x04,	/* write byte, write n, write delay */
		0x00, 0x00, 0x00,	/* Exec opbuf, syncnop, max read-n */
		0x01, 0x06, 0x04,	/* Set used bustype, SPI op, spi-speed */
		0x01 			/* output drivers */
	};


/* We only support delays in "opbuf", thus accumulate them here before EXEC. */
static uint32_t opbuf_delay_acc = 0;

static uint32_t buf2u24(uint8_t *buf) {
	u32_u u24;
	u24.b[0] = buf[0];
	u24.b[1] = buf[1];
	u24.b[2] = buf[2];
	u24.b[3] = 0;
	return u24.l;
}

static void do_cmd_spiop(uint8_t *parbuf) {
	uint32_t sbytes;
	uint32_t rbytes;
	sbytes = buf2u24(parbuf);
	rbytes = buf2u24(parbuf+3);
	spi_spiop(sbytes,rbytes);
}

static void do_cmd_spispeed(uint8_t* parbuf) {
	u32_u hz;
	hz.b[0] = parbuf[0];
	hz.b[1] = parbuf[1];
	hz.b[2] = parbuf[2];
	hz.b[3] = parbuf[3];
	if (hz.l==0) { /* I think this spec is stupid. /UR */
		SEND(S_NAK);
		return;
	}
	u32_u new_hz;
	new_hz.l = spi_set_speed(hz.l);
	SEND(S_ACK);
	SEND(new_hz.b[0]);
	SEND(new_hz.b[1]);
	SEND(new_hz.b[2]);
	SEND(new_hz.b[3]);
}

static void do_cmd_opbuf_delay(uint8_t *parbuf) {
	u32_u usecs;
	usecs.b[0] = parbuf[0];
	usecs.b[1] = parbuf[1];
	usecs.b[2] = parbuf[2];
	usecs.b[3] = parbuf[3];
	opbuf_delay_acc += usecs.l;
	SEND(S_ACK);
	return;
}

void frser_main(void) {
	for(;;) {
		uint8_t parbuf[S_MAXLEN]; /* Parameter buffer */
		uint8_t a_len,p_len;
		uint8_t op;
		uint8_t i;
		op = RECEIVE();
		if (op > S_MAXCMD) {
			/* This is a pretty futile case as in that we shouldnt get
			these commands at all with the supported cmd bitmap system */
			/* Still better to say something vs. nothing.		   */
			SEND(S_NAK);
			continue;
		}
		a_len = pgm_read_byte(&(const_table[op].len));
		/* These are the simple query-like operations, we just reply from ProgMem: */
		/* NOTE: Operations that have a const answer cannot have parameters !!!    */
		if (a_len) {
			PGM_P data = (PGM_P)pgm_read_word(&(const_table[op].data));
			for(i=0;i<a_len;i++) {
				uint8_t c = pgm_read_byte(&(data[i]));
				SEND(c);
			}
			continue;
		}

		p_len = pgm_read_byte(&(op2len[op]));
		for (i=0;i<p_len;i++) parbuf[i] = RECEIVE();

		/* These are the operations that need real acting upon: */
		switch (op) {
			case S_CMD_O_DELAY:
				do_cmd_opbuf_delay(parbuf);
				break;

			case S_CMD_O_EXEC:
				if (opbuf_delay_acc) udelay(opbuf_delay_acc);
				/* FALL-THROUGH */
			case S_CMD_O_INIT:
				SEND(S_ACK);
				opbuf_delay_acc = 0;
				break;

			case S_CMD_S_BUSTYPE:
				SEND(S_ACK);
				/* Effectively NOP for us since only SPI. */
				break;

			case S_CMD_O_SPIOP:
				do_cmd_spiop(parbuf);
				break;

			case S_CMD_S_SPI_FREQ:
				do_cmd_spispeed(parbuf);
				break;

			case S_CMD_S_PIN_STATE:
				SEND(S_ACK);
				/* SPI-op auto-initializes, thus only uninit here. */
				if (!parbuf[0]) spi_uninit();
				break;

			default:
				SEND(S_NAK);
				break;
		}
	}
}
