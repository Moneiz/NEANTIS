#include <errno.h>
#include <string.h>
#include <sys/stat.h>
#include <a.out.h>

#include <neantis/fs.h>
#include <neantis/sched.h>
#include <neantis/kernel.h>
#include <neantis/mm.h>
#include <asm/segment.h>

extern int sys_exit(int exit_code);
extern int sys_close(int fd);

#define MAX_ARG_PAGE 32

static unsigned long * create_tables(char * p,int argc, int envc){
    unsigned long *argv, *envp;
    unsigned long * sp;

    sp = (unsigned long *) (0xfffffffc & (unsigned long) p);
    sp -= envc+1;
    envp = sp;
    sp -= argc +1;
    argv = sp;
    put_fs_long((unsigned long)envp,--sp);
    put_fs_long((unsigned long)argv, --sp);
    put_fs_long((unsigned long)argc, --sp);
    while(argc-- > 0){
        put_fs_long((unsigned long)p, argv++);
        while(get_fs_byte(p++));
    }
    put_fs_long(0, argv);
    while(envc-- >0){
        put_fs_long((unsigned long)p, envp++);
        while(get_fs_byte(p++));
    }
    put_fs_long(0,envp);
    return sp;
}
static int count(char ** argv){
    int i = 0;
    char ** tmp;
    if((tmp = argv))
        while(get_fs_long((unsigned long *) (tmp++)))
            i++;
    return i;
}
static unsigned long copy_strings(int argc, char ** argv, unsigned long *page, unsigned long p, int from_kmem){
    char *tmp, *pag=NULL;
    int len, offset = 0;
    unsigned long old_fs, new_fs;

    if(!p)
        return 0;
    
    new_fs = get_ds();
    old_fs = get_fs();
    if(from_kmem==2){
        set_fs(new_fs);
    }
    while(argc-- > 0){
        if(from_kmem == 1){
            set_fs(new_fs);
        }
        if(!(tmp = (char*)get_fs_long(((unsigned long*)argv)+argc))){
            panic("argc est incorrect");
        }
        if(from_kmem == 1){
            set_fs(old_fs);
        }
        len = 0;
        do {
            len++;
        } while(get_fs_byte(tmp++));
        if(p-len < 0){
            set_fs(old_fs);
            return 0;
        }
        while(len){
            --p; --tmp; --len;
            if(--offset < 0){
                offset = p % PAGE_SIZE;
                if(from_kmem==2){
                    set_fs(old_fs);
                }
                if(!(pag = (char*)page[p/PAGE_SIZE]) && 
                !(pag = (char*) (page[p/PAGE_SIZE] = get_free_page())))
                    return 0;
                if(from_kmem==2)
                    set_fs(new_fs);
            }
            *(pag + offset) = get_fs_byte(tmp);
        }
    }
    if(from_kmem == 2){
        set_fs(old_fs);
    }
    return p;
}
static unsigned long change_ldt(unsigned long text_size, unsigned long * page){
    unsigned long code_limit, data_limit, code_base, data_base;
    int i;

    code_limit = text_size+PAGE_SIZE -1;
    code_limit &= 0xfffff000;
    data_limit = 0x4000000;
    code_base = get_base(current->ldt[1]);
    data_base = code_base;
    set_base(current->ldt[1], code_base);
    set_limit(current->ldt[1], code_limit);
    set_base(current->ldt[2], data_base);
    set_limit(current->ldt[2],data_limit);

    __asm__("pushl $0x17\n\tpop %%fs"::);
    data_base += data_limit;
    for(i= MAX_ARG_PAGE-1; i >= 0; i--){
        data_base -= PAGE_SIZE;
        if(page[i])
            put_page(page[i], data_base);
    }
    return data_limit;
}
int do_execve(unsigned long *eip, long tmp, char * filename, char ** argv, char ** envp){
    struct m_inode * inode;
    struct buffer_head * bh;
    struct exec ex;
    unsigned long page[MAX_ARG_PAGE];
    int i, argc, envc;
    int e_uid, e_gid;
    int retval;
    int sh_bang = 0;
    unsigned long p = PAGE_SIZE*MAX_ARG_PAGE-4;

    if((0xffff & eip[1]) != 0x000f){
        panic("execve appelé depuis le mode superviseur");
    }
    for(i = 0; i < MAX_ARG_PAGE; i++){
        page[i] = 0;
    }
    if(!(inode=namei(filename))){
        return -ENOENT;
    }
    argc = count(argv);
    envc = count(envp);

restart_interp:

    if(!S_ISREG(inode->i_mode)){
        retval = -EACCES;
        goto exec_error2;
    }
    i = inode->i_mode;
    e_uid = (i & S_ISUID) ? inode->i_uid : current->euid;
    e_gid = (i & S_ISGID) ? inode->i_gid : current->egid;

    if(current->euid == inode->i_gid){
        i >>= 6;
    }
    else if(current->egid == inode->i_gid){
        i >>= 3;
    }
    if(!(i & 1) && 
        !((inode->i_mode & 0111) && suser())){
            retval = -ENOEXEC;
            goto exec_error2;
    }
    if(!(bh = bread(inode->i_dev, inode->i_zone[0]))){
        retval = -EACCES;
        goto exec_error2;
    }
    ex = *((struct exec *) bh->b_data);
    if((bh->b_data[0] == '#') && (bh->b_data[1] == '!') && (!sh_bang)){
        char buf[1023], *cp, *interp, *i_name, *i_arg;
        unsigned long old_fs;

        strncpy(buf, bh->b_data+2,1022);
        brelse(bh);
        iput(inode);
        buf[1022] = '\0';
        if((cp = strchr(buf, '\n'))){
            *cp = '\0';
            for(cp = buf; (*cp == ' ') || (*cp == '\t'); cp++);
        }
        if(!cp || *cp == '\0'){
            retval = -ENOEXEC;
            goto exec_error1;
        }
        interp = i_name = cp;
        i_arg = 0;
        for(; *cp && (*cp != ' ') && (*cp != '\t'); cp++){
            if(*cp == '/'){
                i_name = cp + 1;
            }
        }
        if(*cp){
            *cp++ = '\0';
            i_arg = cp;
        }
        if(sh_bang++ == 0){
            p = copy_strings(envc, envp, page, p, 0);
            p = copy_strings(--argc, argv+1, page,p,0);
        }
        p = copy_strings(1, &filename, page, p, 1);
        argc++;
        if(i_arg){
            p = copy_strings(1, &i_arg, page,p,2);
            argc++;
        }
        p = copy_strings(1, &i_name, page, p, 2);
        argc++;
        if(!p){
            retval = -ENOMEM;
            goto exec_error1;
        }

        old_fs = get_fs();
        set_fs(get_ds());
        if(!(inode = namei(interp))){
            set_fs(old_fs);
            retval = -ENOENT;
            goto exec_error1;
        }

        set_fs(old_fs);
        goto restart_interp;
        
    }

    brelse(bh);
    if(N_MAGIC(ex) != ZMAGIC || ex.a_trsize || ex.a_drsize ||
    ex.a_text+ex.a_data+ex.a_bss > 0x3000000 ||
    inode->i_size < ex.a_text+ex.a_data+ex.a_syms+N_TXTOFF(ex)){
        retval = -ENOEXEC;
        goto exec_error2;   
    }

    if(N_TXTOFF(ex) != BLOCK_SIZE){
        printk("%s: N_TXTOFF != BLOCK_SIZE. Vérifiez a.out.h.", filename);
        retval = -ENOEXEC;
        goto exec_error2;
    }

    if(!sh_bang){
        p = copy_strings(envc, envp, page, p,0);
        p = copy_strings(argc, argv, page, p,0);
        if(!p){
            retval = -ENOMEM;
            goto exec_error2;
        }
    }

    if(current->executable)
        iput(current->executable);
    current->executable = inode;
    for(i = 0; i < 32; i++){
        current->sigaction[i].sa_handler = NULL;
    }
    for(i = 0; i <NR_OPEN; i++){
        if((current->close_on_exec>>i)&1){
            sys_close(i);
        }
    }
    current->close_on_exec = 0;
    free_page_tables(get_base(current->ldt[1]), get_limit(0x0f));
    free_page_tables(get_base(current->ldt[2]), get_limit(0x17));
    if(last_task_used_math == current){
        last_task_used_math = NULL;
    }
    current->used_math = 0;
    p += change_ldt(ex.a_text, page) - MAX_ARG_PAGE*PAGE_SIZE;
    p = (unsigned long) create_tables((char*)p, argc, envc);
    current->brk = ex.a_bss + 
        (current->end_data = ex.a_data + 
        (current->end_code = ex.a_text));
    current->start_stack = p & 0xfffff000;
    current->euid = e_uid;
    current->egid = e_gid;
    i = ex.a_text+ex.a_data;
    while(i&0xfff)
        put_fs_byte(0, (char*)(i++));
    eip[0] = ex.a_entry;
    eip[3] = p;
    return 0;

exec_error2:
    iput(inode);
exec_error1:
    for(i = 0; i w < MAX_ARG_PAGE;i++){
        free_page(page[i]);
    }
    return(retval);
}