/* Define what our hardware is. */
#include "spihw_avrspi.h"


#define SPI_PORT	PORTB
#define SCK		PORTB1		/* port 13 */
#define MISO		PORTB3		/* port 12 */
#define MOSI 		PORTB2		/* port 11 */
#define SS		PORTB0		/* port 10 */
#define DDR_SPI		DDRB

/* Change to 1 for testing frser-duino on the LPC+SPI shield. */
#if 0
#define spi_select() do { DDRB |=_BV(0); } while(0)
#define spi_deselect() do { DDRB &= ~_BV(0); _delay_us(1); } while(0);
#else
#define spi_select() do { SPI_PORT &= ~(1<<SS); } while(0)
#define spi_deselect() do { SPI_PORT |= (1<<SS); } while(0)
#endif


