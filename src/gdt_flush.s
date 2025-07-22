    .globl gdt_flush
gdt_flush:
    movl 4(%esp), %eax       # Load pointer to GDT descriptor into EAX
    lgdt (%eax)              # Load the GDT using LGDT

    movw $0x10, %ax          # Data segment selector (index 2 in GDT)
    movw %ax, %ds
    movw %ax, %es
    movw %ax, %fs
    movw %ax, %gs
    movw %ax, %ss

    ljmp $0x08, $flush_point # Far jump to code segment (index 1)
flush_point:
    ret

    .globl tss_flush
tss_flush:
    movw $0x2B, %ax
    ltr %ax
    ret
