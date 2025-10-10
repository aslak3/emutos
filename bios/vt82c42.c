#include "config.h"
#include <stdint.h>

// Keyboard initialisation improved to stop spurios charater interrupts occuringh
// Based on Tom Storey's code https://github.com/tomstorey/emutos/blob/comet68k/bios/comet_vga.c


/* #define ENABLE_KDEBUG */

#ifdef CONF_WITH_VT82C42

#include "emutos.h"
#include "ikbd.h"
#include "vectors.h"
#include "asm.h"
#include "tosvars.h"

#include <stdio.h>
#include <ctype.h>

#include "vt82c42.h"

#define PS2_BASE                0x00F7F200
#define PS2_READ(r) (*(volatile UBYTE *)(PS2_BASE + (r)))
#define PS2_WRITE(v, r) (*(volatile UBYTE *)(PS2_BASE + (r)) = (v))

// Register Offsets
#define PS2_DATA		  		0x00
#define PS2_CMD 				0x02
#define PS2_STAT				0x02

// For mouse
#define MOUSE_REL_POS_REPORT    0xf8    /* values for mouse_packet[0] */
#define RIGHT_BUTTON_DOWN       0x01    /* these values are OR'ed in */
#define LEFT_BUTTON_DOWN        0x02

#define WAIT_TIMEOUT 10000

enum vt_port {
    PORT_KB = 0,
    PORT_MS = 1
};

// Function prototypes
static void vt_delay(ULONG count);
static void vt_write_wait(const UBYTE data, const UBYTE reg);
static UBYTE vt_cmd_data_polled(const UBYTE cmd);
static UBYTE vt_data_data_polled(const UBYTE data);
static UBYTE vt_data_polled(void);
static UBYTE vt_send_device_cmd(const enum vt_port port, UBYTE cmd);
static UBYTE vt_get_cmd_byte(void);
static void vt_set_cmd_byte(const UBYTE cmd);
static UBYTE device_keyboard_reset(void);
static void device_keyboard_led_animate(void);
static void vt_set_leds(UBYTE leds);
static void vt_flush(void);

void __attribute__((interrupt)) vt_interrupt_handler(void);
void vt_process_scancode(UBYTE sc);
void vt_process_mouse(int8_t *packet);
void vt_handle_mouse(UBYTE data);

static UBYTE g_key_mode = 0;

static const UBYTE st_make_code_map[] = {
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

static const UBYTE st_extended_make_code_map[] = {
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

static void vt_delay(ULONG count)
{
    volatile ULONG delay = count;

    while (delay--)
    {}
}

static void vt_write_wait(const UBYTE data, const UBYTE reg)
{
    UBYTE val;
    volatile ULONG timeout = WAIT_TIMEOUT;
    
    /* Wait until input buffer empty */
    do {
        val = PS2_READ(PS2_CMD);
        if (--timeout == 0)
            break;
    } while (val & STATUS_IBF);

    /* Send the command */
    PS2_WRITE(data, reg);
}

static UBYTE vt_cmd_data_polled(const UBYTE cmd)
{
    UBYTE val;
    volatile ULONG timeout = WAIT_TIMEOUT;

    /* Send the command */
    vt_write_wait(cmd, PS2_CMD);

    /* Wait for the response by polling the OBF flag of the status register */
    do {
        val = PS2_READ(PS2_CMD);
        if (--timeout == 0)
            break;        
    } while (!(val & STATUS_OBF));

    /* Return the value from the data register */
    val = PS2_READ(PS2_DATA);

    return val;
}

static UBYTE vt_data_data_polled(const UBYTE data)
{
    UBYTE val;
    volatile ULONG timeout = WAIT_TIMEOUT;

    /* Write the data */
    vt_write_wait(data, PS2_DATA);

    /* Wait for the response by polling the OBF flag of the status register */
    do {
        val = PS2_READ(PS2_CMD);
        if (--timeout == 0)
            break;        
    } while (!(val & STATUS_OBF));

    /* Return the value from the data register */
    val = PS2_READ(PS2_DATA);

    return val;
}

static UBYTE vt_data_polled(void)
{
    UBYTE val;
    volatile ULONG timeout = WAIT_TIMEOUT;

    /* Wait for the response by polling the OBF flag of the status register */
    do {
        val = PS2_READ(PS2_CMD);
        if (--timeout == 0)
            break;      
    } while (!(val & STATUS_OBF));

    /* Return the value from the data register */
    val = PS2_READ(PS2_DATA);

    return val;
}

static UBYTE vt_send_device_cmd(const enum vt_port port, UBYTE cmd)
{
    UBYTE retries = 10;
    UBYTE val;

    while (retries--) {
        if (port == PORT_MS) {
            /* Will write to the mouse output port */
            vt_write_wait(0xD4, PS2_CMD);
        }

        val = vt_data_data_polled(cmd);

        if (retries == 1 || val != 0xFE) {
            break;
        }

        /* Delay until before retrying */
        vt_delay(500);
    }

    return val;
}

static UBYTE vt_get_cmd_byte(void)
{
    return vt_cmd_data_polled(0x20);
}

static void vt_set_cmd_byte(const UBYTE cmd)
{
    vt_write_wait(0x60, PS2_CMD);
    vt_write_wait(cmd, PS2_DATA);
}

static UBYTE device_keyboard_reset(void)
{
    UBYTE val;

    /* Send the keyboard reset command */
    val = vt_send_device_cmd(PORT_KB, 0xFF);

    if (val != 0xFA) {
        /* Keyboard did not acknowledge reset command */
        return 0;
    }

    /* Keyboard acknowledged the reset command, it should also indicate whether the reset was successful */
    val = vt_data_polled();

    if (val != 0xAA) {
        return 0;
    }

    /* Reset succeeded */
    return 1;
}

static void device_keyboard_led_animate(void)
{
    /* Animation pattern: none -> num -> caps -> scroll -> none -> num - 0xFF terminates */
    const UBYTE anim[] = {0, STATUS_NUM_LOCK, STATUS_CAPS_LOCK, STATUS_SCROLL_LOCK, 0, STATUS_NUM_LOCK, 0xFF};

    UBYTE i;

    for (i = 0;; i++) {
        if (anim[i] == 0xFF) {
            /* Animation done */
            break;
        }

        /* Set LED */
        (void)vt_data_data_polled(0xED);
        (void)vt_data_data_polled(anim[i]);

        /* Update LED status for tracking purposes */
        g_key_mode = anim[i];

        /* Delay between transitions */
        vt_delay(2000);
    }
}

static void vt_set_leds(UBYTE leds)
{
    vt_data_data_polled(KBD_CMD_LED);
	vt_data_data_polled(leds);
}

static void vt_flush(void)
{
    UBYTE status;
    /* Flush the output buffer */
    for (;;)
    {
        status = PS2_READ(PS2_CMD);

        if (status & STATUS_OBF) {
            (void)PS2_READ(PS2_DATA);
            KDEBUG(("vt8242_init(): flush\n"));
        } else {
            break;
        }
    }
}

//	keyboard interrupt handler
void __attribute__((interrupt)) vt_interrupt_handler(void)
{
    UBYTE status = PS2_READ(PS2_STAT);
    UBYTE data = PS2_READ(PS2_DATA);

    // Bit 5 set, mouse data
    if (status & 0x20)
        vt_handle_mouse(data);
    else if (status & 0x01)
        vt_process_scancode(data);
}

UBYTE vt8242_init(void)
{
    volatile PFVOID *vector_addr;
    UBYTE data;

    KDEBUG(("vt8242_init()\n"));
    WORD old_sr;
    /* disable interrupts */
    old_sr = set_sr(0x2700);

    /* Disable keyboard and mouse ports, disable interrupts and translation */
    vt_set_cmd_byte(CMD_BYTE_AUX_OFF | CMD_BYTE_KBD_OFF);
    vt_flush();

    KDEBUG(("vt8242_init: install keyboard interrupt handler\n"));
    vector_addr = &VEC_LEVEL1 + (CONF_VT82C42_AUTOVECTOR - 1);
    *vector_addr = (PFVOID)vt_interrupt_handler;

    KDEBUG(("vt8242_init: controller self test\n"));
    data = vt_cmd_data_polled(CMD_DIAG);
	if (data != KBD_STATUS_DIAG_OK)
		KDEBUG(("vt8242_init: PS/2 keyboard controller FAILED.\n"));
    else
        KDEBUG(("vt8242_init: PS/2 keyboard controller passed.\n"));

    /* Controller firmware/hardware versions */
    data = vt_cmd_data_polled(CMD_VERSION_CTRL);
    KDEBUG(("vt8242_init: Version (A1)=%02X\n", data));
    data = vt_cmd_data_polled(CMD_VERSION);
    KDEBUG(("vt8242_init: Version (AF)=%02X\n", data));

    /* Operating mode */
    data = vt_cmd_data_polled(CMD_GET_MODE);
    if (data == 0x01) {
        KDEBUG(("vt8242_init: PS/2 mode\n"));
    } else {
        KDEBUG(("vt8242_init: AT mode (unsupported)\n"));
    }

	KDEBUG(("vt8242_init: starting keyboard test.\n"));
    data = vt_cmd_data_polled(CMD_KBD_TEST);
	if (data != 0x00)
    {
		KDEBUG(("vt8242_init: PS/2 keyboard test failed.\n"));
	}

    /* Enable the keyboard interface */
    vt_write_wait(CMD_KBD_ON, PS2_CMD);

    /* Reset keyboard */
    // if (device_keyboard_reset() != 1) {
    //     KDEBUG(("vt8242_init(): Keyboard reset failed\n"));
    // }

    /* Do a little LED animation :) */
    device_keyboard_led_animate();

    /* Initialise the mouse controller */
    vt_write_wait(CMD_AUX_ON, PS2_CMD); // enable 2nd port

    data = vt_send_device_cmd(PORT_MS, MOUSE_CMD_RATE);
    if (data != 0xFA) {
        KDEBUG(("vt8242_init(): Mouse CMD_RATE failed, got %02X\n", data));
    }
    data = vt_send_device_cmd(PORT_MS, 20); // Set sample rate to 20 reports/sec
    if (data != 0xFA) {
        KDEBUG(("vt8242_init(): Mouse CMD_RATE data failed, got %02X\n", data));
    }

    vt_send_device_cmd(PORT_MS, MOUSE_CMD_RESOLUTION);
    vt_send_device_cmd(PORT_MS, 1);

    /* Enable streaming mode */
    data = vt_send_device_cmd(PORT_MS, MOUSE_CMD_DATAEN); // enable data reporting
    if (data != 0xFA) {
        KDEBUG(("Mouse enable stream failed: got %02X\n", data));
    }

    KDEBUG(("Mouse init complete\n"));
    KDEBUG(("VT82C42 Init complete\n"));

    vt_flush();

    // Enable interrupts
    UBYTE cfg = vt_get_cmd_byte();
    vt_set_cmd_byte(cfg | CMD_BYTE_KBD_INT | CMD_BYTE_AUX_INT);

    /* restore interrupts */
    set_sr(old_sr);    

    return 1;
}

void vt_process_mouse(int8_t *process)
{
    int8_t packet[3];
    UBYTE status = (UBYTE)process[0];


    packet[0] = MOUSE_REL_POS_REPORT;
    if (status & 0x01)
        packet[0] |= LEFT_BUTTON_DOWN;
    if (status & 0x02)
        packet[0] |= RIGHT_BUTTON_DOWN;
    // Mouse positions
    packet[1] = process[1];
    packet[2] = -process[2];

    KDEBUG(("Mouse: X=%d Y=%d B=%d\n", (int)packet[1], (int)packet[2], packet[0] & 0x03));

    call_mousevec(packet);
}

void vt_handle_mouse(UBYTE data)
{
    static UBYTE mouse_cycle = 0;
    static UBYTE mouse_bytes[3] = { 0 };

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
            vt_process_mouse((int8_t *)mouse_bytes);
            break;
    }
}

void vt_process_scancode(UBYTE sc)
{
    static UBYTE key_break = 0;
    static UBYTE key_extended = 0;
    static UBYTE key_remaining = 0;

	UBYTE register chr;

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
