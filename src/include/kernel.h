
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "multiboot.h"

void kernel_main(uint32_t magic, struct multiboot_info* bootInfo);
void prompt_loop();
void dump_sector(uint8_t* buffer);