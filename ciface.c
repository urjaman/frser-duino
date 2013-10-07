#include "main.h"
#include "uart.h"
#include "console.h"
#include "appdb.h"
#include "lib.h"
#include "ciface.h"

#define RECVBUFLEN 64

const unsigned char prompt[] PROGMEM = "\x0D\x0A >";
unsigned char recvbuf[RECVBUFLEN];
unsigned char token_count;
unsigned char* tokenptrs[MAXTOKENS] __attribute__((aligned(16)));


void ciface_main(void) {
	void(*func)(void);
	for (;;) {
		sendstr_P((PGM_P)prompt);
		if (getline(recvbuf,RECVBUFLEN)) return;
		tokenize(recvbuf,tokenptrs, &token_count);
		if (token_count) {
			func = find_appdb(tokenptrs[0]);
			func();
		}
	}
}
