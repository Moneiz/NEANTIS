void verify_area(void * addr, int count);
void panic(const char * str);
int printf(const char * fmt, ...);
int printk(const char * fmt, ...);
int tty_write(unsigned ch, char * buf, int count);
void * malloc(unsigned int size);
void free_s(void * obj, int size);

#define free(x) free_s((x),0)

#define suser() (current->euid == 0)