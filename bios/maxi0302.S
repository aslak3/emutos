		.include "bios/maxi030.i"

		.global _maxi030_int_timer

_maxi030_int_timer:
		jsr _int_timerc
		move.b #1,TIMERCONTROL
		rte
