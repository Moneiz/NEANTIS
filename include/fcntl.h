#ifndef _FCNTL_H
#define _FCNTL_H

#include <sys/types.h>

#define O_ACCMODE       00003
#define O_RDONLY        00
#define O_WRONLY        01
#define O_RDWR          02
#define O_CREAT         00100
#define O_EXCL          00200
#define O_NOCTTY        00400
#define O_TRUNC         01000
#define O_APPEND        02000
#define O_NONBLOCK      04000
#define O_NDELAY        O_NONBLOCK

#define F_DUPFD     0
#define F_GETFD     1
#define F_SETFD     2
#define F_GETFL     3
#define F_SETFL     4
#define F_GETLK     5
#define F_SETLK     6
#define F_SETLKW    7

#define FD_CLOEXEC      1

#define F_RDLCK     0
#define F_WRLCK     1
#define F_UNLCK     2

struct flock{
    short l_type;
    short l_whence;
    off_t l_start;
    off_t l_len;
    pid_t l_pid;
};

extern int creat(const char * filename, mode_t mode);
extern int fcntl(int fildes, int cmd, ...);
extern int open(const char * filename, int flags, ...);

#endif