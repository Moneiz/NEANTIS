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
            a = lodsb(S)
            S++
            stosb(a)
            D++
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
            a = lodsb(S)
            S++
            stosb(a)
            D++
        }while(a != 0)
    */

    return dest;
}

extern inline char * strcat(char * dest, const char * src){
    __asm__("cld\n\t"
        "repne\n\t"
        "scasb\n\t"
        "decl %1\n"
        "1:\tlodsb\n\t"
        "stosb\n\t"
        "testb %%al,%%al\n\t"
        "jne 1b"
        ::"S" (src), "D" (dest), "a" (0), "c" (0xFFFFFFFF):"si", "di", "ax", "cx"
    );
    /*
        while(c++ > 0){
            if(lodsb(D) == 0)
                break;
            D++
        }
        D--
        do{
            if(--c < 0)
                break
            a = lodsb(S)
            S++
            stosb(a)
            D++
        }while(a != 0)
    */
   return dest;
}

extern inline char * strncat(char * dest, const char * src, int count){
    __asm__("cld\n\t"
        "repne\n\t"
        "scasb\n\t"
        "decl %1\n\t"
        "movl %4,%3\n"
        "1:\tdecl %3\n\t"
        "js 2f\n\t"
        "lodsb\n\t"
        "stosb\n\t"
        "testb %%al,%%al\n\t"
        "jne 1b\n"
        "2:\txorl %2,%2\n\t" //ajoute un \0 à la fin de la chaine
        "stosb"
        ::"S" (src), "D" (dest), "a" (0), "c" (0xFFFFFFFF), "g" (count)
        : "si", "di", "ax", "cx" 
    );
    return dest;
}

extern inline int strcmp (const char * cs,const char * ct){
    register int __res __asm__("ax");
    __asm__("cld\n"
        "1:\tlodsb\n\t"
        "scasb\n\t"
        "jne 2f\n\t"
        "testb %%al, %%al\n\t"
        "jne 1b\n\t"
        "xorl %%eax, %%eax\n\t"
        "jmp 3f\n"
        "2:\tmovl $1,%%eax\n\t"
        "jl 3f\n\t"
        "negl %%eax\n"
        "3:"
        :"=a" (__res):"S" (cs), "D" (ct), 
        : "si", "di"
    );
    return __res;
}

#endif