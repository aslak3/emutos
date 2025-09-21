#include "config.h"
#include <stdint.h>

/* #define ENABLE_KDEBUG  */

#ifdef CONF_WITH_VT82C42

#include "emutos.h"
#include "ikbd.h"
#include "vectors.h"

#include <stdio.h>
#include <ctype.h>

#include "vt82c42.h"

#define PS2_BASE                0x00F7F200
#define PS2_REG(x)      (*((volatile char *) PS2_BASE + x))

// Register Offsets
#define PS2_DATA		  		0x00
#define PS2_CMD 				0x02
#define PS2_STAT				0x02

// For mouse
#define MOUSE_REL_POS_REPORT    0xf8    /* values for mouse_packet[0] */
#define RIGHT_BUTTON_DOWN       0x01    /* these values are OR'ed in */
#define LEFT_BUTTON_DOWN        0x02

// Function prototypes
static void vt_wait_read(void);
static void vt_wait_write(void);
static void vt_send_command(uint8_t cmd);
static void vt_send_data(uint8_t data);
static uint8_t vt_read_data(void);
static uint8_t vt_device_send_command(uint8_t port, uint8_t cmd);
static uint8_t vt_get_config_byte(void);
static void vt_set_config_byte(uint8_t cfg_byte);
static void vt_disable_for_init(void);
static void vt_set_leds(uint8_t leds);
static uint8_t vt_flush(void);

void __attribute__((interrupt)) vt_interrupt_handler(void);
void vt_process_scancode(uint8_t sc);
void vt_process_mouse(uint8_t *packet);
void vt_handle_mouse(uint8_t data);

static uint8_t g_key_mode = 0;

static const uint8_t st_make_code_map[] = {
    0 , 67 /*F9*/, 0 , 63 /*F5*/, 61 /*F3*/, 59 /*F1*/, 60 /*F2*/, 97 /*F12*/,
	0 , 68 /*F10*/, 66 /*F8*/, 64 /*F6*/, 62 /*F4*/, 15 /*Tab*/, 41 /*Backtick/Tilde (`~)*/, 0 , 
    0 , 56 /*Left Alt*/, 42 /*Left Shift*/, 0 , 29 /*Left Ctrl*/, 16 /*Q*/, 2 /*1*/, 0 ,
    0 , 0 , 44 /*Z*/, 31 /*S*/, 30 /*A*/, 17 /*W*/, 3 /*2*/, 0 ,
	0 , 46 /*C*/, 45 /*X*/, 32 /*D*/, 18 /*E*/, 5 /*4*/, 4 /*3*/, 0 ,
    0 , 57 /*Space*/, 47 /*V*/, 33 /*F*/, 20 /*T*/, 19 /*R*/, 6 /*5*/, 0 ,
    0 , 49 /*N*/, 48 /*B*/, 35 /*H*/, 34 /*G*/, 21 /*Y*/, 7 /*6*/, 0 ,
    0 , 0 , 50 /*M*/, 36 /*J*/, 22 /*U*/, 8 /*7*/, 9 /*8*/, 0 ,
    0 , 51 /*Comma (,<)*/, 37 /*K*/, 23 /*I*/, 24 /*O*/, 11 /*0*/, 10 /*9*/, 0 ,
    0 , 52 /*Period (.>)*/, 53 /*Slash (/?)*/, 38 /*L*/, 39 /*Semicolon (;:)*/, 25 /*P*/, 12 /*Minus (-_)*/, 0 ,
    0 , 0 , 40 /*Apostrophe ('")*/, 0 , 26 /*Left Bracket ([{)*/, 13 /*Equals (=+)*/, 0 , 0 , 58 /*CapsLock*/,
    54 /*Right Shift*/, 28 /*Enter*/, 27 /*Right Bracket (]})*/, 0 , 43 /*Backslash (\|)*/, 0 , 
    0 , 0 , 96 /*UK \| between left shift and Z*/, 0 , 0 , 0 , 0 , 14 /*Backspace*/, 0 , 0 , 109 /*Keypad 1/End*/,
    0 , 106 /*Keypad 4/Left*/, 103 /*Keypad 7/Home*/, 0 , 0 , 0 , 112 /*Keypad 0/Ins*/, 113 /*Keypad ./Del*/,
    110 /*Keypad 2/Down*/, 107 /*Keypad 5*/, 108 /*Keypad 6/Right*/, 104 /*Keypad 8/Up*/, 1 /*Escape*/,
    -1 /*NumLock*/, 98 /*F11*/, 78 /*Keypad +*/, 111 /*Keypad 3/PgDn*/, 74 /*Keypad -*/, 102 /*Keypad **/,
    105 /*Keypad 9/PgUp*/, -1 /*ScrollLock*/, 0 , 0 , 0 , 0 , 65 /*F7*/
};

static const uint8_t st_extended_make_code_map[] = {
    0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 
    56 /*Right Alt*/, 0 , 0 , 29 /*Right Ctrl*/, 0 , 0 , 0 , 0 , 0 , 0 ,
    0 , 0 , 0 , 0 , -1 /*Left GUI (Windows)*/, 0 , 0 , 0 , 0 , 0 , 0 , 0 ,
    -1 /*Right GUI (Windows)*/, 0 , 0 , 0 , 0 , 0 , 0 , 0 , -1 /*Menu*/,
    0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 ,
    0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 101 /*Keypad /*/, 0 , 0 , 0 ,
    0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 114 /*Keypad Enter*/,
    0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 79 /*End*/,
    0 , 75 /*Left Arrow*/, 71 /*Home*/, 0 , 0 , 0 , 82 /*Insert*/, 83 /*Delete*/,
    80 /*Down Arrow*/, 0 , 77 /*Right Arrow*/, 72 /*Up Arrow*/, 0 , 0 , 0 , 0 ,
    81 /*Page Down*/, 0 , 0 , 73 /*Page Up*/, 0 , 0
};

#define WAIT_TIMEOUT 10000

// Wait until the output buffer is full (data available to read)
static void vt_wait_read(void)
{
    volatile uint32_t timeout = WAIT_TIMEOUT;
    while ((PS2_REG(PS2_STAT) & STATUS_OBF) == 0)
    {
        timeout--;
        if (timeout == 0)
            break;
    }
}

// Wait until the input buffer is empty (ready to write)
static void vt_wait_write(void)
{
    volatile uint32_t timeout = WAIT_TIMEOUT;
    while (PS2_REG(PS2_STAT) & STATUS_IBF)
    {
        timeout--;
        if (timeout == 0)
            break;
    }
}

// Send a command to the PS/2 controller
static void vt_send_command(uint8_t cmd)
{
    vt_wait_write();
    PS2_REG(PS2_CMD) = cmd;
}

// Send data to the PS/2 controller
static void vt_send_data(uint8_t data)
{
    vt_wait_write();
    PS2_REG(PS2_DATA) = data;
}

// Read data from the PS/2 controller
static uint8_t vt_read_data(void)
{
    vt_wait_read();
    return PS2_REG(PS2_DATA);
}

// Send a command directly to a PS/2 device (keyboard or mouse)
static uint8_t vt_device_send_command(uint8_t port, uint8_t cmd)
{
    uint8_t retries = 10;
    uint8_t res;

    while (retries--)
    {
        if (port == 2) {
            vt_send_command(CMD_AUX_WRITE);  // Mouse
        }
        vt_send_data(cmd);
        res = vt_read_data();
        if (res != KBD_STATUS_RESEND)
            return res;
    }

    return res; // last response, even if error
}

static uint8_t vt_get_config_byte(void)
{
	vt_send_command(0x20);
    return vt_read_data();
}

static void vt_set_config_byte(uint8_t cfg_byte)
{
	vt_send_command(0x60);
	vt_send_data(cfg_byte);
}

static void vt_disable_for_init(void)
{
	uint8_t cfg = vt_get_config_byte();
	vt_set_config_byte(cfg & ~(CMD_BYTE_KBD_INT | CMD_BYTE_AUX_INT | CMD_BYTE_TRANS));
}

static void vt_set_leds(uint8_t leds)
{
    vt_send_data(KBD_CMD_LED);
	vt_send_data(leds);
}

static uint8_t vt_flush(void)
{
    int timeout = WAIT_TIMEOUT;
    // Clear the Output Buffer
    while (timeout)
	{
        if ((PS2_REG(PS2_STAT) & STATUS_OBF))
            PS2_REG(PS2_DATA);
        timeout--;
    }
	return 0;
}


//	keyboard interrupt handler
void __attribute__((interrupt)) vt_interrupt_handler(void)
{
    uint8_t status = PS2_REG(PS2_STAT);
    uint8_t data = PS2_REG(PS2_DATA);

    // Bit 5 set, mouse data
    if (status & 0x20)
        vt_handle_mouse(data);
    else if (status & 0x01)
        vt_process_scancode(data);
}

uint8_t vt8242_init(void)
{
    volatile PFVOID *vector_addr;

    KDEBUG(("vt8242_init()\n"));

    vt_flush();			 // flush buffer

    vt_send_command(CMD_KBD_OFF); // disable first port
	vt_send_command(CMD_AUX_OFF); // disable 2nd port

    vt_disable_for_init();

    KDEBUG(("vt8242: install keyboard interrupt handler\n"));
    vector_addr = &VEC_LEVEL1 + (CONF_VT82C42_AUTOVECTOR - 1);
    *vector_addr = (PFVOID)vt_interrupt_handler;


    KDEBUG(("vt8242: controller self test\n"));
    vt_send_command(CMD_DIAG);
	if (vt_read_data() != KBD_STATUS_DIAG_OK)
    {
		KDEBUG(("ERROR: PS/2 keyboard controller failed.\n"));
        return 0;
	}

	vt_send_command(CMD_AUX_ON); // enable 2nd port
	if (!(vt_get_config_byte() & CMD_BYTE_AUX_OFF))
    {
		KDEBUG(("PS/2 controller has 2 channels.\n"));
	}

    vt_send_command(CMD_KBD_TEST);
	if (vt_read_data() != 0x00)
    {
		KDEBUG(("ERROR: PS/2 keyboard test failed.\n"));
	}

    // enable first PS/2 port
	vt_send_command(CMD_KBD_ON);
    vt_flush();


    uint8_t response;

    response = vt_device_send_command(1, KBD_CMD_RESET);
    if ((response != KBD_STATUS_ACK))
    {
        KDEBUG(("ERROR: Keyboard reset error, resp = %02x\n", response));
    }

	response = vt_read_data();
    if (response != KBD_STATUS_RST_OK)
    {
		KDEBUG(("ERROR: Keyboard self test failed, resp = %02X\n", response));
		if (response == 0xFC)
        {
			KDEBUG(("Basic assurance test failed (0xFC)\n"));
		}
	}

    // Initiailise the mouse controller
    vt_send_command(CMD_AUX_ON); // enable 2nd port
    vt_flush();

    response = vt_device_send_command(2, 0xFF); // should return 0xFA (ACK)
    if (response != 0xFA) {
        KDEBUG(("Mouse reset: expected ACK (0xFA), got %02X\n", response));
    }

    vt_wait_read();
    response = PS2_REG(PS2_DATA);  // should be 0x00 (mouse ID)
    KDEBUG(("Got mouse ID: %02X\n", response));

    // --- Enable streaming mode ---
    response = vt_device_send_command(2, 0xF4); // enable data reporting
    if (response != 0xFA) {
        KDEBUG(("Mouse enable stream failed: got %02X\n", response));
    }

    KDEBUG(("Mouse init complete\n"));
    KDEBUG(("VT82C42 Init complete\n"));

    vt_flush();
    // Enable interrupts
    uint8_t cfg = vt_get_config_byte();
    vt_set_config_byte(cfg | CMD_BYTE_KBD_INT | CMD_BYTE_AUX_INT);

    return 1;
}

void vt_process_mouse(uint8_t *process)
{
    int8_t packet[3];
    uint8_t status = process[0];

    packet[0] = MOUSE_REL_POS_REPORT;
    if (status & 0x01)
        packet[0] |= LEFT_BUTTON_DOWN;
    if (status & 0x02)
        packet[0] |= RIGHT_BUTTON_DOWN;
    // Mouse positions
    packet[1] = (int8_t)packet[1];
    packet[2] = (int8_t)packet[2];

    KDEBUG(("Mouse: X=%d Y=%d S=%X\n", (int)packet[1], (int)packet[2], packet[0]));

    call_mousevec(packet);
}

void vt_handle_mouse(uint8_t data)
{
    static uint8_t mouse_cycle = 0;
    static uint8_t mouse_bytes[3] = { 0 };

    switch (mouse_cycle)
    {
        case 0:
            // FIrst byte should have bit 3 set (sync)
            if (!(data & 0x08))
                return;

            mouse_bytes[0] = data;
            mouse_cycle = 1;
            break;
        
        case 1:
            mouse_bytes[1] = data;
            mouse_cycle = 2;
            break;

        case 2:
            mouse_bytes[2] = data;
            mouse_cycle = 0;
            vt_process_mouse(mouse_bytes);
            break;
    }
}

void vt_process_scancode(uint8_t sc)
{
    static uint8_t key_break = 0;
    static uint8_t key_extended = 0;
    static uint8_t key_remaining = 0;

	uint8_t register chr;

    if (key_remaining > 0)
    {
        key_remaining--;
        return;
    }
    else if (sc == SCAN_CODE_BREAK)
        key_break = 1;
    else if (sc == SCAN_CODE_MODIFIER)
        key_extended  = 1;
    else if (sc == SCAN_CODE_PSBRK)
    {
        // Pause/Break keys extended sequence, ignore for now
        key_remaining = 7;
    }
    else
    {
        if (sc == SCAN_CODE_CAPLOCK)
        {
            g_key_mode ^= STATUS_CAPS_LOCK;
            vt_set_leds(g_key_mode);
        }
        else if (sc == SCAN_CODE_NUMLOCK)
        {
            g_key_mode ^= STATUS_NUM_LOCK;
            vt_set_leds(g_key_mode);
        }
        else if (sc == SCAN_CODE_SCRLOCK)
        {
            g_key_mode ^= STATUS_SCROLL_LOCK;
            vt_set_leds(g_key_mode);
        }

        sc &= 0x7f;

        if (key_extended)
            chr = st_extended_make_code_map[sc];
        else
            chr = st_make_code_map[sc];

        if (key_break)
            chr |= 0x80; // set break code

        
        KDEBUG(("call_ikbdraw 0x%02x\n", chr));
        call_ikbdraw(chr);
        key_extended = 0;
        key_break = 0;        
	}
}

#endif
