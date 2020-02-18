include ../Makefile.header

LDFLAGS += -Ttext 0

all: bootsect

bootsect: bootsect.s 
	@$(AS) -o bootsect.o bootsect.s
	@$(LD) $(LDF
		@$(OBJCOPY) -R .pdr -R .comment -R.note -S -O binary bootsect

clean:
	@rm -f bootsect bootsect.o setup setup.o head.o