/* #define ENABLE_KDEBUG  */

#include "emutos.h"
#include "ddraig.h"
#include "vectors.h"
#include "tosvars.h"
#include "bios.h"
#include "processor.h"
#include "biosext.h"            /* for cache control routines */
#include "gemerror.h"
#include "ikbd.h"               /* for call_mousevec() */
#include "screen.h"
#include "videl.h"
#include "delay.h"
#include "asm.h"
#include "string.h"
#include "disk.h"
#include "biosmem.h"
#include "bootparams.h"
#include "machine.h"
#include "has.h"
#include "../bdos/bdosstub.h"

#ifdef MACHINE_DDRAIG68K


UWORD ddraig_screen_width;
UWORD ddraig_screen_height;
const UBYTE *ddraig_screenbase;

static void ddraig_set_videomode(UWORD width, UWORD height)
{
    KDEBUG(("ddraig_set_videomode()\n"));
    ddraig_screen_width = width;
    ddraig_screen_height = height;

    drvga_write_control_reg(DISPMODE_BITMAPHIRES);

    sshiftmod = FALCON_REZ;
}

WORD ddraig_check_moderez(WORD moderez)
{
    KDEBUG(("ddraig_check_moderez()\n"));
    WORD current_mode, return_mode;

    if (moderez == 0xff02) /* ST High */
        moderez = VIDEL_COMPAT|VIDEL_TRUECOLOR|VIDEL_80COL|VIDEL_VERTICAL;

    if (moderez < 0)                /* ignore other ST video modes */
        return 0;

    current_mode = ddraig_vgetmode();
    return_mode = moderez;          /* assume always valid */
    return (return_mode == current_mode) ? 0 : return_mode;
}

void ddraig_get_current_mode_info(UWORD *planes, UWORD *hz_rez, UWORD *vt_rez)
{   
    KDEBUG(("ddraig_get_current_mode_info()\n"));

    *planes = 1;
    *hz_rez = ddraig_screen_width;
    *vt_rez = ddraig_screen_height;
}


void ddraig_setphys(const UBYTE *addr)
{
    KDEBUG(("ddraig_setphys(%p)\n", addr));
    ddraig_screenbase = addr;
}

const UBYTE *ddraig_physbase(void)
{
    KDEBUG(("ddraig_physbase()\n"));
    return ddraig_screenbase;
}

void ddraig_setrez(WORD rez, WORD videlmode)
{
    KDEBUG(("ddraig_setrez()\n"));
    // Fixed for now
    ddraig_set_videomode(640, 480);
}

WORD ddraig_vgetmode(void)
{
    KDEBUG(("ddraig_vgetmode()\n"));

    WORD mode = VIDEL_TRUECOLOR;

    if (ddraig_screen_width == 640)
        mode |= VIDEL_80COL;

    if (ddraig_screen_height == 480)
        mode |= VIDEL_VGA;
    return mode;
}


#endif /* MACHINE_DDRAIG68K */
