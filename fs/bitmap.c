#include <string.h>

#include <neantis/sched.h>
#include <neantis/kernel.h>

#define clear_block(addr) \
__asm__("cld\n\t" \
    "rep\n\t" \
    "stosl" \
    ::"a" (0), "c" (BLOCK_SIZE/4),"D" ((long)(addr)) : "cx","di")

#define set_bit(nr, addr) ({ \
register int res __asm__("ax"); \
__asm__("btsl %2,%3\n\tsetb %%al":"=a" (res) : "0" (0), "r" (nr), "m" (*(addr))); \
res;})

#define clear_bit(nr, addr) ({ \
    register int res __asm__("ax"); \
    __asm__("btrl %2,%3\n\tsetnb %%al" : "=a"(res) : "0" (0), "r" (nr), "m" (*(addr))); \
    res;})

#define find_first_zero(addr) ({ \
    int __res; \
    __asm__("cld\n" \
        "1:\tlodsl\n\t" \
        "notl %%eax\n\t" \
        "bsfl %%eax,%%edx\n\t" \
        "je 2f\n\t" \
        "addl %%edx,%%ecx\n\t" \
        "jmp 3f\n" \
        "2:\taddl $32,%%ecx\n\t" \
        "cmpl $8192,%%ecx\n\t" \
        "jl 1b\n" \
        "3:" \
        :"=c" (__res):"c" (0), "S" (addr):"ax","dx","si"); \
    __res;})

void free_block(int dev, int block){
    struct super_block * sb;
    struct buffer_head * bh;

    if(!(sb = get_super(dev))){
        panic("tentative de liberer un bloc sur un appareil inexistant");
    }
    if(block < sb->s_firstdatazone || block>=sb->s_nzones){
        panic("tentative de liberer un bloc hors de la datazone");
    }
    bh = get_hash_table(dev, block);
    if(bh){
        if(bh->b_count != 1){
            printk("tentative de liberation de bloc (%04x:%d), count=%d\n",
            dev, block, bh->b_count);
            return;
        }
        bh->b_dirt = 0;
        bh->b_uptodate = 0;
        brelse(bh);
    }
    block -= sb->s_firstdatazone - 1;
    if(clear_bit(block&8191,sb->s_zmap[block/8192]->b_data)){
        printk("bloc (%04x:%d) ", dev, block+sb->s_firstdatazone-1);
        panic("free_block: bit deja corrige");
    }
    sb->s_zmap[block/8192]->b_dirt = 1;
}