#include "config.h"
#include <stdint.h>

// Keyboard initialisation improved to stop spurious charater interrupts occuringh
// Based on Tom Storey's code https://github.com/tomstorey/emutos/blob/comet68k/bios/comet_vga.c

#define ENABLE_KDEBUG

#ifdef CONF_WITH_VT82C42

#include "emutos.h"
#include "ikbd.h"
#include "vectors.h"
#include "asm.h"
#include "tosvars.h"

#include <stdio.h>
#include <ctype.h>

#include "vt82c42.h"
#include "comet_vga_keymap_us_set2.h"

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

enum VT82C42_port {
    VT82C42_KB = 0,
    VT82C42_MS = 1
};

/* Keyboard state machine states */
enum key_state {
    KEY_STATE_DEFAULT = 0,
    KEY_STATE_UNTIL_BREAK,
    KEY_STATE_ESCAPE,
    KEY_STATE_PAUSE_BREAK
};

// Function prototypes
static void vt82c42_delay(ULONG count);
static void vt82c42_write_wait(UBYTE data, UBYTE reg);
static UBYTE vt82c42_cmd_data_polled(UBYTE cmd);
static UBYTE vt82c42_data_data_polled(UBYTE data);
static UBYTE vt82c42_data_polled(void);
static BOOL vt82c42_data_polled_timeout(UBYTE *data);
static UBYTE vt82c42_send_device_cmd(enum VT82C42_port port, UBYTE cmd);
static UBYTE vt82c42_get_cmd_byte(void);
static void vt82c42_set_cmd_byte(UBYTE cmd);
static BOOL device_keyboard_init(void);
static BOOL device_keyboard_reset(void);
static void device_keyboard_led_animate(void);
static BOOL device_mouse_init(void);
static BOOL device_mouse_reset(void);
static BOOL device_mouse_configure(void);
void keyboard_set_leds(UBYTE led);
void vt82c42_flush(void);

void __attribute__((interrupt)) vt_interrupt_handler(void);
void vt_process_scancode(UBYTE sc);
void vt_process_mouse(int8_t *packet);
void vt_handle_mouse(UBYTE data);

static UBYTE g_key_mode = 0;
static SBYTE mouse_packet[3];

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

static void vt82c42_delay(ULONG count)
{
    volatile ULONG delay = count;

    while (delay--)
    {}
}

static void vt82c42_write_wait(const UBYTE data, const UBYTE reg)
{
    UBYTE val;

    /* Wait until input buffer empty */
    do {
        val = PS2_READ(PS2_CMD);
    } while (val & STATUS_IBF);

    /* Send the command */
    PS2_WRITE(data, reg);
}

static UBYTE vt82c42_cmd_data_polled(const UBYTE cmd)
{
    UBYTE val;

    /* Send the command */
    vt82c42_write_wait(cmd, PS2_CMD);

    /* Wait for the response by polling the OBF flag of the status register */
    do {
        val = PS2_READ(PS2_CMD);
    } while (!(val & STATUS_OBF));

    /* Return the value from the data register */
    val = PS2_READ(PS2_DATA);

    return val;
}

static UBYTE vt82c42_data_data_polled(const UBYTE data)
{
    UBYTE val;

    /* Write the data */
    vt82c42_write_wait(data, PS2_DATA);

    /* Wait for the response by polling the OBF flag of the status register */
    do {
        val = PS2_READ(PS2_CMD);
    } while (!(val & STATUS_OBF));

    /* Return the value from the data register */
    val = PS2_READ(PS2_DATA);

    return val;
}

static UBYTE vt82c42_data_polled(void)
{
    UBYTE val;

    /* Wait for the response by polling the OBF flag of the status register */
    do {
        val = PS2_READ(PS2_CMD);
    } while (!(val & STATUS_OBF));

    /* Return the value from the data register */
    val = PS2_READ(PS2_DATA);

    return val;
}

static BOOL vt82c42_data_polled_timeout(UBYTE *data)
{
    UBYTE val;
    LONG timer;

    /* Set a timeout for how long we will wait for a response */
    timer = hz_200 + 20;

    /* Wait for the response by polling the OBF flag of the status register */
    do {
        val = PS2_READ(PS2_CMD);
    } while (!(val & STATUS_OBF) && hz_200 < timer);

    if (hz_200 == timer) {
        /* Timeout */
        return FALSE;
    }

    /* Return the value from the data register */
    *data = PS2_READ(PS2_DATA);

    return TRUE;
}

static UBYTE vt82c42_send_device_cmd(const enum VT82C42_port port, const UBYTE cmd)
{
    UBYTE retries = 10;
    UBYTE val;
    uint32_t timer;

    while (retries--) {
        if (port == VT82C42_MS) {
            /* Will write to the mouse output port */
            vt82c42_write_wait(0xD4, PS2_CMD);
        }

        val = vt82c42_data_data_polled(cmd);

        if (retries == 1 || val != 0xFE) {
            break;
        }

        /* Delay until the next tick of the system timer before retrying */
        timer = hz_200 + 5;

        while (timer == hz_200) {}
    }

    return val;
}

static UBYTE vt82c42_get_cmd_byte(void)
{
    return vt82c42_cmd_data_polled(0x20);
}

static void vt82c42_set_cmd_byte(const UBYTE cmd)
{
    vt82c42_write_wait(0x60, PS2_CMD);
    vt82c42_write_wait(cmd, PS2_DATA);
}

static BOOL device_keyboard_init(void)
{
    /* Keyboard interface test */
    if (vt82c42_cmd_data_polled(0xAB) != 0) {
        KDEBUG(("device_keyboard_init(): Keyboard interface test failed\n\r"));

        return FALSE;
    }

    /* Enable the keyboard interface */
    vt82c42_write_wait(0xAE, PS2_CMD);

    /* Reset keyboard */
    if (device_keyboard_reset() != TRUE) {
        KDEBUG(("device_keyboard_init(): Keyboard reset failed - is a working keyboard connected?\n\r"));

        return FALSE;
    }

    /* Do a little LED animation :) */
    device_keyboard_led_animate();

    return TRUE;
}

static BOOL device_keyboard_reset(void)
{
    /* Send the keyboard reset command */
    UBYTE resp = vt82c42_send_device_cmd(VT82C42_KB, 0xFF);
    if (resp != 0xFA) {
        /* Keyboard did not acknowledge reset command */
        KDEBUG(("device_keyboard_reset: exected 0xFA, got 0x%02X\n\r", resp));
        return FALSE;
    }

    /* Keyboard acknowledged the reset command, it should also indicate whether selft tests were successful */
    resp = vt82c42_data_polled();
    if (resp != 0xAA) {
        KDEBUG(("device_keyboard_reset: exected 0xAA, got 0x%02X\n\r", resp));
        return FALSE;
    }

    /* Reset succeeded */
    return TRUE;
}

void keyboard_set_leds(UBYTE led)
{
    (void)vt82c42_data_data_polled(0xED);
    (void)vt82c42_data_data_polled(led);
}

static void device_keyboard_led_animate(void)
{
    /* Animation pattern: none -> num -> caps -> scroll -> none -> num - 0xFF terminates */
    const UBYTE anim[] = {0, STATUS_NUM_LOCK, STATUS_CAPS_LOCK, STATUS_SCROLL_LOCK, 0, STATUS_NUM_LOCK, 0xFF};

    UBYTE i;
    //uint32_t timer;

    for (i = 0;; i++) {
        if (anim[i] == 0xFF) {
            /* Animation done */
            break;
        }

        /* Set LED */
        (void)vt82c42_data_data_polled(0xED);
        (void)vt82c42_data_data_polled(anim[i]);

        /* Update LED status for tracking purposes */
        g_key_mode = anim[i];

        /* Delay between transitions */
        //timer = hz_200 + 20;
        vt82c42_delay(10000);
        //while (hz_200 < timer) {}
    }
}

static BOOL device_mouse_init(void)
{
    /* Mouse interface test */
    if (vt82c42_cmd_data_polled(0xA9) != 0) {
        KDEBUG(("device_mouse_init(): Mouse interface test failed\n\r"));

        return FALSE;
    }

    /* Enable the mouse interface */
    vt82c42_write_wait(0xA8, PS2_CMD);

    /* Reset mouse */
    if (device_mouse_reset() != TRUE) {
        KDEBUG(("device_mouse_init(): Mouse reset failed - is a working mouse connected?\n\r"));

        return FALSE;
    }

    /* Configure mouse */
    if (device_mouse_configure() != TRUE) {
        KDEBUG(("device_mouse_init(): Mouse configuration failed\n\r"));

        return FALSE;
    }

    return TRUE;
}

static BOOL device_mouse_reset(void)
{
    UBYTE success = FALSE;
    UBYTE data = 0xFF;

    /* Send the mouse reset command */
    data = vt82c42_send_device_cmd(VT82C42_MS, 0xFF);
    if (data != 0xFA) {
        /* Mouse did not acknowledge reset command */
        KDEBUG(("device_mouse_reset: exected 0xFA, got 0x%02X\n\r", data));
        return FALSE;
    }

    /* Mouse acknowledged the reset command, it should also indicate whether selft tests were successful */
    if (vt82c42_data_polled() != 0xAA) {
        KDEBUG(("device_mouse_reset: exected 0xAA, got 0x%02X\n\r", data));
        return FALSE;
    }

    /* Finally, the mouse should send an ID to indicate that the device is a mouse */
    success = vt82c42_data_polled_timeout(&data);
    if (success == FALSE || data != 0) {
        KDEBUG(("device_mouse_reset: success = %d, exected 0xAA, got 0x%02X\n\r", success, data));
        return FALSE;
    }

    /* Reset succeeded */
    return TRUE;
}

static BOOL device_mouse_configure(void)
{
    /* Set the report rate */
    if (vt82c42_send_device_cmd(VT82C42_MS, 0xF3) != 0xFA) {
        return FALSE;
    }

    if (vt82c42_send_device_cmd(VT82C42_MS, 10) != 0xFA) {
        return FALSE;
    }

    /* Set the resolution */
    if (vt82c42_send_device_cmd(VT82C42_MS, 0xE8) != 0xFA) {
        return FALSE;
    }

    if (vt82c42_send_device_cmd(VT82C42_MS, 1) != 0xFA) {
        return FALSE;
    }

    /* Enable reporting to start getting updates */
    if (vt82c42_send_device_cmd(VT82C42_MS, 0xF4) != 0xFA) {
        return FALSE;
    }

    return TRUE;
}

void vt82c42_flush(void)
{
    UBYTE status;
    /* Flush the output buffer */
    for (;;) {
        status = PS2_READ(PS2_CMD);

        if (status & STATUS_OBF) {
            (void)PS2_READ(PS2_DATA);
            KDEBUG(("vt82c42_flush(): flush\n\r"));
        } else {
            break;
        }
    }
}

//	keyboard interrupt handler
void __attribute__((interrupt)) vt_interrupt_handler(void)
{
    // /* disable interrupts */
    UWORD old_sr = set_sr(0x2700);

    UBYTE status = PS2_READ(PS2_STAT);
    UBYTE data = PS2_READ(PS2_DATA);

    // Bit 5 set, mouse data
    if (status & STATUS_MS_DATA)
        vt_handle_mouse(data);
    else
        vt_process_scancode(data);

    // /* restore interrupts */
    set_sr(old_sr);
}

UBYTE vt8242_init(void)
{
    volatile PFVOID *vector_addr;
    UBYTE data;
    UBYTE status;
    UBYTE kb_stat;
    UBYTE ms_stat;

    KDEBUG(("vt8242_init()\n"));

    /* Disable keyboard and mouse ports, disable interrupts and translation */
    vt82c42_set_cmd_byte(CMD_BYTE_AUX_OFF | CMD_BYTE_KBD_OFF);
    vt82c42_flush();
    KDEBUG(("vt82c42_init: Keyboard buffer flushed\n\r"));

    KDEBUG(("vt8242_init: install keyboard interrupt handler\n"));
    vector_addr = &VEC_LEVEL1 + (CONF_VT82C42_AUTOVECTOR - 1);
    *vector_addr = (PFVOID)vt_interrupt_handler;

    KDEBUG(("vt82c42_init: controller self test\n\r"));
    data = vt82c42_cmd_data_polled(CMD_DIAG);
	if (data != KBD_STATUS_DIAG_OK)
		KDEBUG(("vt82c42_init: PS/2 keyboard controller FAILED.\n\r"));
    else
        KDEBUG(("vt82c42_init: PS/2 keyboard controller passed.\n\r"));

    /* Controller firmware/hardware versions */
    data = vt82c42_cmd_data_polled(CMD_VERSION_CTRL);
    KDEBUG(("vt82c42_init: Version (A1)=%02X\n\r", data));
    data = vt82c42_cmd_data_polled(CMD_VERSION);
    KDEBUG(("vt82c42_init: Version (AF)=%02X\n\r", data));

    /* Operating mode */
    data = vt82c42_cmd_data_polled(CMD_GET_MODE);
    if (data == 0x01) {
        KDEBUG(("vt82c42_init: PS/2 mode\n\r"));
    } else {
        KDEBUG(("vt82c42_init: AT mode (unsupported)\n\r"));
    }

    /* Check fuse status */
    vt82c42_write_wait(0xC1, PS2_CMD);
    status = PS2_READ(PS2_CMD);
    if (!(status & 0x40)) {
        KDEBUG(("vt82c42_init(): Fuse NOT OK - tests failed\n\r"));
        return 0;
    }

    /* Initialise keyboard and mouse interfaces and devices */
    kb_stat = device_keyboard_init();
    ms_stat = device_mouse_init();

    if (kb_stat == FALSE) {
        /* Disable the keyboard interface because it is unused or errored */
        vt82c42_write_wait(0xAD, PS2_CMD);
    }

    if (ms_stat == FALSE) {
        /* Disable the mouse interface because it is unused or errored */
        vt82c42_write_wait(0xA7, PS2_CMD);
    }

    if (kb_stat == FALSE && ms_stat == FALSE) {
        KDEBUG(("vt82c42_init(): no peripherals, early exit\n\r"));
        return 0;
    }

    KDEBUG(("vt82c42_init(): viable peripherals: "));

    if (kb_stat) {
        KDEBUG(("keyboard "));
    }
    if (ms_stat) {
        KDEBUG(("mouse"));
    }
    KDEBUG(("\n\r"));

    /* Enable interrupt sources in the controller */
    data = vt82c42_get_cmd_byte();

    if (kb_stat) {
        data |= CMD_BYTE_KBD_INT;
    }

    if (ms_stat) {
        data |= CMD_BYTE_AUX_INT;
    }

    vt82c42_set_cmd_byte(data);

    return 1;
}

void vt_process_mouse(int8_t *process)
{
    UBYTE status = (UBYTE)process[0];
    uint8_t xoverflow;
    uint8_t yoverflow;

    mouse_packet[0] = MOUSE_REL_POS_REPORT;
    if (status & 0x01)
        mouse_packet[0] |= LEFT_BUTTON_DOWN;
    if (status & 0x02)
        mouse_packet[0] |= RIGHT_BUTTON_DOWN;
    // Mouse positions
    mouse_packet[1] = process[1];
    mouse_packet[2] = -process[2];

        /* Overflow handling */
    xoverflow = (process[0] >> 6) & 1;
    yoverflow = (process[0] >> 7) & 1;

    if (xoverflow || yoverflow)
    {
        KDEBUG(("Packet0: 0x%02X Mouse overflow x=%d,y=%d\n\r", process[0], xoverflow, yoverflow));
        mouse_packet[1] = 0;
        mouse_packet[2] = 0;
    }
    KDEBUG(("Mouse: X=%d Y=%d B=%d\n", (int)mouse_packet[1], (int)mouse_packet[2], mouse_packet[0] & 0x07));

    call_mousevec(mouse_packet);
}

void vt_handle_mouse(UBYTE data)
{
    static UBYTE pktctr = 0;
    static UBYTE pkt[6] = {0};

    /* Synchronise the mouse handling */
    if (pktctr == 0) {
        /* Check for the start of a new packet */
        if ((data & 0x08) == 0x08) {
            /* Bit 3 set - valid start of packet */
            pkt[pktctr++] = data;
        } else {
            /* Invalid start of packet - ignore */
            KDEBUG(("desync\n"));
            return;
        }
    }
    else {
        /* Collect up to 3 bytes of packet data */
        if (pktctr < 3) {
            pkt[pktctr++] = data;
        }
    }

    /* Once 3 bytes have been collected, process the packet - form a new packet to be queued with EmuTOS */
    if (pktctr == 3) {
        pkt[3] = 0xF8;                      /* MOUSE_REL_POS_REPORT */
        pkt[3] |= (pkt[0] & 0x01) << 1;     /* LEFT_BUTTON_DOWN */
        pkt[3] |= (pkt[0] & 0x02) >> 1;     /* RIGHT_BUTTON_DOWN */
        pkt[4] = pkt[1];                    /* X rel */
        pkt[5] = -pkt[2];                   /* Y rel */

        /* Overflow handling */
        if (pkt[0] & 0x40) {
            /* X overflow */
            pkt[4] = pkt[0] & 0x10 ? -128 : 127;
        }

        if (pkt[0] & 0x80) {
            /* X overflow */
            pkt[5] = pkt[0] & 0x20 ? -128 : 127;
        }

        // KDEBUG(("Mouse: X=%i Y=%i B=%1X\n", (SBYTE)pkt[4], (SBYTE)pkt[5], pkt[0] & 0x03));

        call_mousevec((SBYTE *)&pkt[3]);

        /* Reset packet counter */
        pktctr = 0;
    }
}

void vt_process_scancode(UBYTE code)
{
    static enum key_state state = KEY_STATE_DEFAULT;
    const UBYTE make_code = code & 0x7F;
    static BOOL is_break_code = FALSE;
    UBYTE xlat_code = 0;
    BOOL queue_code = FALSE;
    BOOL update_leds = FALSE;
    static BOOL is_escape2 = FALSE;

    /* Ignore code 0 */
    if (code == 0) {
        return;
    }

    switch (state) {
        case KEY_STATE_ESCAPE:
            if (code == 0xF0) {
                /* Next code will be a break code */
                is_break_code = TRUE;

                return;
            }

            if (code > COMET_VGA_MAX_KEY_CODE) {
                /* Ignore/consume codes that are out of range */
                is_break_code = FALSE;
                /* Should we return to the default state here? */

                return;
            }

            if (code == 0x12) {
                /* Enable/disable double escaped code set */
                is_escape2 = is_break_code ? FALSE : TRUE;
                is_break_code = FALSE;
                state = KEY_STATE_DEFAULT;

                return;
            }

            /* Look up translated code ... */
            xlat_code = is_escape2 ? ps2_extended2_scancode_map[make_code] : ps2_extended_scancode_map[make_code];

            if (xlat_code == 0) {
                /* Ignore/consume this code */
                is_break_code = FALSE;
                state = KEY_STATE_DEFAULT;

                return;
            }

            if ((SBYTE)xlat_code != -1) {
                /* This code will be queued */
                queue_code = TRUE;
                state = KEY_STATE_DEFAULT;
            }

            break;

        case KEY_STATE_UNTIL_BREAK:
            if (code == 0xF0) {
                /* Next code will be a break code */
                is_break_code = TRUE;
                state = KEY_STATE_DEFAULT;
            }

            break;

        case KEY_STATE_PAUSE_BREAK:
            if (code == 0xF0) {
                /* Next code will be a break code */
                is_break_code = TRUE;

                return;
            }

            if (!is_escape2) {
                if (code == 0x14) {
                    /* Abuse the is_escape2 flag to keep track of where we are processing this key */
                    is_escape2 = TRUE;
                } else if (is_break_code && code == 0x77) {
                    /* Sequence complete */
                    is_break_code = FALSE;
                    state = KEY_STATE_DEFAULT;
                }
            } else {
                if (code == 0x77) {
                    /* Pause/Break key pressed */
                    /* TODO: something? */
                    KDEBUG(("vt82c42_handle_key(): Pause/Break\n"));
                } else if (is_break_code && code == 0x14) {
                    is_break_code = FALSE;
                    is_escape2 = FALSE;
                }
            }

            return;

        default:
            if (code == 0xF0) {
                /* Next code will be a break code */
                is_break_code = TRUE;

                return;
            }

            if (code == 0xE0) {
                /* Extended key code */
                state = KEY_STATE_ESCAPE;

                return;
            }

            if (code == 0xE1) {
                /* Probably Pause/Break */
                state = KEY_STATE_PAUSE_BREAK;

                return;
            }

            if (code > COMET_VGA_MAX_KEY_CODE) {
                /* Ignore/consume codes that are out of range */
                is_break_code = FALSE;

                return;
            }

            /* Look up translated code ... */
            xlat_code = g_key_mode & STATUS_NUM_LOCK ? ps2_scancode_map_numlock[make_code] : ps2_scancode_map[make_code];

            if (xlat_code == 0) {
                /* Ignore/consume this code */
                is_break_code = FALSE;

                return;
            }

            if ((SBYTE)xlat_code != -1) {
                /* This code will be queued */
                queue_code = TRUE;
            } else {
                /* Special handling */
                if (make_code == 0x58) {
                    /* Caps lock */
                    if (!is_break_code) {
                        g_key_mode ^= STATUS_CAPS_LOCK;
                        update_leds = TRUE;

                        /* Consume repeats to prevent toggling */
                        state = KEY_STATE_UNTIL_BREAK;
                    }

                    xlat_code = 0x3A;
                    queue_code = TRUE;

                    break;
                }

                if (make_code == 0x7E) {
                    /* Scroll lock */
                    if (!is_break_code) {
                        g_key_mode ^= STATUS_SCROLL_LOCK;
                        update_leds = TRUE;

                        /* Consume repeats to prevent toggling */
                        state = KEY_STATE_UNTIL_BREAK;
                    }

                    xlat_code = 0x46;
                    queue_code = TRUE;

                    break;
                }

                if (make_code == 0x77) {
                    /* Num lock */
                    if (!is_break_code) {
                        g_key_mode ^= STATUS_NUM_LOCK;
                        update_leds = TRUE;

                        /* Consume repeats to prevent toggling */
                        state = KEY_STATE_UNTIL_BREAK;
                    }

                    xlat_code = 0x45;
                    queue_code = TRUE;

                    break;
                }

                if (make_code == 0x7C) {
                    /* Numpad * */
                    if (!is_break_code) {
                        push_ascii_ikbdiorec('*');
                    }

                    is_break_code = FALSE;

                    return;
                }

                if (make_code == 0x71) {
                    /* Numpad . */
                    if (!is_break_code) {
                        push_ascii_ikbdiorec('.');
                    }

                    is_break_code = FALSE;

                    return;
                }
            }
    }

    if (queue_code) {
        if (is_break_code) {
            /* Set MSb for break code */
            xlat_code |= 0x80;

            is_break_code = FALSE;
        }

        call_ikbdraw(xlat_code);
    }

    if (update_leds) {
        /* Set LEDs */
        (void)vt82c42_data_data_polled(0xED);
        (void)vt82c42_data_data_polled(g_key_mode);
    }
}

#endif
