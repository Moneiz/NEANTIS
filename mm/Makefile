include ../Makefile.header

LDFLAGS += -r
CFLAGS += -I../include
CPP += -I../include

.c.o:
	@$(CC) $(CFLAGS) \
	-c -o $*.o $<

.s.o:
	@$(AS) -o $*.o $<

.c.s:
	@$(CC) $(CFLAGS) \
	-S -o $*.s $<

OBJS = memory.o

all: mm.o 

mm.o: $(OBJS)
	@$(LD) $(LDFLAGS) -o mm.o $(OBJS)

clean:
	@rm -f core *.o *.a tmp_make
	@for i in *.c;do rm -f `basename $$i .c`.s;done

dep:
	@sed '/\#\#\# Dependencies/q' < Makefile > tmp_make
	@(for i in *.c; do $(CPP) -M $$i;done) >> tmp_make
	@cp tmp_make Makefile

memory.o: memory.c ../include/asm/system.h ../include/neantis/sched.h \
	../include/neantis/head.h ../include/neantis/mm.h ../include/neantis/kernel.h
