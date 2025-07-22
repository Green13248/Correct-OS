bits 32
org 0xC0400000

section .text
global _start

_start:
    mov eax, 0          ; syscall number 0 (custom)
    mov ebx, msg        ; pointer to msg
    int 0x80            ; call syscall

    mov eax, 1          ; syscall exit
    mov ebx, 0
    int 0x80

    jmp $               ; hang

section .data
msg db "Hello from user program!", 0