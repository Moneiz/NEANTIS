OS = Mac

HDA = hdc-0.01.img 
CALLTREE=$(shell find tools/ -name "calltree" -perm 755 -type f)

BOCHS=bochs 

RAMDISK =

include Makefile.header #todo

LDFLAGS += -Ttext 0 -e startup32
CFLAGS += $(RAMDISK) -Iinclude
CPP += -Iinclude

ROOT_DEV =

ARCHIVES=kernel/kernel.o mm/mm.o fs/fs.o 
DRIVERS =kernel/blk_drv/blk_drv.a kernel/chr_drv/chr_drv.a
MATH = kernel/math/math.a
LIBS = lib/lib.a

.c.s:
	@$(CC) $(CFLAGS) -S -o $*.s $<
.s.o:
	@$(AS) -o $*.o $<
.c.o:
	@$(CC) $(CFLAGS) -c -o $*.o $<
all: image
image: boot/bootsect boot/setup tools/system
	@cp -f tools/system system.tmp
	@$(STRIP) system.tmp
	@$(OBJCOPY) -O binary -R .note -R .comment system.tmp tools/kernel
	@tools/build.sh boot/bootsect boot/setup tools/kernel Image $(ROOT_DEV)
	@rm system.tmp
	@rm -f tools/kernel
	@sync
disk: image
	@dd bs=8192 if=image of=/dev/fd0
boot/head.o: boot/head.s
	@make head.o -C boot/
tools/system: boot/head.o init/main.o \
	$(ARCHIVES) $(DRIVERS) $(MATH) $(LIBS)
	@$(LD) $(LDFLAGS) boot/head.o init/main.o \
	$(ARCHIVES) \
	$(DRIVERS) \
	$(MATH) \
	$(LIBS) \
	-o tools/system 
	@nm tools/system | grep -v '\(compiled\)\|\(\.o$$\)\|\( [aU] \)\|\(\.\.ng$$\)\|\(LASH[RL]DI\)'|sort > System.map
kernel/math/math.a:
	@make -C kernel/math
kernel/blk_drv/blk_drv.a:
	@make -C kernel/blk_drv
kernel/chr_drv/chr_drv.a:
	@make -C kernel/chr_drv
kernel/kernel.o:
	@make -C kernel
mm/mm.o:
	@make -C mm
fs/fs.o:
	@make -C fs
lib/lib.a:
	@make -C lib
boot/setup: boot/setup.s 
	@make setup -C boot
boot/bootsect: boot/bootsect.s
	@make bootsect -C boot
tmp.s: boot/bootsect.s tools/system
	@(echo -n "SYSSIZE = "(";ls -l tools/system | grep system \
		| cut -c25-31 | tr '\012' ' '; echo "+15) / 16") > tmp.s
		@cat boot/bootsect.s >> tmp.s
clean:
	@rm -f image System.map tmp_make core boot/bootsect boot/setup
	@rm -f init/*
	@for i in mm fs kernel lib boot; do make clean -C $$i; done
info:
	@make clean
	@script -q -c "make all"
	@cat typescript | col -bp | grep -E "warning|Error" > info
	@cat info
distclean: clean
	@rm -f tag cscope* neantis-0.01.* $(CALLTREE)
	@(find tools/calltree-2.3 -name "*.o" | xargs -i rm -f {})
	@make clean -C tools/calltree-2.3
	@make clean -C tools/bochs/bochs-2.3.7
backup: clean
	@(cd .. ; tar cf - linux | compress16 - > backup.Z)
	@sync 
dep:
	@sed '/\#\#\# Dependencies/q' < Makefile > tmp_make
	@(for i in init/*.c; do echo -n "init/";$(CPP) -M $$i; done) >> tmp_make
	@cp tmp_make Makefile
	@for i in fs kernel mm; do make dep -C $$i; done

tag: tags
tags:
	@cscope -R
cscope:
	@cscope -Rbkq
start:
	@qemu-system-x86_64 -m 16M -boot a -fda image -hda $(HDA_IMG)
debug:
	@echo $(OS)
	@qemu-system-x86_64 -m 16M -boot a -fda image -hda $(HDA_IMG) -s -S
bochs-debug:
	@$(BOCHS) -q -f tools/bochs/bochsrc/bochsrc-hd-dbg.bxrc
bochs:
ifeq ($(BOCHS),)
	@(cd tools/bochs/bochs-2.3.7;) \
	./configure --enable-plugin --enable-disasm --enable-gdb-stub; \
	make)
endif
bochs-clean:
	@make clean -C tools/bochs/bochs-2.3.7
calltree:
ifeq ($(CALLTREE),)
	@make -C tools/calltree-2.3 
endif
calltree-clean:
	@(find tools/calltree-2.3 -name "*.o" \
	-o -name "calltree" -type f | xargs -i rm -f {})
cg: callgraph
callgraph:
	@calltree -b -np -m init/main.c | tools/tree2dotx > neantis-0.01.dot
	@dot -Tjpg neantis-0.01.dot -o neantis-0.01.jpg

init/main.o: init/main.c