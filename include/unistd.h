#ifndef _UNISTD_H
#define _UNISTD_H

#define _POSIX_VERSION  198808L

#define _POSIX_CHOWN_RESTRICTED
#define _POSIX_VDISABLE

#define STDIN_FILENO    0
#define STDOUT_FILENO   1
#define STDERR_FILENO   2

#ifndef NULL
#define NULL    ((void *)0)
#endif

#define F_OK    0
#define X_OK    1
#define W_OK    2
#define R_OK    4

#define SEEK_SET    0
#define SEEK_CUR    1
#define SEEK_END    2

#define _SC_ARG_MAX         1
#define _SC_CHILD_MAX       2
#define _SC_CLOCKS_PER_SEC  3
#define _SC_NGROUPS_MAX     4
#define _SC_OPEN_MAX        5
#define _SC_JOB_CONTROL     6
#define _SC_SAVED_IDS       7
#define _SC_VERSION         8

#define _PC_LINK_MAX            1
#define _PC_MAX_CANON           2
#define _PC_MAX_INPUT           3
#define _PC_NAME_MAX            4
#define _PC_PATH_MAX            5
#define _PC_PIPE_BUF            6
#define _PC_NO_TRUNC            7
#define _PC_VDISABLE            8
#define _PC_CHOWN_RESTRICTED    9

#include <sys/stat.h>
#include <sys/times.h>
#include <sys/utsname.h>
#include <utime.h>

#ifdef __LIBRARY__

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

#endif

extern int errno;


static int fork(void);
static int pause(void);
static int sync(void);

#endif