#define __LIBRARY__
#include <time.h>
#include <unistd.h>

static inline fork(void) __attribute__((always_inline));
static inline pause(void) __attribute__((always_inline));
static inline _syscall0(int, fork);
static inline _syscall0(int, pause);
static inline _syscall1(int, setup, void *, BIOS);
static inline _syscall0(int, sync);

#include <asm/system.h>
#include <asm/io.h>
#include <neantis/fs.h>
#include <neantis/tty.h>
#include <neantis/head.h>
#include <neantis/sched.h>

#include <stddef.h>
#include <stdarg.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>


static char printbuf[1024];

extern int vsprintf();
extern void init();
extern void blk_dev_init();
extern void chr_dev_init();
extern void hd_init();
extern void mem_init(long start, long end);
extern long rd_init(long mem_start, long length);
extern long kernel_mktime(struct tm * tm);
extern long startup_time;

#define EXT_MEM_K (*(unsigned short*) 0x90002)
#define DRIVE_INFO (*(struct drive_info*) 0x90080)
#define ORIG_ROOT_DEV (*(unsigned short *) 0x901FC)

//to get time
#define CMOS_READ(addr) ({ \
    outb_p(0x80|addr,0x70);\
    inb_p(0x71); \
})

#define BCD_TO_BIN(val) ((val)=(val)&0xF + ((val)>>0x4)*10)

static void time_init(void){
    struct tm time;

    do{
        time.tm_sec = CMOS_READ(0);
        time.tm_min = CMOS_READ(2);
        time.tm_hour = CMOS_READ(4);
        time.tm_mday = CMOS_READ(7);
        time.tm_mon = CMOS_READ(8);
        time.tm_year = CMOS_READ(9);
    } while(time.tm_sec != CMOS_READ(0));
    BCD_TO_BIN(time.tm_sec);
    BCD_TO_BIN(time.tm_min);
    BCD_TO_BIN(time.tm_hour);
    BCD_TO_BIN(time.tm_mday);
    BCD_TO_BIN(time.tm_mon);
    BCD_TO_BIN(time.tm_year);
    time.tm_mon--;
    startup_time = kernel_mktime(&time);
}

static long memory_end = 0;
static long buffer_memory_end = 0;
static long main_memory_start = 0;

struct drive_info {
    char dummy[32];
} drive_info;

void main(void){
    ROOT_DEV = ORIG_ROOT_DEV;
    drive_info = DRIVE_INFO;
    memory_end = (1<<20) + (EXT_MEM_K<<10);
    memory_end &= 0xFFFFF000;
    if(memory_end > 16*1024*1024){
        memory_end = 16*1024*1024;
    }
    if(memory_end > 12*1024*1024){
        buffer_memory_end = 4*1024*1024;
    }else if(memory_end > 6*1024*1024){
        buffer_memory_end = 2*1024*1024;
    }else{
        buffer_memory_end = 1*1024*1024;
    }
    main_memory_start = buffer_memory_end;

    mem_init(main_memory_start,memory_end);
    trap_init();
    blk_dev_init();

    sti();
    move_to_user_mode();
}

