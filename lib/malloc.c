#include <neantis/kernel.h>
#include <neantis/mm.h>
#include <asm/system.h>

struct bucket_desc{ // 16 b
    void *page;
    struct bucket_desc *next;
    void *freeptr;
    unsigned short refcnt;
    unsigned short bucket_size;
};

struct _bucket_dir{ // 8 b
    int size;
    struct bucket_desc *chain;
};

struct _bucket_dir bucket_dir[] = {
    {   16,     (struct bucket_desc *) 0},
    {   32,     (struct bucket_desc *) 0},
    {   64,     (struct bucket_desc *) 0},
    {   128,    (struct bucket_desc *) 0},
    {   256,    (struct bucket_desc *) 0},
    {   512,    (struct bucket_desc *) 0},
    {   1024,   (struct bucket_desc *) 0},
    {   2048,   (struct bucket_desc *) 0},
    {   4096,   (struct bucket_desc *) 0},
    {   0,      (struct bucket_desc *) 0}
};

struct bucket_desc *free_bucket_desc = (struct bucket_desc *) 0;

static inline void init_bucket_desc(){
    struct bucket_desc *bdesc, *first;
    int i;

    first = bdesc = (struct bucket_desc *) get_free_page();
    if(!bdesc){
        panic("Plus de mémoire disponible init_bucket_desc()");
    }
    for(i = PAGE_SIZE/sizeof(struct bucket_desc);i > 1; i--){
        bdesc->next = bdesc + 1;
        bdesc++;
    }
    bdesc->next = free_bucket_desc;
    free_bucket_desc = first;
}
// max 4096
void *malloc(unsigned int len){
    struct _bucket_dir *bdir;
    struct bucket_desc *bdesc;
    void *retval;

    for(bdir = bucket_dir; bdir->size; bdir++){
        if(bdir->size >= len){
            break;
        }
    }
    if(!bdir->size){
        printk("malloc appele avec une taille impossible a alloue (%d)\n",len );
        panic("malloc: arg incorrect");
    }

    cli();
    for(bdesc = bdir->chain; bdesc; bdesc = bdesc->next){
        if(bdesc->freeptr){
            break;
        }
    }

    if(!bdesc){
        char *cp;
        int i;

        if(!free_bucket_desc){
            init_bucket_desc();
        }
        bdesc = free_bucket_desc;
        free_bucket_desc = bdesc->next;
        bdesc->refcnt = 0;
        bdesc->bucket_size = bdir->size;
        bdesc->page = bdesc->freeptr = (void*) (cp = (char*) get_free_page());
        if(!cp){
            panic("Plus de mémoire disponible malloc()");
        }
        for(i = PAGE_SIZE/bdir->size; i > 1; i--){
            *((char**) cp) = cp + bdir->size;
            cp += bdir->size;
        }
        *((char**) cp) = 0;
        bdesc->next = bdir->chain;
        bdir->chain = bdesc;
    }
    retval = (void *) bdesc->freeptr;
    bdesc->freeptr = *((void**) retval);
    bdesc->refcnt++;
    sti();
    return(retval);

}

// on donne la taille, faudra voir pour l'enlever
void free_s(void *obj, int size){
    void *page;
    struct _bucket_dir *bdir;
    struct bucket_desc *bdesc, *prev;

    bdesc = prev = 0;

    page = (void*) ((unsigned long) obj & 0xFFFFF000);
    for(bdir = bucket_dir; bdir->size; bdir++){
        prev = 0;
        if(bdir->size < size){
            continue;
        }
        for(bdesc = bdir->chain ; bdesc; bdesc = bdesc->next){
            if(bdesc->page == page){
                goto found; // to change >_<
            }
            prev = bdesc;
        }
    }
    panic("Adresse donnée incorrecte free_s()");
found: //(╬ Ò﹏Ó)
    cli();
    *((void**)obj) = bdesc->freeptr;
    bdesc->freeptr = obj;
    bdesc->refcnt--;
    if(bdesc->refcnt == 0){
        if((prev && (prev->next != bdesc)) || 
            (!prev && (bdir->chain != bdesc))){
            for(prev = bdir->chain; prev; prev = prev->next){
                if(prev->next == bdesc){
                    break;
                }
            }
        }
        if(prev){
            prev->next = bdesc->next;
        }
        else{
            if(bdir->chain != bdesc){
                panic("corruption des allocations");
            }
            bdir->chain = bdesc->next;
        }
        free_page((unsigned long) bdesc->page));
        bdesc->next = free_bucket_desc;
        free_bucket_desc = bdesc;
    }
    sti();
    return;
}