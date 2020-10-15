#ifndef _HDREG_H
#define _HDREG_H

#define HARD_DISK_TYPE  17

#if HARD_DISK_TYPE == 17
#define _CYL        977
#define _HEAD       5
#define __WPCOM     300
#define _LZONE      977
#define _SECT       17
#define _CTL        0
#elif HARD_DISK_TYPE == 18
#define _CYL        977
#define _HEAD       7
#define __WPCOM     (-1)
#define _LZONE      977
#define _SECT       17
#define _CTL        0
#else
#error "No HD type defined"
#endif

#if __WPCOM >= 0
#define _WPCOM ((__WPCOM)>>2)
#else
#define _WPCOM __WPCOM
#endif

struct partition{
    unsigned char boot_ind;
    unsigned char head;
    unsigned char sector;
    unsigned char cyl;
    unsigned char sys_ind;
    unsigned char end_head;
    unsigned char end_sector;
    unsigned char end_cyl;
    unsigned int start_sect;
    unsigned int nr_sects;
};

#endif