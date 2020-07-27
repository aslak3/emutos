		.include "bios/maxi000.i"

		.global _maxi000_int_timer
		.global _maxi000_update_display

_maxi000_int_timer:
		move.b #0x01,SPIDATA
		jsr _int_timerc
		move.w #0,TIMERCOUNT
		rte

_maxi000_update_display:
		movem.l %a0-%a1/%d0,-(%sp)
		clr.l %d0
		move.l %d0,VGARWADDRHI
|		movea.l #VGADATA,%a1
|		movea.l _v_bas_ad,%a0
|		move.w #640/16*2*480-1,%d0
|1:		move.w (%a0)+,(%a1)
|		dbra %d0,1b
		move.l _v_bas_ad,DMASRCHI
		move.w #640/16*2*480-1,DMALEN
		move.w #0x3,DMAFLAGS
		movem.l (%sp)+,%a0-%a1/%d0
		rts
