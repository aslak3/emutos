#ifdef MACHINE_MAXI030

# ifndef CONF_ATARI_HARDWARE
#  define CONF_ATARI_HARDWARE 0
# endif
# ifndef CONF_WITH_ADVANCED_CPU
#  define CONF_WITH_ADVANCED_CPU 1
# endif
# ifndef CONF_WITH_APOLLO_68080
#  define CONF_WITH_APOLLO_68080 0
# endif
# ifndef CONF_WITH_BUS_ERROR
#  define CONF_WITH_BUS_ERROR 1
# endif
# ifndef CONF_WITH_CACHE_CONTROL
#  define CONF_WITH_CACHE_CONTROL 1
# endif
# ifndef ALWAYS_SHOW_INITINFO
#  define ALWAYS_SHOW_INITINFO 1
# endif

# ifndef CONF_STRAM_SIZE
#  define CONF_STRAM_SIZE 31*1024*1024
# endif
# ifndef CONF_WITH_ALT_RAM
#  define CONF_WITH_ALT_RAM 0
# endif
# ifndef CONF_WITH_MFP
#  define CONF_WITH_MFP 0
# endif

#ifndef CONF_WITH_FDC
# define CONF_WITH_FDC 0
#endif

#ifndef CONF_WITH_ACSI
# define CONF_WITH_ACSI 0
#endif

#ifndef CONF_WITH_SCSI
# define CONF_WITH_SCSI 0
#endif

#define CONF_WITH_VBL_RTE 0

# ifndef CONF_WITH_IDE
#  define CONF_WITH_IDE 0
# endif
# ifndef CONF_ATARI_IDE
#  define CONF_ATARI_IDE 0
# endif
# ifndef CONF_IDE_NO_RESET
#  define CONF_IDE_NO_RESET 0
# endif

# ifndef CONF_WITH_SDMMC
#  define CONF_WITH_SDMMC 0
# endif

# ifndef CONF_WITH_RESET
#  define CONF_WITH_RESET 0
# endif

/*
 * CONF_VRAM_ADDRESS sets to the address where the video ram should be
 */

#ifndef CONF_VRAM_ADDRESS
# define CONF_VRAM_ADDRESS 0x40000000
#endif
# ifndef CONF_WITH_VDI_16BIT
#  define CONF_WITH_VDI_16BIT 0
# endif

# ifndef USE_STOP_INSN_TO_FREE_HOST_CPU
#  define USE_STOP_INSN_TO_FREE_HOST_CPU 0
# endif
# ifndef DETECT_NATIVE_FEATURES
#  define DETECT_NATIVE_FEATURES 0
# endif

# ifndef CONF_WITH_IKBD_SC26C94
#  define CONF_WITH_IKBD_SC26C94 1
# endif
# ifndef CONF_SERIAL_CONSOLE_POLLING_MODE
#  define CONF_SERIAL_CONSOLE_POLLING_MODE 1
# endif
# ifndef CONF_SERIAL_CONSOLE
#  define CONF_SERIAL_CONSOLE 1
# endif
# ifndef CONF_SERIAL_CONSOLE_ANSI
#  define CONF_SERIAL_CONSOLE_ANSI 1
# endif

# ifndef CONF_WITH_FLEXCAN
#  define CONF_WITH_FLEXCAN 0
# endif
# ifndef CONF_DETECT_FIRST_BOOT_WITHOUT_MEMCONF
#  define CONF_DETECT_FIRST_BOOT_WITHOUT_MEMCONF 0
# endif

# ifndef AES_STACK_SIZE
#  define AES_STACK_SIZE 2048   /* in LONGs */
# endif

/* #define DEFAULT_BAUDRATE B38400 */

#endif
