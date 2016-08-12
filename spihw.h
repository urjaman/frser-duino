/* Define what our hardware is. */
#include "spihw_avrspi.h"

#if (defined __AVR_ATmega328P__)||(defined __AVR_ATmega328__)||(defined __AVR_ATmega168P__)||(defined __AVR_ATmega168__)||(__AVR_ATmega88P__)||(__AVR_ATmega88__)||(defined __AVR_ATmega48P__)||(defined __AVR_ATmega48__)
/* The usual duinos ... */
#define SPI_PORT	PORTB
#define SCK		PORTB5		/* port 13 */
#define MISO		PORTB4		/* port 12 */
#define MOSI 		PORTB3		/* port 11 */
#define SS		PORTB2		/* port 10 */
#define DDR_SPI		DDRB

/* Change to 1 for testing frser-duino on the LPC+SPI shield. */
#if 0
#define spi_select() do { DDRB |=_BV(0); } while(0)
#define spi_deselect() do { DDRB &= ~_BV(0); _delay_us(1); } while(0);
#else
#define spi_select() do { SPI_PORT &= ~(1<<SS); } while(0)
#define spi_deselect() do { SPI_PORT |= (1<<SS); } while(0)
#endif

#elif (defined __AVR_ATmega1280__)||(defined __AVR_ATmega1281__)||(defined __AVR_ATmega2560__)||(defined __AVR_ATmega2561__)
/* The Megas ... */

#define SPI_PORT	PORTB
#define DDR_SPI		DDRB
#define SCK		PORTB1
#define MISO		PORTB3
#define MOSI 		PORTB2
#define SS		PORTB0
#define spi_select() do { SPI_PORT &= ~(1<<SS); } while(0)
#define spi_deselect() do { SPI_PORT |= (1<<SS); } while(0)

#else
#error define your SPI hardware pins
#endif


