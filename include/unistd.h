#ifndef _UNISTD_H
#define _UNISTD_H

#define __NR_setup  0
#define __NR_fork   2
#define __NR_pause  29
#define __NR_sync   36

#define _syscall0(type, name) \
    type name(void) \
{ \
    long __res; \
    __asm__ volatile ("int $0x80" \
        :"=a" (__res) \
        : "0" (__NR_##name)); \
    if (__res >= 0) \
        return (type) __res; \
    errno = -__res; \
    return -1; \
}

#define _syscall1(type,name,atype,a) \
    type name(atype a) \
{ \
    long __res; \
    __asm__ volatile ("int $0x80" \
        : "=a" (__res) \
        : "0" (__NR_##name), "b" ((long)(a))); \
    if(__res >= 0) \
        return (type) __res; \
    errno = -__res; \
    return -1; \
}

extern int errno;


static int fork(void);
static int pause(void);
static int sync(void);

#endif