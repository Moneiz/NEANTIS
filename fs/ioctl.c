#include <string.h>
#include <errno.h>
#include <sys/stat.h>

#include <neantis/sched.h>

extern int tty_ioctl(int dev, int cmd, int arg);

typedef int (*ioctl_ptr)(int dev, int cmd, int arg);

#define NR_DEVS ((sizeof(ioctl_table)) / (sizeof(ioctl_ptr)))

static ioctl_ptr ioctl_table[]={
    NULL, 
    NULL,
    NULL,
    NULL,
    tty_ioctl,
    tty_ioctl,
    NULL,
    NULL
};

int sys_ioctl(unsigned int fd, unsigned int cmd, unsigned long arg){
    struct file * filp;
    int dev,mode;
    if(fd >= NR_OPEN || !(filp = current->filp[fd])){
        return - EBADF;
    }
    mode=filp->f_inode->i_mode;
    if(!S_ISCHR(mode) && !S_ISBLK(mode)){
        return -EINVAL;
    }
    dev = filp->f_inode->i_zone[0];
    if(MAJOR(dev)>=NR_DEVS){
        panic("appareil inconnu pour ioctl");
    }
    if(!ioctl_table[MAJOR(dev)]){
        return -ENOTTY;
    }
    return ioctl_table[MAJOR(dev)](dev, cmd, arg);
}