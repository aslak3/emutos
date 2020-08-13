#include <stdint.h>

void maxi000_init(void);

/* vectors */

#define VRESET (*(uint32_t volatile *)(0))
#define VUSER128 (*(uint32_t volatile *)(512))
#define VUSER129 (*(uint32_t volatile *)(516))
#define VUSER130 (*(uint32_t volatile *)(520))

/* alpha */

#define ALPHABASE 0x100000
#define SYSCONF (*(uint16_t volatile *)(ALPHABASE+0))
#define LED  (*(uint16_t volatile *)(ALPHABASE+2))
#define BUZZER (*(uint16_t volatile *)(ALPHABASE+4))
#define DMASRC (*(uint32_t volatile *)(ALPHABASE+0x6))
#define DMASRCHI (*(uint16_t volatile *)(ALPHABASE+0x6))
#define DMASRCLO (*(uint16_t volatile *)(ALPHABASE+0x8))
#define DMADSTHI (*(uint16_t volatile *)(ALPHABASE+0xa))
#define DMADSTLO (*(uint16_t volatile *)(ALPHABASE+0xc))
#define DMALEN (*(uint16_t volatile *)(ALPHABASE+0xe))
#define DMAFLAGS (*(uint16_t volatile *)(ALPHABASE+0x10))

#define BASE16C654 0x102000

/* beta */

#define BETABASE 0x101000
#define VGARWADDR (*(uint32_t volatile *)(BETABASE+0))
#define VGARWADDRHI (*(uint16_t volatile *)(BETABASE+0))
#define VGARWADDRLO (*(uint16_t volatile *)(BETABASE+2))
#define VGADATA (*(uint16_t volatile *)(BETABASE+4))
#define VGAMODEDEFATTR (*(uint16_t volatile *)(BETABASE+6))
#define VGAOFFSETADDR (*(uint16_t volatile *)(BETABASE+8))
#define PS2ASTATUS  (*(uint8_t volatile *)(BETABASE+0xa))
#define PS2ASCANCODE  (*(uint8_t volatile *)(BETABASE+0xb))
#define SPISELECTS (*(uint8_t volatile *)(BETABASE+0xe))
#define SPIDATA (*(uint8_t volatile *)(BETABASE+0x10))
#define TIMERCOUNT  (*(uint16_t volatile *)(BETABASE+0x12))

/* SC16C654 UART */

/* regsiters within one port */

#define RHR16C654 0
#define THR16C654 0
#define IER16C654 2
#define ISR16C654 4
#define FCR16C654 4
#define LCR16C654 6
#define MCR16C654 8
#define LSR16C654 10
#define MSR16C654 12
#define SPR16C654 14
#define DLL16C654 0
#define DLM16C654 2
#define EFR16C654 4
#define XON116C654 8
#define XON216C654 10
#define XOFF116C654 12
#define XOFF216C654 14

/* the base address of each port */

#define BASEPA16C654 (*(uint16_t volatile *)(BASE16C654+0))
#define BASEPA BASEPA16C654
#define BASEPB16C654 (*(uint16_t volatile *)(BASE16C654+16))
#define BASEPB BASEPB16C654
#define BASEPC16C654 (*(uint16_t volatile *)(BASE16C654+32))
#define BASEPC BASEPC16C654
#define BASEPD16C654 (*(uint16_t volatile *)(BASE16C654+48))
#define BASEPD BASEPD16C654
