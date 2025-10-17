#include <stdint.h>

void maxi030_init(void);

/* vectors */

#define VL1AUTOVECTOR (*(uint32_t volatile *)(100))
#define VL2AUTOVECTOR (*(uint32_t volatile *)(104))
#define VL3AUTOVECTOR (*(uint32_t volatile *)(108))
#define VL4AUTOVECTOR (*(uint32_t volatile *)(112))
#define VL5AUTOVECTOR (*(uint32_t volatile *)(116))
#define VL6AUTOVECTOR (*(uint32_t volatile *)(120))
#define VL7AUTOVECTOR (*(uint32_t volatile *)(124))

#define VUSER128 (*(uint32_t volatile *)(512))
#define VUSER129 (*(uint32_t volatile *)(516))
#define VUSER130 (*(uint32_t volatile *)(520))

/* core */

#define COREBASE 0x44000000
#define LED (*(uint8_t volatile *)(COREBASE+0))
#define BUZZER (*(uint8_t volatile *)(COREBASE+1))
#define SYSCONF (*(uint16_t volatile *)(COREBASE+2))
#define PS2ASTATUS  (*(uint8_t volatile *)(COREBASE+3))
#define PS2ASCANCODE (*(uint8_t volatile *)(COREBASE+4))
#define TIMERCOUNTU (*(uint8_t volatile *)(COREBASE+0xd))
#define TIMERCOUNTM (*(uint8_t volatile *)(COREBASE+0xe))
#define TIMERCOUNTL (*(uint8_t volatile *)(COREBASE+0xf))
#define TIMERCONTROL (*(uint8_t volatile *)(COREBASE+0x10))
#define INTPASS (*(uint8_t volatile *)(COREBASE+0x11))

#define VCARDREGBASE 0x40200000
#define VCARDLED (*(uint16_t volatile *)(VCARDREGBASE+0xa))
#define VCARDSTMODE (*(uint16_t volatile *)(VCARDREGBASE+0xe))
#define VCARDVBLANKINTCLEAR (*(uint16_t volatile *)(VCARDREGBASE+0x10))

//#define BASE16C654 0x102000

/* beta */

//#define SPISELECTS (*(uint8_t volatile *)(BETABASE+0xe))
//#define SPIDATA (*(uint8_t volatile *)(BETABASE+0x10))
//#define TIMERCOUNT  (*(uint16_t volatile *)(BETABASE+0x12))
