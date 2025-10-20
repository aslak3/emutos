//
//------------------------------------------------------------
//                                  ___ ___ _
//  ___ ___ ___ ___ ___       _____|  _| . | |_
// |  _| . |_ -|  _| . |     |     | . | . | '_|
// |_| |___|___|___|___|_____|_|_|_|___|___|_,_|
//                     |_____|
// ------------------------------------------------------------
// Copyright (c) 2022 Xark
// MIT License
//
// Asm implementation based on C code by Ross Bamford
//
// Converted to AS MRI syntax for EmuTOS by Rob Gowin.
// ------------------------------------------------------------
//



// Default rosco 2.x SPI pins
#define SPI_CS_B        2       // Output port pin
#define SPI_SCK_B       4       // Output port pin
#define SPI_COPI_B      6       // Output port pin
#define SPI_CIPO_B      2       // Input port pin
#define SPI_CS1_B       7       // Output port pin

#define RED_LED_B       3       // Output port pin

// calculate bit values for bit positions
#define SPI_CS          (1<<SPI_CS_B)
#define SPI_CS1         (1<<SPI_CS1_B)
#define SPI_SCK         (1<<SPI_SCK_B)
#define SPI_COPI        (1<<SPI_COPI_B)
#define SPI_CIPO        (1<<SPI_CIPO_B)
#define RED_LED         (1<<RED_LED_B)

#define R_INPUTPORT     0x1a           // R register 13
#define W_OPR_SETCMD    0x1c           // W register 14
#define W_OPR_RESETCMD  0x1e           // W register 15

// NOTE: Since DUART GPIO is active-low, use register aliases for DUART ports to be less confusing
#define DUART_BASE_MBR2 0xF00001
#define DUART_INPUT     DUART_BASE_MBR2+R_INPUTPORT     // R_INPUTPORT      = GPIO input address
#define DUART_OUT_LO    DUART_BASE_MBR2+W_OPR_SETCMD    // W_OPR_SETCMD     = GPIO output LO address
#define OUT_LO_OFFSET   W_OPR_SETCMD-R_INPUTPORT        // W_OPR_SETCMD     = GPIO output LO (offset from input addr) 
#define OUT_HI_OFFSET   W_OPR_RESETCMD-W_OPR_SETCMD     // W_OPR_RESETCMD   = GPIO output HI (offset from LO addr) 

                .globl _spi_clock_ident
                .globl _spi_clock_sd
                .globl _spi_clock_mmc
                .globl _spi_cs_assert
                .globl _spi_cs_unassert
                .globl _spi_initialise
                .globl _spi_recv_byte
                .globl _spi_send_byte
#if 0
                .globl _spi_send_buffer
                .globl _spi_read_buffer
#endif

                .text

_spi_initialise:
                jmp  _spi_cs_unassert

_spi_clock_ident:
		rts

_spi_clock_sd:
		rts

_spi_clock_mmc:
		rts

_spi_cs_assert:
               move.l  #DUART_OUT_LO,a0
               move.b  #SPI_CS,(a0) 
               rts

// Output five volts (inactive high) on SPI_CS
_spi_cs_unassert:
               move.l  #DUART_OUT_LO,a0
               lea.l   OUT_HI_OFFSET(a0),a1
               move.b  #SPI_CS,(a1) 
               rts

// send byte to SPI via DUART GPIO
// void spi_send_byte(int byte) - C callable
_spi_send_byte:
                move.b  5(sp),d0                //   12  d0 = send byte
                movem.l d2-d5,-(a7)             //12+32  save regs
                move.l  #DUART_OUT_LO,a0        //   12  a0 = output LO
                lea.l   OUT_HI_OFFSET(a0),a1    //    8  a1 = output HI
                moveq.l #SPI_SCK,d1             //    4  d1 = SCK bit mask
                moveq.l #SPI_COPI,d2            //    4  d2 = COPI bit mask
                moveq.l #(SPI_SCK+SPI_COPI),d3  //    4  d3 = SCK|COPI bit mask
                                                //       d4 = temp (COPI LO)
                                                //       d5 = temp (COPI HI)

                .rept    8
// send bits 7...0
                add.b   d0,d0                   //    4  shift MSB to carry
                scs     d5                      //  4/6  temp set to 0 or 0xff based on carry
                and.b   d2,d5                   //    4  isolate COPI HI bit to output
                move.b  d5,d4                   //    4  copy COPI HI bit
                eor.b   d3,d4                   //    4  set SCK LO and invert COPI for LO bit 
                move.b  d4,(a0)                 //    8  output SCK LO and COPI LO (if send bit LO)
                move.b  d5,(a1)                 //    8  output COPI HI (if send bit HI)
                move.b  d1,(a1)                 //    8  output SCK HI

                .endr

                movem.l (a7)+,d2-d5             //12+32  restore regs
spi_sb_rts:     rts

#if 0
// send count bytes from buffer to SPI via DUART GPIO
// void spi_send_buffer(void* data, int count) - C callable
_spi_send_buffer:
                move.l  8(sp),d0                //   16  d0 = byte count  
                ble.s   spi_sb_rts              // 8/10  done if <= 0

                move.l  4(sp),a0                //   16  a0 = data buffer
                movem.l d2-d6/a2,-(a7)          //12+48  save regs
                move.l  #DUART_OUT_LO,a1        //   12  a1 = output LO
                lea.l   OUT_HI_OFFSET(a1),a2    //    8  a2 = output HI
                moveq.l #SPI_SCK,d2             //    4  d2 = SCK bit mask
                moveq.l #SPI_COPI,d3            //    4  d3 = COPI bit mask
                moveq.l #(SPI_SCK|SPI_COPI),d4  //    4  d4 = SCK|COPI bit mask
                                                //       d5 = temp COPI LO
                                                //       d6 = temp COPI HI
                move.b  #RED_LED,(a1)           //   12  RED LED on (active LO)

spi_sb_loop:    move.b  (a0)+,d1                //    8  load send byte

                .rept   8
// send bits 7...0
                add.b   d1,d1                   //    4  shift MSB to carry
                scs     d6                      //  4/6  temp set to 0 or 0xff based on carry
                and.b   d3,d6                   //    4  isolate COPI HI bit to output
                move.b  d6,d5                   //    4  copy COPI HI bit
                eor.b   d4,d5                   //    4  set SCK LO and invert COPI for LO bit 
                move.b  d5,(a1)                 //    8  output SCK LO and COPI LO (if send bit LO)
                move.b  d6,(a2)                 //    8  output COPI HI (if send bit HI)
                move.b  d2,(a2)                 //    8  output SCK HI

                .endr

                subq.l  #1,d0
                bne     spi_sb_loop  

                move.b  #RED_LED,(a2)           //   12  RED LED off (active LO)
                movem.l (a7)+,d2-d6/a2          //12+48  restore regs
                rts
#endif
// read byte from DUART GPIO SPI
// int spi_recv_byte(void) - C callable
_spi_recv_byte:
                movem.l d2-d3/a2,-(a7)          //12+24  save regs
                move.l  #DUART_INPUT,a0         //    12 a0 = input
                lea.l   OUT_LO_OFFSET(a0),a1    //    8  a1 = output LO
                lea.l   OUT_HI_OFFSET(a1),a2    //    8  a2 = output HI
                moveq.l #SPI_SCK,d1             //    4  d1 = SCK bit mask
                moveq.l #SPI_CIPO_B,d2          //    4  d2 = CIPO bit number
                                                //       d3 = temp

// read bit 7
                moveq.l #0,d0                   //    4  clear read byte
                move.b  d1,(a1)                 //    8  output SCK LO
                btst.b  d2,(a0)                 //    8  test input CIPO bit
                sne     d3                      //  4/6  temp set to 0 or 0xff based on CIPO bit
                sub.b   d3,d0                   //    4  set low bit of read byte by subtracting 0 or -1
                move.b  d1,(a2)                 //    8  output SCK HI

                .rept    7
// read bits 6...0
                add.b   d0,d0                   //    4  shift read byte left
                move.b  d1,(a1)                 //    8  output SCK LO
                btst.b  d2,(a0)                 //    8  test input CIPO bit
                sne     d3                      //  4/6  temp set to 0 or 0xff based on CIPO bit
                sub.b   d3,d0                   //    4  set low bit of read byte by subtracting 0 or -1
                move.b  d1,(a2)                 //    8  output SCK HI
                .endr
                movem.l (a7)+,d2-d3/a2          //12+24  restore regs
                                                //       d0 = result read byte
spi_rb_rts:     rts

#if 0
// read count bytes into buffer from DUART GPIO SPI
// void spi_recv_buffer(void* data, int count) - C callable
_spi_recv_buffer:
                move.l  8(sp),d0                //   12  load count  
                ble.s   spi_rb_rts              // 8/10  rts if <= 0

                move.l  4(sp),a0                //   12  load data buffer
                movem.l d2-d4/a2-a3,-(a7)       //12+40  save regs
                move.l  #DUART_INPUT,a1         //    12 a1 = input
                lea.l   OUT_LO_OFFSET(a1),a2    //    8  a2 = output LO
                lea.l   OUT_HI_OFFSET(a2),a3    //    8  a3 = output HI
                moveq.l #SPI_SCK,d1             //    4  d1 = SCK bit mask
                moveq.l #SPI_CIPO_B,d2          //    4  d2 = CIPO bit num
                                                //       d3 = temp bit
                                                //       d4 = temp byte
                move.b  #RED_LED,(a1)           //   12  RED LED on (active LO)
spi_rb_loop:
                .rept    8
// read bits 7...0
                add.b   d4,d4                   //    4  shift read byte left
                move.b  d1,(a2)                 //    8  set SCK LO
                btst.b  d2,(a1)                 //    8  read input CIPO bit
                sne     d3                      //  4/6  d3 = 0 or -1
                sub.b   d3,d4                   //    4  set low bit of read byte
                move.b  d1,(a3)                 //    8  set SCK HI
                .endr

                move.b  d4,(a0)+                //    8  save read byte
                subq.l  #1,d0                   //    8  decrement count
                bne.s   spi_rb_loop             // 8/10  loop if not zero

                move.b  #RED_LED,(a2)           //   12  RED LED off (active LO)
                movem.l (a7)+,d2-d4/a2-a3       //12+40  restore regs
                rts
#endif
