#include <avr/io.h>

//#define DELAY7 asm volatile("rjmp .+0" "\n\t"  "rjmp .+0" "\n\t" "rjmp .+0" "\n\t" "nop" "\n" ::);

#define RD_PORT PORTF
#define RD_PIN  7
#define WR_PORT PORTF
#define WR_PIN  6
#define CD_PORT PORTF
#define CD_PIN  5
#define CS_PORT PORTF
#define CS_PIN  4
#define RESET_PORT PORTF
#define RESET_PIN  1

#define write_8(d) \
{  \
PORTE = (PORTE & B10111111) | (((d) & B10000000)>>1); \
PORTD = (PORTD & B01101111) | (((d) & B01000000)<<1) | ((d) & B00010000);  \
PORTC = (PORTC & B01111111) | (((d) & B00100000)<<2); \
PORTB = (PORTB & B00001111) | (((d) & B00001111)<<4); \
}

#define read_8() \
( \
((PINE & B01000000) << 1) | \
((PIND & B10000000) >> 1) | \
((PINC & B10000000) >> 2) | \
((PINB & B11110000) >> 4) | \
 (PIND & B00010000) \
)

#define setWriteDir() { DDRE |=  B01000000; DDRD |=  B10010000; DDRC |=  B10000000; DDRB |=  B11110000; }
#define setReadDir()  { DDRE &= ~B01000000; DDRD &= ~B10010000; DDRC &= ~B10000000; DDRB &= ~B11110000; }

#define write8(x)     { write_8(x); WR_STROBE; }
#define write16(x)    { uint8_t h = (x)>>8, l = x; write8(h); write8(l); }
#define READ_8(dst)   { RD_STROBE; dst = read_8(); RD_IDLE; }
#define READ_16(dst)  { uint8_t hi; READ_8(hi); READ_8(dst); dst |= (hi << 8); }
#define PIN_LOW(p, b)        (p) &= ~(1<<(b))
#define PIN_HIGH(p, b)       (p) |= (1<<(b))
#define PIN_OUTPUT(p, b)     *(&p-1) |= (1<<(b))

#define RD_ACTIVE     PIN_LOW(RD_PORT, RD_PIN)
#define RD_IDLE       PIN_HIGH(RD_PORT, RD_PIN)
#define RD_OUTPUT     PIN_OUTPUT(RD_PORT, RD_PIN)

#define WR_ACTIVE     PIN_LOW(WR_PORT, WR_PIN)
#define WR_IDLE	      PIN_HIGH(WR_PORT, WR_PIN)
#define WR_OUTPUT	  PIN_OUTPUT(WR_PORT, WR_PIN)

#define CD_COMMAND	  PIN_LOW(CD_PORT, CD_PIN)
#define CD_DATA		  PIN_HIGH(CD_PORT, CD_PIN)
#define CD_OUTPUT	  PIN_OUTPUT(CD_PORT, CD_PIN)

#define CS_ACTIVE	  PIN_LOW(CS_PORT, CS_PIN)
#define CS_IDLE		  PIN_HIGH(CS_PORT, CS_PIN)
#define CS_OUTPUT	  PIN_OUTPUT(CS_PORT, CS_PIN)

#define RESET_ACTIVE  PIN_LOW(RESET_PORT, RESET_PIN)
#define RESET_IDLE    PIN_HIGH(RESET_PORT, RESET_PIN)
#define RESET_OUTPUT  PIN_OUTPUT(RESET_PORT, RESET_PIN)

// General macros.   IOCLR registers are 1 cycle when optimised.
#define WR_STROBE { WR_ACTIVE; WR_IDLE; }       //PWLW=TWRL=50ns
#define RD_STROBE RD_IDLE, RD_ACTIVE, RD_ACTIVE, RD_ACTIVE      //PWLR=TRDL=150ns, tDDR=100ns
#define CTL_INIT()   { RD_OUTPUT; WR_OUTPUT; CD_OUTPUT; CS_OUTPUT; }
#define WriteCmd(x)  { CD_COMMAND; write16(x); }
#define WriteData(x) { CD_DATA; write16(x); }
