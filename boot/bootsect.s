.code16

.equ SYSSIZE, 0x3000

.global _start, begtext, begdata, begbss, endtext,enddata,endbss
.text
begtext:
.data
begdata:
.bss
begbss:
.text

.equ SETUPLEN, 4 # number of sectors
.equ BOOTSEG, 0x07c0
.equ INITSEG, 0x9000
.equ SETUPSEG, 0x9020
.equ SYSSEG, 0x1000
.equ ENDSEG, SYSSEG + SYSSIZE

.equ ROOT_DEV, 0x301

ljmp $BOOTSEG, $_start
_start:

        mov $BOOTSEG, %ax
        mov %ax, %ds
        mov $INITSEG, %ax
        mov %ax, %es
        mov $256, %cx
        sub %si, %si
        sub %di, %di
        rep
        movsw
        ljmp $INITSEG, $go
go:
        mov %cs, %ax
        mov %ax, %ds
        mov %ax, %es
        mov %ax, %ss 
        mov $0xFF00 , %sp 
load_setup:
        mov $0x0000, %dx
        mov $0x0002, %cx
        mov $0x0200, %bx 
        .equ AX, 0x0200+SETUPLEN
        mov $AX, %ax
        int $0x13
        jnc ok_load_setup
        mov $0x0000, %dx
        mov $0x0000, %ax
        int $0x13
        jmp load_setup
ok_load_setup:
        mov $0x00,%dl
        mov $0x0800, %ax
        int $0x13
        mov $0x00, %ch
        mov %cx, %cs:sectors+0
        mov $INITSEG, %ax
        mov %ax, %es
        mov $0x03, %ah
        xor %bh, %bh
        int $0x10
        mov $30, %cx
        mov $0x0007, %bx
        mov $msg1, %bp
        mov $0x1301, %ax
        int $0x10
        mov $SYSSEG, %ax
        mov %ax, %es
        call read_it
        call kill_motor

        mov %cs:root_dev+0, %ax
        cmp $0, %ax
        jne root_defined
        mov %cs:sectors+0,%bx
        mov $0x0208, %ax
        cmp $15, %bx
        je root_defined
        mov $0x021c, %ax
        cmp $18, %bx
        je root_defined
undef_root:
        jmp undef_root
root_defined:
        mov %ax, %cs:root_dev+0
        ljmp $SETUPSEG, $0
sread:  .word 1 + SETUPLEN
head:   .word 0
track:  .word 0

read_it:
        mov %es, %ax
        test $0x0fff, %ax
die:   
        jne die
        xor %bx, %bx
rp_read:
        mov %es, %ax
        cmp $ENDSEG, %ax
        jb ok1_read
        ret 
ok1_read:
        mov %cs:sectors+0, %ax
        sub sread, %ax
        mov %ax, %cx
        shl $9, %cx
        add %bx, %cx
        jnc ok2_read
        je ok2_read
        xor %ax, %ax
        sub %bx, %ax
        shr $9, %ax
ok2_read:
        call read_track
        mov %ax, %cx
        add sread, %ax
        cmp %cs:sectors+0, %ax
        jne ok3_read
        mov $1, %ax
        sub head, %ax
        jne ok4_read
        incw track
ok4_read:
        mov %ax, head
        xor %ax, %ax
ok3_read:
        mov %ax, sread
        shl $9, %cx
        add %cx, %bx
        jnc rp_read
        mov %es, %ax
        add $0x1000, %ax
        mov %ax, %es
        xor %bx, %bx
        jmp rp_read
read_track:
        push %ax
        push %bx
        push %cx
        push %dx
        mov track, %dx
        mov sread, %cx
        inc %cx
        mov %dl, %ch
        mov head, %dx
        mov %dl, %dh
        mov $0, %dl
        and $0x0100, %dx
        mov $2, %ah
        int $0x13
        jc bad_rt
        pop %dx
        pop %cx
        pop %bx
        pop %ax
        ret
bad_rt:
        mov $0, %ax
        mov $0, %dx
        int $0x13
        pop %dx
        pop %cx
        pop %bx
        pop %ax
        jmp read_track
kill_motor:
        push %dx
        mov $0x3f2, %dx
        mov $0, %al
        outsb
        pop %dx
        ret
sectors:
        .word 0
msg1:
        .byte 13,10
        .ascii "Bootage de Neantis..."
        .byte 13,10,13,10
        .org 508
root_dev:
        .word ROOT_DEV
boot_flag:
        .word 0xAA55
        .text
        endtext:
        .data
        enddata:
        .bss
        endbss: