#include "main.h"
#include "uart.h"
#include "console.h"
#include "lib.h"
#include "appdb.h"
#include "flash.h"
#include "ciface.h"

static void sendcrlf(void) {
	sendstr_P(PSTR("\r\n"));
}

void echo_cmd(void) {
	unsigned char i;
	for (i=1;i<token_count;i++) {
		sendstr(tokenptrs[i]);
		SEND(' ');
	}
}

unsigned long int calc_opdo(unsigned long int val1, unsigned long int val2, unsigned char *op) {
	switch (*op) {
		case '+':
			val1 += val2;
			break;
		case '-':
			val1 -= val2;
			break;
		case '*':
			val1 *= val2;
			break;
		case '/':
			val1 /= val2;
			break;
		case '%':
			val1 %= val2;
			break;
		case '&':
			val1 &= val2;
			break;
		case '|':
			val1 |= val2;
			break;
	}
	return val1;
}

void luint2outdual(unsigned long int val) {
	unsigned char buf[11];
	luint2str(buf,val);
	sendstr(buf);
	sendstr_P(PSTR(" ("));
	luint2xstr(buf,val);
	sendstr(buf);
	sendstr_P(PSTR("h) "));
}

unsigned long int closureparser(unsigned char firsttok, unsigned char*ptr) {
	unsigned char *op=NULL;
	unsigned char i,n;
	unsigned long int val1, val2;
	if (token_count <= firsttok) return 0;
	val1 = astr2luint(tokenptrs[firsttok]);
	sendstr_P(PSTR("{ "));
	luint2outdual(val1);
	n=0;
	for(i=firsttok+1;i<token_count;i++) {
		if (n&1) {
			sendstr(op);
			SEND(' ');
			if (*(tokenptrs[i]) == '(') {
				val2 = closureparser((i+1),&i);
			} else {
				val2 = astr2luint(tokenptrs[i]);
				luint2outdual(val2);
			}
			val1 = calc_opdo(val1,val2,op);
		} else {
			if (*(tokenptrs[i]) == ')') {
				sendstr_P(PSTR("} "));
				*ptr = i+1;
				return val1;
			}
			op = tokenptrs[i];
		}
		n++;
	}
	return val1;
}

void calc_cmd(void) {
	unsigned char *op=NULL;
	unsigned char i,n;
	unsigned long int val1;
	unsigned long int val2;
	if (token_count < 2) return;

	if (*(tokenptrs[1]) == '(') {
		val1 = closureparser(2,&i);
	} else {
		val1 = astr2luint(tokenptrs[1]);
		luint2outdual(val1);
		i=2;
	}
	n=0;
	for (;i<token_count;i++) {
		if (n&1) {
			sendstr(op);
			SEND(' ');
			if (*(tokenptrs[i]) == '(') {
				val2 = closureparser((i+1),&i);
			} else {
				val2 = astr2luint(tokenptrs[i]);
				luint2outdual(val2);
			}
			val1 = calc_opdo(val1,val2,op);
		} else {
			op = tokenptrs[i];
		}
		n++;
	}
	sendstr_P(PSTR("= "));
	luint2outdual(val1);
}

void help_cmd(void) {
	unsigned char i;
	const struct command_t * ctptr;
	PGM_P name;
	for(i=0;;i++) {
		ctptr = &(appdb[i]);
		name = (PGM_P)pgm_read_word(&(ctptr->name));
		if (!name) break;
		sendstr_P(name);
		SEND(' ');
	}
}


// Returns Vendor (LOW) and Device (High) ID
unsigned int identify_flash(void) {
	unsigned int rv;
	unsigned char device;
	unsigned char vendor;
	
	flash_write(0x5555,0xAA);
	_delay_us(10);
	flash_write(0x2AAA,0x55);
	_delay_us(10);
	flash_write(0x5555,0x90);
	_delay_ms(10);
	device = flash_read(1);
	vendor = flash_read(0);
	rv = ((device<<8)|(vendor));
	flash_write(0x5555,0xAA);
	_delay_us(10);
	flash_write(0x2AAA,0x55);
	_delay_us(10);
	flash_write(0x5555,0xF0);
	_delay_ms(10);
	return rv;
}


void flash_readsect_cmd(void) {
	unsigned char i,d,z;
	unsigned char buf[3];
	unsigned char dbuf[16];
	unsigned long int addr;
	if (strlen((char*)tokenptrs[1]) != 4) return;
	addr = (((unsigned long int)xstr2uchar(tokenptrs[1]))<<16);
	addr |= (((unsigned long int)xstr2uchar((tokenptrs[1]+2)))<<8);
	i=0;
	while(1) {
		d = flash_read(addr|i);
		uchar2xstr(buf,d);
		sendstr(buf);
		SEND(' ');
		dbuf[i&0x0F] = d;
		if ((i & 0x0F) == 0x0F) {
			for(z=0;z<16;z++) {
				if (((dbuf[z]) < 32) || (dbuf[z] == 127) || (dbuf[z] > 127))
					SEND('.');
				else SEND(dbuf[z]);
			}
			sendcrlf();
		}
		i++;
		if (i == 0) break;
	}
	return;
}


void flash_proto_cmd(void) {
	uint8_t proto = flash_get_proto();
	sendstr_P(PSTR("PROTO: "));
	switch (proto) {
		case 0:
		sendstr_P(PSTR("NONE"));
		break;

		case CHIP_BUSTYPE_PARALLEL:
		sendstr_P(PSTR("PARALLEL"));
		break;

		case CHIP_BUSTYPE_LPC:
		sendstr_P(PSTR("LPC"));
		break;

		case CHIP_BUSTYPE_FWH:
		sendstr_P(PSTR("FWH"));
		break;

		case CHIP_BUSTYPE_SPI:
		sendstr_P(PSTR("SPI"));
		break;
	}
}

void flash_idchip_cmd(void) {
	unsigned char buf[5];
	unsigned int chipid;
	chipid = identify_flash();
	uint2xstr(buf,chipid);
	sendstr(buf);
	return;
}

void bljump_cmd(void) {
	void (*btloader)(void)	= (void*)(BTLOADERADDR>>1); // Make PM
	_delay_ms(100);
	btloader();
}
