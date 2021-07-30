#ifndef _FDREG_H
#define _FDREG_H

extern int ticks_to_floppy_on(unsigned int nr);
extern void floppy_on(unsigned int nr);
extern void floppy_off(unsigned int nr);
extern void floppy_select(unsigned int nr);
extern void floppy_deselect(unsigned int nr);

#define FD_STATUS   0x3f4
#define FD_DATA     0x3f5
#define FD_DOR      0x3f2
#define FD_DIR      0x3f7
#define FD_DCR      0x3f7

#define STATUS_BUSYMASK 0x0f
#define STATUS_BUSY     0x10
#define STATUS_DMA      0x20
#define STATUS_DIR      0x40
#define STATUS_READY    0X80

#define ST0_DS          0x03
#define ST0_HA          0x04
#define ST0_NR          0x08
#define ST0_ECE         0x10
#define ST0_SE          0x20
#define ST0_INTR        0xc0

#define ST1_MAM         0x01
#define ST1_WP          0x02
#define ST1_ND          0x04
#define ST1_OR          0x10
#define ST1_CRC         0x20
#define ST1_EOC         0x80

#define ST2_MAM         0x01
#define ST2_BC          0x02
#define ST2_SNS         0x04
#define ST2_SEH         0x08
#define ST2_WC          0x10
#define ST2_CRC         0x20
#define ST2_CM          0x40

#define ST3_HA          0x04
#define ST3_TZ          0x10
#define ST3_WP          0x40

#define FD_RECALIBRATE  0x07
#define FD_SEEK         0x0f
#define FD_READ         0xe6
#define FD_WRITE        0xc5
#define FD_SENSEI       0x08
#define FD_SPECIFY      0x03

#define DMA_READ        0x46
#define DMA_WRITE       0x4a

#endif