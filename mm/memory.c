#include <signal.h>

#include <asm/system.h>

//#include <neantis/sched.h>
#include <neantis/head.h>
#include <neantis/kernel.h>

void do_exit(long code);

static inline void oom(void){
    printk("depassement memoire\n\r");
    do_exit(SIGSEGV);
}

#define invalidate() \
__asm__("movl %%eax,%%cr3"::"a" (0))

#define LOW_MEM 0x100000
#define PAGING_MEMORY (15*1024*1024)
#define PAGING_PAGES (PAGING_MEMORY>>12)
#define MAP_NR(addr) (((addr)-LOW_MEM)>>12)
#define USED 100

#define CODE_SPACE(addr) ((((addr)+4095)&~4095) < \
current->start_code + current->end_code)

static long HIGH_MEMORY = 0;

#define copy_page(from,to) \
__asm__("cld ; rep ; movsl"::"S" (from), "D" (to), "c" (1024))

static unsigned char mem_map [ PAGING_PAGES ] = {0,};

unsigned long get_free_page(void){
    register unsigned long __res asm("ax");
    
    __asm__("std ; repne ; scasb\n\t" // iter mem_map (dec)
        "jne 1f\n\t" // Aucun espace dispo
        "movb $1,1(%%edi)\n\t" // Alloue (je crois)
        "sall $12,%%ecx\n\t" // Obtient l'adresse 
        "addl %2,%%ecx\n\t" // offset ?
        "movl %%ecx,%%edx\n\t"  
        "movl $1024,%%ecx\n\t"
        "leal 4092(%%edx),%%edi\n\t"
        "rep ; stosl\n\t"
        " movl %%edx,%%eax\n"
        "1: cld"
        :"=a" (__res)
        :"0" (0),"i" (LOW_MEM), "c" (PAGING_PAGES),
        "D" (mem_map+PAGING_PAGES-1)
        );
    return __res;
}

void free_page(unsigned long addr){
    if(addr < LOW_MEM) return;
    if(addr >= HIGH_MEMORY) // trop haut
        panic("tentative de liberation: page inexistante");
    addr -= LOW_MEM;
    addr >>= 12; // id de la page
    if(mem_map[addr]--) // def inutilise
        return;
    mem_map[addr]=0; // force à 0 (anormal)
    panic("tentative de liberation: deja libre");
}
/*
int free_page_tables(unsigned long from, unsigned long size){
    unsigned long *pg_table;
    unsigned long *dir, nr;

    if(from & 0x3FFFFF)
        panic("free_page_tables appele avec un mauvais alignement");
    if(!from)
        panic("tentative de liberer l'espace memoire swap");
    //TODO
}

unsigned long put_page(unsigned long page, unsigned long address){
    unsigned long tmp, *page_table;

    if(page < LOW_MEM || page >= HIGH_MEMORY){
        printk("Tentative de mettre la page %p dans %p", page, address);
    }
    if(mem_map[(page-LOW_MEM)>>12] != 1){
        printk("mem_map est en désaccord avec %p dans %p", page, address);
    }
}
*/

void mem_init(long start_mem, long end_mem){
    int i;
    long size;

    HIGH_MEMORY = end_mem;
    for(i = 0; i < PAGING_PAGES ; i++){
        mem_map[i] = USED;
    }
    i = MAP_NR(start_mem);
    size = end_mem - start_mem;
    size >>= 12;
    while(size-- > 0){
        mem_map[i++] = 0;
    }
}
