#ifndef DDRAIG_H
#define DDRAIG_H

#include "ddraig_vga.h"

#ifdef MACHINE_DDRAIG68K

extern UWORD ddraig_screen_width;
extern UWORD ddraig_screen_height;
extern const UBYTE *ddraig_screenbase;

WORD ddraig_check_moderez(WORD moderez);
void ddraig_get_current_mode_info(UWORD *planes, UWORD *hz_rez, UWORD *vt_rez);
void ddraig_setphys(const UBYTE *addr);
const UBYTE *ddraig_physbase(void);
void ddraig_setrez(WORD rez, WORD videlmode);
WORD ddraig_vgetmode(void);

#endif

#endif /* DDRAIG_H */
