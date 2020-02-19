.code16

.equ INITSEG, 0x9000
.equ SYSSEG, 0x1000
.equ SETUPSEG, 0x9020

.global _start, begtext, begdata, begbss, endtext, enddata, endbss
.text
begtext:
.data
begdata:
.bss
begbss:

.text
ljmp $SETUPSEG, $_start

_start:
    mov %cs, %ax
    mov %ax, %ds
    mov %ax, %es 

    mov $0x03, %ah
    xor %bh, %bh
    int $0x10

    mov $29, %cx
    mov $0x000b, %bx 
    mov $msg2, %bp
    mov $0x1301, %ax
    int $0x10

    # cursor position
    mov $INITSEG, %ax
    mov %ax, %ds
    mov $0x03, %ah
    xor %bh, %bh
    int $0x10
    mov %dx, %ds:0

    #get mm size (in kB)
    mov $0x88, %ah
    int $0x15
    mov %ax,%ds:2

    #video card
    mov $0x0f, %ah
    int $0x10
    mov %bx, %ds:4
    mov %ax, %ds:6

    #EGA/VGA params
    mov $0x12, %ah
    mov $0x10, %bl
    int $0x10
    mov %ax, %ds:8
    mov %bx, %ds:10
    mov %cx, %ds:12

    #hd0 data
    mov $0x0000, %ax
    mov %ax, %ds
    lds %ds:4*0x41, %si
    mov $INITSEG, %ax
    mov %ax, %es 
    mov $0x0080, %di
    mov $0x10, %cx
    rep
    movsb

    #hd1 data
    mov $0x0000, %ax
    mov %ax, %ds
    lds %ds:4*0x46, %si
    mov $INITSEG, %ax
    mov $0x0090, %di
    mov $0x10, %cx
    rep
    movsb

    #alter ds
    mov $INITSEG, %ax
    mov %ax, %ds
    mov $SETUPSEG, %ax
    mov %ax, %es

    #print cursor position
    mov $0x03, %ah
    xor %bh, %bh
    int $0x10
    mov $11, %cx
    mov $0x000c, %bx
    mov $cur, %bp
    mov $0x1301, %ax
    int $0x10

    #print detail
    mov %ds:0, %ax
    call print_hex
    call print_nl

    #print mm size
    mov $0x13, %ah
    xor %bh, %bh
    int $0x10
    mov $12, %cx
    mov $0x000a, %bx
    mov $mem, %bp
    mov $0x1301, %ax
    int $0x10

    #print detail
    mov %ds:2, %ax
    call print_hex

    
    mov $0x01500, %ax
    mov $0x81, %dl 
    int $0x13
    jc no_disk1
    cmp $3, %ah
    je is_disk1


no_disk1:
    mov $INITSEG, %ax
    mov %ax, %es
    mov $0x0090, %di
    mov $0x10, %cx
    mov $0x00, %ax
    rep
    stosb
is_disk1:
    mov $0x0000, %ax
    cld
do_move:
    mov %ax, %es
    add $0x1000, %ax
    cmp $0x9000, %ax
    jz end_move
    mov %ax, %ds
    sub %di, %di
    sub %si, %si
    mov $0x8000, %cx
    rep
    movsw
    jmp do_move
end_move:
    mov $SETUPSEG, %ax
    mov %ax, %ds
    lidt idt_48
    lgdt gdt_48

    inb $0x92, %al 
    orb $0b00000010, %al
    outb %al, $0x92

    mov $0x11, %al

    out %al, $0x20
    .word 0x00eb, 0x00eb
    out %al, $0xa0
    .word 0x00eb, 0x00eb
    mov $0x20, %al
    out %al, $0x21
    .word 0x00eb, 0x00eb
    mov $0x28, %al
    out %al, $0xa1
    .word 0x00eb, 0x00eb
    mov $0x04, %al
    out %al, $0x21
    .word 0x00eb, 0x00eb
    mov $0x02, %al
    out %al, $0xa1
    .word 0x00eb, 0x00eb
    mov $0x01, %al
    out %al, $0x21
    .word 0x00eb, 0x00eb
    out %al, $0xa1
    .word 0x00eb, 0x00eb
    mov $0xff, %al 
    out %al, $0x21
    .word 0x00eb, 0x00eb
    out %al, $0xa1

    mov %cr0, %eax
    bts $0, %eax
    mov %eax, %cr0

    .equ sel_cs0, 0x0008
    ljmp $sel_cs0, $0

empty_8042:
    .word 0x00eb, 0x00eb
    in $0x64, %al 
    test $2, %al
    jnz empty_8042
    ret
gdt:
    .word 0,0,0,0

    .word 0x07ff
    .word 0x0000
    .word 0x9a00
    .word 0x00c0

    .word 0x07FF
    .word 0x0000
    .word 0x9200
    .word 0x00c0
idt_48:
    .word 0
    .word 0,0
gdt_48:
    .word 0x800
    .word 512+gdt, 0x9
print_hex:
    mov $4, %cx
    mov %ax, %dx
print_digit:
    rol $4, %dx
    mov $0xe0f, %ax
    and %dl, %al
    add $0x30, %al
    cmp $0x3a, %al
    jl outp
    add $0x07, %al
outp:
    int $0x10
    loop print_digit
    ret
print_nl:
    mov $0xe0d, %ax
    int $0x10
    mov $0xa, %al
    int $0x10
    ret
msg2:
    .byte 13, 10
    .ascii "Setup..."
    .byte 13,10,13,10
cur:
    .ascii "Cursor POS:"
mem:
    .ascii "MM size:"
cyl:
    .ascii "KB"
    .byte 13,10,13,10
    .ascii "HD Info"
    .byte 13,10
    .ascii "Cylinders:"
head:
    .ascii "Headers:"
sect:
    .ascii "Sectors:"
.text
endtext:
.data
enddata:
.bss
endbss:
