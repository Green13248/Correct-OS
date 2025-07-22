#include "stdint.h"
#include <stddef.h>

void memset(void *dest, char val, uint32_t count);

void outPortB(uint16_t Port, uint8_t Value);

void outPortW(uint16_t Port, uint16_t Value);

char inPortB(uint16_t port);

static inline uint16_t inw(uint16_t port);

void intToStr(int N, char *str);

static inline uint16_t inw(uint16_t port);

int strncmp(const char *s1, const char *s2, size_t n);

struct InterruptRegisters{
    uint32_t cr2;
    uint32_t ds;
    uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax;
    uint32_t int_no, err_code;
    uint32_t eip, csm, eflags, useresp, ss;
};

#define CEIL_DIV(a,b) ((a+b) - 1)/b