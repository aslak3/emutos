// registers within one port pair

#define IPCR26C94 4
#define ACR26C94 4
#define ISR26C94 5
#define IMR26C94 5
#define CTU26C94 6
#define CTL26C94 7
#define START26C94 14
#define STOP26C94 15

// globals

#define IVR26C94 41

// the base address of each port and port pair

#define BASE26C94 0x44010000

#define BASEPA26C94 BASE26C94+0
#define BASEPA BASEPA26C94
#define BASEPB26C94 BASE26C94+8
#define BASEPB BASEPB26C94
#define BASEPC26C94 BASE26C94+16
#define BASEPC BASEPC26C94
#define BASEPD26C94 BASE26C94+24
#define BASEPD BASEPD26C94

#define BASEPAB BASEPA26C94
#define BASEPCD BASEPCC2694

// regsiters within one port

 #define MRX26C94 0
 #define SR26C94 1
 #define CSR26C94 1
 #define CR26C94 2
 #define RXFIFO26C94 3
 #define TXFIFO26C94 3
