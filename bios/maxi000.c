#include <stdint.h>

#include "emutos.h"
#include "ikbd.h"               /* for call_mousevec() */
#include "tosvars.h"

#include "maxi000.h"

#define STATE_IDLE 0
#define STATE_ENABLE_ACK 1
#define STATE_STREAMING 2

#define CMD_ENABLE 0xf4

#define SLICE_LEN 4800

extern void int_vbl(void);
extern void maxi000_int_timer(void);
extern void maxi000_update_display(void);

static void  __attribute__ ((interrupt)) maxi000_int_mouse(void);
static void  __attribute__ ((interrupt)) maxi000_int_vbl(void);

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
volatile int slice_count = 0;

volatile uint32_t this_slice_ad;
volatile uint32_t vram_pointer;

void maxi000_init(void)
{
	state = STATE_IDLE;

	VRESET = 0;
	VUSER129 = maxi000_int_mouse;
	VUSER130 = maxi000_int_vbl;
	VUSER128 = maxi000_int_timer;

	TIMERCOUNT = 5 * 8000U;

	PS2ASCANCODE = CMD_ENABLE;

	slice_count = 0;
}

static void  __attribute__ ((interrupt)) maxi000_int_mouse(void)
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

				packet_counter = 0;

				packet[0] = 0xf8;
				if (mp.mouse_state & 0x01) {
					packet[0] |= 0x02;
					LED = 1;
				}
				else {
					LED = 0;
				}
				if (mp.mouse_state & 0x02)
					packet[0] |= 0x01;

				int x = mp.x_delta;
				int y = mp.y_delta;
				if (x && mp.mouse_state & 0x10)
					x -= 0x100;
				if (y && mp.mouse_state & 0x20)
					y -= 0x100;

				packet[1] = x;
				packet[2] = -y;

				call_mousevec(packet);
			}
			break;

		default:
			break;
	}
}

static void  __attribute__ ((interrupt)) maxi000_int_vbl(void)
{
	TIMERCOUNT;

	int_vbl();

	if (slice_count == 0)
	{
		this_slice_ad = (uint32_t) v_bas_ad;
		VGARWADDR = 0;
	}

	DMASRC = this_slice_ad;
	DMALEN = SLICE_LEN;
	DMAFLAGS = 3;

	slice_count++;
	this_slice_ad += SLICE_LEN * 2;

	if (slice_count > 7) slice_count = 0;
}
