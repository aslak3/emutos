#include <stdint.h>

#include "emutos.h"
#include "ikbd.h"               /* for call_mousevec() */
#include "vectors.h"

#include "maxi030.h"

#define STATE_IDLE 0
#define STATE_ENABLE_ACK 1
#define STATE_STREAMING 2

#define CMD_ENABLE 0xf4

//extern void int_vbl(void);
// extern void maxi030_int_timer(void);

// static void  __attribute__ ((interrupt)) maxi030_int_mouse(void);
// static void  __attribute__ ((interrupt)) maxi030_int_vbl(void);
static void  __attribute__ ((interrupt)) maxi030_int_timer(void);

struct mouse_packet
{
    uint8_t mouse_state;
    SBYTE x_delta;
    SBYTE y_delta;
};

volatile uint16_t state = STATE_IDLE;
volatile uint16_t packet_counter = 0;
volatile struct mouse_packet mp;
volatile int vbl_count = 0;

void maxi030_init(void)
{
    state = STATE_IDLE;

    LED = 1;

    // VL6AUTOVECTOR = (uint32_t) maxi030_int_vbl;
    // VL5AUTOVECTOR = (uint32_t) maxi030_int_mouse;
    VL1AUTOVECTOR = (uint32_t) maxi030_int_timer;

	TIMERCOUNTU = 0x03;
    TIMERCOUNTM = 0x0d;
    TIMERCOUNTL = 0x40;
    TIMERCONTROL = 0x01;

    INTPASS = 0x20 | 0x10 | 0x01;
        
//   VCARDSTMODE = 0x0001;

    // Enable interrupt
    // PS2ASTATUS = 0x01;
    // PS2ASCANCODE = CMD_ENABLE;
}

#if 0
static void  __attribute__ ((interrupt)) maxi030_int_mouse(void)
{
    uint8_t data = PS2ASCANCODE;
    
    switch (state)
    {
        case STATE_IDLE:
            state = STATE_ENABLE_ACK;
            break;

        case STATE_ENABLE_ACK:
            state = STATE_STREAMING;
            packet_counter = 0;
            break;
            
        case STATE_STREAMING:
            if (packet_counter == 0)
                mp.mouse_state = data;
            else if (packet_counter == 1)
                mp.x_delta = data;
            else if (packet_counter == 2)
                mp.y_delta = data;
            packet_counter++;
            if (packet_counter >= 3)
            {
                SBYTE packet[3];
                packet[0] = 0xf8;
                if (mp.mouse_state & 0x01)
                    packet[0] |= 0x02;

                if (mp.mouse_state & 0x02)
                    packet[0] |= 0x01;

                int x = mp.x_delta;
                if (x > 100) x = 100;
                if (x < -100) x = -100;
                int y = mp.y_delta;
                if (y > 100) y = 100;
                if (y < -100) y = -100;
//				if (x && mp.mouse_state & 0x10)
//					x -= 0x100;
//				if (y && mp.mouse_state & 0x20)
//					y -= 0x100;

                packet[1] = x;
                packet[2] = -y;

                call_mousevec(packet);
                packet_counter = 0;
            }
            break;

        default:
            break;
    }
}
#endif

// static void  __attribute__ ((interrupt)) maxi030_int_vbl(void)
// {
//     VCARDVBLANKINTCLEAR = 0;
    
//     int_vbl();
// }

static void  __attribute__ ((interrupt)) maxi030_int_timer(void)
{
    TIMERCONTROL = 1;

    int_timerc();
}
