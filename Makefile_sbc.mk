

#
# Tiny68K image
#

TINY68K_IMG = tiny68k.img

.PHONY: tiny68k
NODEP += tiny68k
tiny68k: UNIQUE = $(COUNTRY)
tiny68k: OPTFLAGS = $(STANDARD_OPTFLAGS)
tiny68k: override DEF += -DMACHINE_TINY68K
tiny68k: WITH_AES = 0
tiny68k:
	$(MAKE) DEF='$(DEF)' OPTFLAGS='$(OPTFLAGS)' UNIQUE=$(UNIQUE) WITH_AES=$(WITH_AES) emutos.img
	cp emutos.img $(TINY68K_IMG)
	@MEMBOT=$(call SHELL_SYMADDR,__end_os_stram,emutos.map);\
	echo "# RAM used: $$(($$MEMBOT)) bytes ($$(($$MEMBOT - $(MEMBOT_TOS206))) bytes more than TOS 2.06)"
	@printf "$(LOCALCONFINFO)"

#
# ROSCO_M68K Classic V2 image
#

ROSCO_V2_IMG = rosco_v2.img

.PHONY: rosco_v2
NODEP += rosco_v2
rosco_v2: UNIQUE = $(COUNTRY)
rosco_v2: OPTFLAGS = $(STANDARD_OPTFLAGS)
rosco_v2: override DEF += -DMACHINE_ROSCO_V2
rosco_v2: WITH_AES = 0
rosco_v2:
	$(MAKE) DEF='$(DEF)' OPTFLAGS='$(OPTFLAGS)' UNIQUE=$(UNIQUE) WITH_AES=$(WITH_AES) emutos.img
	cp emutos.img $(ROSCO_V2_IMG)
	@MEMBOT=$(call SHELL_SYMADDR,__end_os_stram,emutos.map);\
	echo "# RAM used: $$(($$MEMBOT)) bytes ($$(($$MEMBOT - $(MEMBOT_TOS206))) bytes more than TOS 2.06)"
	@printf "$(LOCALCONFINFO)"
