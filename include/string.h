#ifndef _STRING_H_
#define _STRING_H_

#ifndef NULL
#define NULL ((void *) 0)
#endif

#ifndef _SIZE_T
#define _SIZE_T
typedef unsigned int size_t;
#endif

extern char * strerror(int errno);

extern inline char * strcpy(char * dest, const char * src){
    __asm__("cld\n"
        "1:\tlodsb\n\t"
        "stosb\n\t"
        "testb %%al,%%al\n\t"
        "jne 1b"
        ::"S" (src), "D" (dest) : "si", "di", "ax"
    );

    /*
        do{
            a = lodsb(src)
            si++
            stosb(a)
            di++
        }while(a != 0)
    */

    return dest;
}
extern inline char * strncpy(char * dest, const char *src, int count){
    __asm__("cld\n"
        "1:\tdecl %2\n\t"
        "js 2f\n\t"
        "lodsb\n\t"
        "stosb\n\t"
        "testb %%al, %%al\n\t"
        "jne 1b\n\t"
        "rep\n\t"
        "stosb\n"
        "2:"
        ::"S" (src), "D" (dest), "c" (count):"si","di","ax","cx"
    );

    /*
        do{
            if(--c < 0)
                break
            a = lodsb(src)
            si++
            stosb(a)
            di++
        }while(a != 0)
    */

    return dest;
}

#endif