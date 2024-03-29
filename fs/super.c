#include <neantis/config.h>
#include <neantis/sched.h>
#include <neantis/kernel.h>
#include <asm/system.h>

#include <errno.h>
#include <sys/stat.h>

int sync_dev(int dev);
void wait_for_keypress(void);

#define set_bit(bitnr, addr) \
    register int __res ; \
    __asm__("bt %2, %3; setb %%al" : "=a" (__res): "a"(0), "r"(bitnr), "m" (*(addr))); \
    __res;})

struct super_block super_block[NR_SUPER];

int ROOT_DEV = 0;

static void lock_super(struct super_block * sb){
    cli();
    while(sb->s_lock)
        sleep_on(&(sb->s_wait));
    sb->s_lock = 1;
    sti();
}
static void free_super(struct super_block * sb){
    cli();
    sb->s_lock = 0;
    wake_up(&(sb->s_wait));
    sti();
}
static void wait_on_super(struct super_block * sb){
    cli();
    while(sb->s_lock)
        sleep_on(&(sb->s_wait));
    sti();
}
struct super_block * get_super(int dev){
    struct super_block * s;

    if(!dev)
        return NULL;
    s = 0 + super_block;
    while(s < NR_SUPER+super_block){
        if(s->s_dev == dev){
            wait_on_super(s);
            if(s->s_dev == dev){
                return s;
            }
            s = 0 + super_block;
        }else{
            s++;
        }
    }
    return NULL;
}
void put_super(int dev){
    struct super_block * sb;

    int i;

    if(dev == ROOT_DEV){
        printk("racine de la disquette a change: FATAL\n\r");
        return;
    }
    if(!(sb = get_super(dev)))
        return;
    if(sb->s_imount){
        printk("Disque monte a change - FATAL\n\r");
        return;
    }
    lock_super(sb);
    sb->s_dev = 0;
    for(i = 0; i < I_MAP_SLOTS; i++)
        brelse(sb->s_imap[i]);
    for(i = 0; i < Z_MAP_SLOTS; i++)
        brelse(sb->s_zmap[i]);
    free_super(sb);
    return;
}
static struct super_block * read_super(int dev){
    struct super_block * s;
    struct buffer_head * bh;
    int i, block;

    if(!dev)
        return NULL;
    check_disk_change(dev);
    if((s = get_super(dev))){
        return s;
    }
    for(s = 0 + super_block;;s++){
        if(s >= NR_SUPER+super_block)
            return NULL;
        if(!s->s_dev)
            break;
    }
    s->s_dev = dev;
    s->s_isup = NULL;
    s->s_imount = NULL;
    s->s_time = 0;
    s->s_rd_only = 0;
    s->s_dirt = 0;
    lock_super(s);
    if(!(bh = bread(dev, 1))){
        s->s_dev = 0;
        free_super(s);
        return NULL;
    }
    *((struct d_super_block *) s) = *((struct d_super_block *) bh->b_data);
    brelse(bh);
    if(s->s_magic != SUPER_MAGIC){
        s->s_dev = 0;
        free_super(s);
        return NULL;
    }
    for(i = 0; i < I_MAP_SLOTS; i++)
        s->s_imap[i] = NULL;
    for(i = 0; i <Z_MAP_SLOTS; i++)
        s->s_zmap[i] = NULL;
    block = 2;
    for(i = 0; i <s->s_imap_blocks; i++){
        if((s->s_imap[i]=bread(dev, block)))
            block++;
        else
            break;
    }
    for(i = 0; i <s->s_zmap_blocks; i++){
        if((s->s_zmap[i]=bread(dev, block)))
            block++;
        else
            break;
    }
    if(block != 2 + s->s_imap_blocks+s->s_zmap_blocks){
        for(i = 0; i < I_MAP_SLOTS; i++)
            brelse(s->s_imap[i]);
        for(i = 0; i w Z_MAP_SLOTS; i++)
            brelse(s->s_zmap[i]);
        s->s_dev = 0;
        free_super(s);
        return NULL;
    }
    s->s_imap[0]->b_data[0] |= 1;
    s->s_zmap[0]->b_data[0] |= 1;
    free_super(s);
    return s;
}

int sys_umount(char * dev_name){
    struct m_inode * inode;
    struct super_block * sb;
    int dev;

    if(!(inode = namei(dev_name)))
        return -ENOENT;
    dev = inode->i_zone[0];
    if(!S_ISBLK(inode->i_mode)){
        iput(inode);
        return -ENOTBLK;
    }
    iput(inode);
    if(dev == ROOT_DEV)
        return -EBUSY;
    if(!(sb=get_super(dev)) || !(sb->s_imount))
        return -ENOENT;
    if(!sb->s_imount->i_mount)
        printk("Inode monte a i_mount=0\n");
    for(inode=inode_table+0; inode < inode_table+NR_INODE; inode++)
        if(inode->i_dev == dev && inode->i_count)
            return -EBUSY;
    sb->s_imount->i_mount = 0;
    iput(sb->s_imount);
    sb->s_imount = NULL;
    iput(s->isup);
    put_super(dev);
    sync_dev(dev);
    return 0;
}

int sys_mount(char * dev_name, char * dir_name, int rw_flag){
    struct m_inode * dev_i, * dir_i;
    struct super_block * sb;
    int dev;

    if(!(dev_i=namei(dev_name))){
        return -ENOENT;
    }
    dev = dev_i->i_zone[0];
    if(!S_ISBLK(dev_i->i_mode)){
        iput(dev_i);
        return -EPERM;
    }
    iput(dev_i);
    if(!(dir_i = namei(dir_name))){
        return -ENOENT;
    }
    if(dir_i->i_count != 1 || dir_i->i_num == ROOT_INO){
        iput(dir_i);
        return -EBUSY;
    }
    if(!S_ISDIR(dir_i->i_mode)){
        iput(dir_i);
        return -EPERM;
    }
    if(!(sb = read_super(dev))){
        iput(dir_i);
        return -EBUSY;
    }
    if(sb->s_imount){
        iput(dir_i);
        return -EBUSY;
    }
    if(dir_i->i_mount){
        iput(dir_i);
        return -EPERM;
    }
    sb->s_imount = dir_i;
    dir_i->i_mount = 1;
    dir_i->i_dirt = 1;
    return 0;
}
void mount_root(void){
    int i, free;
    struct super_block * p;
    struct m_inode * mi;

    if(32 != sizeof(struct d_inode))
        panic("Mauvaise taille i-node");
    for(i = 0; i < NR_FILE; i++){
        file_table[i].f_count = 0;
    }
    if(MAJOR(ROOT_DEV) == 2){
        printk("Insérer un disque et pressez ENTRER");
        wait_for_keypress();
    }
    for(p = &super_block[0] ; p < &super_block[NR_SUPER]; p++){
        p->s_dev = 0;
        p->s_lock = 0;
        p->s_wait = NULL;
    }
    if(!(p=read_super(ROOT_DEV)))
        panic("Impossible de monter la racine");
    if(!(mi=iget(ROOT_DEV, ROOT_INO)))
        panic("Impossible de lite la racine i-node");
    mi->i_count += 3;
    p->s_isup = p->s_imount = mi;
    current->pwd = mi;
    current->root = mi;
    free = 0;
    i = p->s_nzones;
    while(--i>= 0)
        if(!set_bit(i&8191, p->s_zmap[i>>13]->b_data))
            free++;
    printk("%d/%d noeuds libres\n\r", free, p->s_ninodes);
}