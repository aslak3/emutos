		.include "bios/maxi000.i"

		.global _maxi000_int_timer

_maxi000_int_timer:
		jsr _int_timerc
		move.w #0,TIMERCOUNT
		rte
