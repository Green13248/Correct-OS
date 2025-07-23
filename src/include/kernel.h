
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "multiboot.h"

typedef uint8_t  u8;
typedef int32_t  i32;
typedef uint32_t u32;
typedef uint64_t u64;

typedef struct VideoGraphicsArray {
    uint32_t width;
    uint32_t height;
    uint32_t pitch;
    uint8_t* framebuffer;

    void (*FillRectangle)(struct VideoGraphicsArray* self, int32_t x, int32_t y, uint32_t w, uint32_t h, uint32_t color);
} VideoGraphicsArray;

void kernel_main(uint32_t magic, struct multiboot_info* bootInfo);
void prompt_loop();
void dump_sector(uint8_t* buffer);