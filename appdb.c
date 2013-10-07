#include "main.h"
#include "ciface.h"
#include "console.h"
#include "appdb.h"

const unsigned char echostr[] PROGMEM = "ECHO";
const unsigned char readsectstr[] PROGMEM = "READSECT";
const unsigned char flashidstr[] PROGMEM = "IDCHIP";
const unsigned char calcstr[] PROGMEM = "CALC";
const unsigned char helpstr[] PROGMEM = "?";

const struct command_t appdb[] PROGMEM = {
	{(PGM_P)echostr, &(echo_cmd)},
	{(PGM_P)readsectstr, &(flash_readsect_cmd)},
	{(PGM_P)flashidstr, &(flash_idchip_cmd)},
	{(PGM_P)calcstr, &(calc_cmd)},
	{(PGM_P)helpstr, &(help_cmd)},
	{NULL,NULL}
};

void invalid_command(void) {
	sendstr(tokenptrs[0]);
	sendstr_P(PSTR(": not found"));
	}

void *find_appdb(unsigned char* cmd) {
	unsigned char i;
	const struct command_t * ctptr;
	PGM_P name;
	void* fp;
	for(i=0;;i++) {
		ctptr = &(appdb[i]);
		name = (PGM_P)pgm_read_word(&(ctptr->name));
		fp = (void*)pgm_read_word(&(ctptr->function));
		if (!name) break;
		if (strcmp_P((char*)cmd,name) == 0) {
			return fp;
			}
	}
	return &(invalid_command);
}


