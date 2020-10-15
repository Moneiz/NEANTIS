#include <errno.h>

#include <neantis/fs.h>
#include <neantis/kernel.h>
#include <asm/segment.h>

#define NR_BLK_DEV  ((sizeof(rd_blk))/(sizeof(rd_blk[0])))

int block_write(int dev, long * pos, char * buf, int count){

}