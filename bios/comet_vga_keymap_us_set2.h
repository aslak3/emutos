#ifndef COMET_VGA_KEYMAP_H
#define COMET_VGA_KEYMAP_H

/* 0 means the code should be consumed and not queued with ikbdraw().
 *
 * -1 means they code needs special handling - e.g. sending an ASCII character via push_ascii_ikbdiorec().
 *
 * Any other value is the translated code that is to be queued with ikbdraw().
 */

/* The maximum value of a valid key code that should be accepted */
#define COMET_VGA_MAX_KEY_CODE 0x83

/* Map for normal key presses - numlock disengaged */
static const UBYTE ps2_scancode_map[] = {
    0,                              0x43 /* F9 */,                  0,                              0x3F /* F5 */,
    0x3D /* F3 */,                  0x3B /* F1 */,                  0x3C /* F2 */,                  0x58 /* F12 */,
    0,                              0x44 /* F10 */,                 0x42 /* F8 */,                  0x40 /* F6 */,
    0x3E /* F4 */,                  0x0F /* Tab */,                 0x29 /* ` (backtick) */,        0,

    0,                              0x38 /* Left Alt */,            0x2A /* Left Shift */,          0,
    0x1D /* Left Ctrl */,           0x10 /* Q */,                   0x02 /* 1 */,                   0,
    0,                              0,                              0x2C /* Z */,                   0x1F /* S */,
    0x1E /* A */,                   0x11 /* W */,                   0x03 /* 2 */,                   0,

    0,                              0x2E /* C */,                   0x2D /* X */,                   0x20 /* D */,
    0x12 /* E */,                   0x05 /* 4 */,                   0x04 /* 3 */,                   0,
    0,                              0x39 /* Space */,               0x2F /* V */,                   0x21 /* F */,
    0x14 /* T */,                   0x13 /* R */,                   0x06 /* 5 */,                   0,

    0,                              0x31 /* N */,                   0x30 /* B */,                   0x23 /* H */,
    0x22 /* G */,                   0x15 /* Y */,                   0x07 /* 6 */,                   0,
    0,                              0,                              0x32 /* M */,                   0x24 /* J */,
    0x16 /* U */,                   0x08 /* 7 */,                   0x09 /* 8 */,                   0,

    0,                              0x33 /* , */,                   0x25 /* K */,                   0x17 /* I */,
    0x18 /* O */,                   0x0B /* 0 */,                   0x0A /* 9 */,                   0,
    0,                              0x34 /* . */,                   0x35 /* / */,                   0x26 /* L */,
    0x27 /* ; */,                   0x19 /* P */,                   0x0C /* - */,                   0,

    0,                              0,                              0x28 /* ' */,                   0,
    0x1A /* [ */,                   0x0D /* = */,                   0,                              0,
    -1   /* Caps Lock */,           0x36 /* Right Shift */,         0x1C /* Enter */,               0x1B /* ] */,
    0,                              0x2B /* \ */,                   0,                              0,

    0,                              0,                              0,                              0,
    0,                              0,                              0x0E /* Backspace */,           0,
    0,                              0x4F /* Numpad End */,          0,                              0x4B /* Numpad Left */,
    0x47 /* Numpad Home */,         0,                              0,                              0,

    0x52 /* Numpad Insert */,       0x53 /* Numpad Del */,          0x50 /* Numpad Down */,         0,
    0x4D /* Numpad Right */,        0x48 /* Numpad Up */,           0x01 /* Esc */,                 -1   /* Num Lock */,
    0x57 /* F11 */,                 0x4E /* Numpad + */,            0x51 /* Numpad Pg Dn */,        0x4A /* Numpad - */,
    -1   /* Numpad * */,            0x49 /* Numpad Pg Up */,        -1   /* Scroll Lock */,         0,

    0,                              0,                              0,                              0x41 /* F7 */
};

/* Map for normal key presses - numlock engaged */
static const UBYTE ps2_scancode_map_numlock[] = {
    0,                              0x43 /* F9 */,                  0,                              0x3F /* F5 */,
    0x3D /* F3 */,                  0x3B /* F1 */,                  0x3C /* F2 */,                  0x58 /* F12 */,
    0,                              0x44 /* F10 */,                 0x42 /* F8 */,                  0x40 /* F6 */,
    0x3E /* F4 */,                  0x0F /* Tab */,                 0x29 /* ` (backtick) */,        0,

    0,                              0x38 /* Left Alt */,            0x2A /* Left Shift */,          0,
    0x1D /* Left Ctrl */,           0x10 /* Q */,                   0x02 /* 1 */,                   0,
    0,                              0,                              0x2C /* Z */,                   0x1F /* S */,
    0x1E /* A */,                   0x11 /* W */,                   0x03 /* 2 */,                   0,

    0,                              0x2E /* C */,                   0x2D /* X */,                   0x20 /* D */,
    0x12 /* E */,                   0x05 /* 4 */,                   0x04 /* 3 */,                   0,
    0,                              0x39 /* Space */,               0x2F /* V */,                   0x21 /* F */,
    0x14 /* T */,                   0x13 /* R */,                   0x06 /* 5 */,                   0,

    0,                              0x31 /* N */,                   0x30 /* B */,                   0x23 /* H */,
    0x22 /* G */,                   0x15 /* Y */,                   0x07 /* 6 */,                   0,
    0,                              0,                              0x32 /* M */,                   0x24 /* J */,
    0x16 /* U */,                   0x08 /* 7 */,                   0x09 /* 8 */,                   0,

    0,                              0x33 /* , */,                   0x25 /* K */,                   0x17 /* I */,
    0x18 /* O */,                   0x0B /* 0 */,                   0x0A /* 9 */,                   0,
    0,                              0x34 /* . */,                   0x35 /* / */,                   0x26 /* L */,
    0x27 /* ; */,                   0x19 /* P */,                   0x0C /* - */,                   0,

    0,                              0,                              0x28 /* ' */,                   0,
    0x1A /* [ */,                   0x0D /* = */,                   0,                              0,
    -1   /* Caps Lock */,           0x36 /* Right Shift */,         0x1C /* Enter */,               0x1B /* ] */,
    0,                              0x2B /* \ */,                   0,                              0,

    0,                              0,                              0,                              0,
    0,                              0,                              0x0E /* Backspace */,           0,
    0,                              0x02 /* Numpad 1 */,            0,                              0x05 /* Numpad 4 */,
    0x08 /* Numpad 7 */,            0,                              0,                              0,

    0x0B /* Numpad 0 */,            -1   /* Numpad . */,            0x03 /* Numpad 2 */,            0x06 /* Numpad 5 */,
    0x07 /* Numpad 6 */,            0x09 /* Numpad 8 */,            0x01 /* Esc */,                 -1   /* Num Lock */,
    0x57 /* F11 */,                 0x4E /* Numpad + */,            0x04 /* Numpad 3 */,            0x4A /* Numpad - */,
    -1   /* Numpad * */,            0x0A /* Numpad 9 */,            -1   /* Scroll Lock */,         0,

    0,                              0,                              0,                              0x41 /* F7 */
};

/* Key map for single escaped keys */
static const UBYTE ps2_extended_scancode_map[] = {
    0,                              0,                              0,                              0,
    0,                              0,                              0,                              0,
    0,                              0,                              0,                              0,
    0,                              0,                              0,                              0,

    0,                              0x38 /* Right Alt */,           0,                              0,
    0x1D /*Right Ctrl */,           0,                              0,                              0,
    0,                              0,                              0,                              0,
    0,                              0,                              0,                              0,

    0,                              0,                              0,                              0,
    0,                              0,                              0,                              0,
    0,                              0,                              0,                              0,
    0,                              0,                              0,                              0,

    0,                              0,                              0,                              0,
    0,                              0,                              0,                              0,
    0,                              0,                              0,                              0,
    0,                              0,                              0,                              0,

    0,                              0,                              0,                              0,
    0,                              0,                              0,                              0,
    0,                              0,                              0x35 /* Numpad / */,            0,
    0,                              0,                              0,                              0,

    0,                              0,                              0,                              0,
    0,                              0,                              0,                              0,
    0,                              0,                              0x1C /* Numpad Enter */,        0,
    0,                              0,                              0,                              0,

    0,                              0,                              0,                              0,
    0,                              0,                              0,                              0,
    0,                              0,                              0,                              0,
    0,                              0,                              0,                              0,

    0,                              0,                              0,                              0,
    0,                              0,                              0,                              0,
    0,                              0,                              0,                              0,
    0,                              0,                              0,                              0,

    0,                              0,                              0,                              0
};

/* Key map for double escaped keys */
static const UBYTE ps2_extended2_scancode_map[] = {
    0,                              0,                              0,                              0,
    0,                              0,                              0,                              0,
    0,                              0,                              0,                              0,
    0,                              0,                              0,                              0,

    0,                              0,                              0,                              0,
    0,                              0,                              0,                              0,
    0,                              0,                              0,                              0,
    0,                              0,                              0,                              0,

    0,                              0,                              0,                              0,
    0,                              0,                              0,                              0,
    0,                              0,                              0,                              0,
    0,                              0,                              0,                              0,

    0,                              0,                              0,                              0,
    0,                              0,                              0,                              0,
    0,                              0,                              0,                              0,
    0,                              0,                              0,                              0,

    0,                              0,                              0,                              0,
    0,                              0,                              0,                              0,
    0,                              0,                              0,                              0,
    0,                              0,                              0,                              0,

    0,                              0,                              0,                              0,
    0,                              0,                              0,                              0,
    0,                              0,                              0,                              0,
    0,                              0,                              0,                              0,

    0,                              0,                              0,                              0,
    0,                              0,                              0,                              0,
    0,                              0x4F /* End */,                 0,                              0x4B /* Left arrow */,
    0x47 /* Home */,                0,                              0,                              0,

    0x52 /* Insert */,              0x53 /* Delete */,              0x50 /* Down arrow */,          0,
    0x4D /* Right arrow */,         0x48 /* Up arrow */,            0,                              0,
    0,                              0,                              0x51 /* Page Down */,           0,
    0,                              0x49 /* Page Up */,             0,                              0,

    0,                              0,                              0,                              0
};

#endif /* COMET_VGA_KEYMAP_H */
