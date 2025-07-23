.set MAGIC, 0x1badb002
.set FLAGS, 7
.set CHECKSUM, -(MAGIC + FLAGS)
.set MODE_TYPE, 0
.set WIDTH, 0
.set HEIGHT, 0
.set DEPTH, 0

.set HEADER_ADDR, 0
.set LOAD_ADDR, 0
.set LOAD_END_ADDR, 0
.set BSS_END_ADDR, 0
.set ENTRY_ADDR, 0

/**
Multiboot header layout:
0   u32 magic         required
4   u32 flags         required
8   u32 checksum      required
12  u32 header_addr   if flags[16] is set
16  u32 load_addr     if flags[16] is set
20  u32 load_end_addr if flags[16] is set
24  u32 bss_end_addr  if flags[16] is set
28  u32 entry_addr    if flags[16] is set
32  u32 mode_type     if flags[2] is set
36  u32 width         if flags[2] is set
40  u32 height        if flags[2] is set
44  u32 depth         if flags[2] is set
*/

.section .multiboot
    .long MAGIC
    .long FLAGS
    .long CHECKSUM
    .long HEADER_ADDR
    .long LOAD_ADDR
    .long LOAD_END_ADDR
    .long BSS_END_ADDR
    .long ENTRY_ADDR
    .long MODE_TYPE
    .long WIDTH
    .long HEIGHT
    .long DEPTH
    .space 4 * 13


.section .bss
    .balign 16
stack_bottom:
    .skip 16384 * 8
stack_top:

.section .data
    .balign 4096
    .globl initial_page_dir
initial_page_dir:
    .long 0x00000083        # present, rw, 4MB
    .rept 767
        .long 0
    .endr
    .long 0x00000083        # maps 0x00000000–0x00400000
    .long 0x00400083        # maps 0x00400000–0x00800000
    .long 0x00800083        # maps 0x00800000–0x00C00000
    .long 0x00C00083        # maps 0x00C00000–0x01000000
    .rept 256-4
        .long 0
    .endr

.section .boot
    .globl _start
_start:
    # Set up paging
    movl initial_page_dir - 0xC0000000, %ecx
    movl %ecx, %cr3

    # Enable PSE (Page Size Extension)
    movl %cr4, %ecx
    orl $0x10, %ecx
    movl %ecx, %cr4

    # Enable paging
    movl %cr0, %ecx
    orl $0x80000000, %ecx
    movl %ecx, %cr0

    # Jump to higher half
    jmp higher_half

.section .text
higher_half:
    movl $stack_top, %esp
    pushl %ebx
    pushl %eax
    xorl %ebp, %ebp
    .extern kernel_main
    call kernel_main

halt:
    hlt
    jmp halt