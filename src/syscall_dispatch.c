#include <stdint.h>
#include "include/VGA.h"
#include "include/kernel.h"

#define USER_SPACE_START 0xC0400000
#define USER_SPACE_END   0xC0800000

// Copies at most max_len bytes from user string to kernel buffer
// Returns 0 if success, -1 if invalid pointer or no null terminator found
int safe_copy_from_user(char* dest, const char* src, int max_len) {
    for (int i = 0; i < max_len; i++) {
        uint32_t addr = (uint32_t)(src + i);
        if (addr < USER_SPACE_START || addr >= USER_SPACE_END) {
            return -1; // invalid address
        }
        char c = *(char*)addr; // Read one byte from user memory safely
        dest[i] = c;
        if (c == '\0') {
            return 0; // success
        }
    }
    return -1; // no null terminator within max_len bytes
}

//syscall made, eax = what syscall
int syscall_dispatch(uint32_t* esp) {
    uint32_t edi = esp[0];
    uint32_t esi = esp[1];
    uint32_t ebp = esp[2];
    uint32_t saved_esp = esp[3]; // not usually useful
    uint32_t ebx = esp[4];
    uint32_t edx = esp[5];
    uint32_t ecx = esp[6];
    uint32_t eax = esp[7];
    switch (eax) {
            case 0x0:  // print string
                printf("%s", (char*)ebx);
                break;

            case 0x1:  // exit
                printf("\nExit%d\n", ebx);
                asm volatile("jmp prompt_loop");
                __builtin_unreachable();
            default:
                printf("Unknown syscall number: 0x%x\n", eax);
                break;
        }
    return 0; // Optional return value
}