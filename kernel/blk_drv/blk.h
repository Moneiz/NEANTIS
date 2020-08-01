#ifndef _BLK_H
#define _BLK_H

#define NR_REQUEST 32

struct request{
    int dev;
    int cmd;
    int errors;
    unsigned long sector;
    unsigned long nr_sectors;
    char * buffer;
    struct task_struct * waiting;
    struct buffer_head * bh;
    struct request * next;
};

#endif