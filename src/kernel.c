#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "include/gdt.h"
#include "include/idt.h"
#include "include/timer.h"
#include "include/VGA.h"
#include "include/keyboard.h"
#include "include/kernel.h"
#include "include/memory.h"
#include "include/kmalloc.h"
#include "include/liballoc.h"
#include "include/liballoc_hooks.h"
#include "include/ata_pio.h"
#include "include/SFS.h"
#include "include/util.h"
#include "include/syscall_dispatch.h"

void dump_sector(uint8_t* buffer) {
    for (int i = 0; i < 512; i++) {
        printf("%x", buffer[i]);
        terminal_putchar(' ');
        if ((i + 1) % 16 == 0)
            terminal_putchar('\n');
    }
}

void execute_program(const char *program_name) {
    void* entry = (void*)0xC0400000;
	if (!read_file(program_name, entry)) {
        printf("Failed to load program: %s\n", program_name);
        return;
    }
    printf("\nExecuting %s...\n", program_name);
    uint32_t* stack = (uint32_t*)0xC0BFFFF0;
    asm volatile (
        "cli\n"               // Disable interrupts
        "movl %0, %%esp\n"    // Set up new stack
        "xor %%ebp, %%ebp\n"  // Clear base pointer (important for some ABIs)
        "jmp *%1\n"           // Jump to entry point
        :
        : "r"(stack), "r"(entry)
        : "memory"
    );
}

#define FRAMEBUFFER_VIRTUAL 0xFFFF000000000000

void prompt_loop() {
	printf("\nA>");
	asm volatile("sti");
	while (enter_pressed != 1);
	enter_pressed = 0;
	input[21] = '\0';
	if (input[0] == 'l' && input[1] == 's') {
		list_files();
	} else if (input[0] == 'd' && input[1] == 'u' && input[2] == 'm' && input[3] == 'p') {
		index = 0;
		for (int i = 0; i < 20; i++) input[i] = '\0';

		printf("\nEnter LBA number:\n");
		printf("A>");
		while (enter_pressed != 1);
		enter_pressed = 0;
		input[21] = '\0';

		int sector = 0;
		for (int i = 0; i < 4; i++) {
			if (input[i] >= '0' && input[i] <= '9') {
				sector = sector * 10 + (input[i] - '0');
			}
		}

		uint8_t sector_buffer[512];
		read_sector(sector, sector_buffer);
		dump_sector(sector_buffer);
		} else if (input[0] == 't' && input[1] == 'y' && input[2] == 'p' && input[3] == 'e') {
			index = 0;
			for (int i = 0; i < 21; i++) input[i] = '\0';
			printf("\nFile name:\n");
			printf("A>");
			while (enter_pressed != 1);
			uint8_t file_buffer[2048];
			if (read_file(input, file_buffer)) {
				printf("\n%s\n", file_buffer);
			}
		} else if (input[0] == 'r' && input[1] == 'u' && input[2] == 'n') {
			index = 0;
			for (int i = 0; i < 21; i++) input[i] = '\0';
			printf("\nProgram name:\n");
			printf("A>");
			while (enter_pressed != 1);
			uint8_t buffer_temp[1];
			if (!read_file(input, buffer_temp)) {
				printf("\nNo such file or program ->%s", input);
			} else {
				execute_program(input);
			}
		} else if (input[0] == 'w' && input[1] == 'i' && input[2] == 'n') {
			printf("\nInit window");
			printf("\nStarting GUI");
		} else if (input[0] == 't' && input[1] == 's' && input[2] == 'c') {
			index = 0;
			for (int i = 0; i < 21; i++) input[i] = '\0';
			printf("\nColor (1-15)\n");
			printf("A>");
			while (enter_pressed != 1);
			uint8_t buffer_temp[1];
			int color = 0;
			for (int i = 0; i < 4; i++) {
				if (input[i] >= '0' && input[i] <= '9') {
					color = color * 10 + (input[i] - '0');
				}
			}
			if (color >= 0 && color < 15) {
				terminal_setcolor(color);
			} else {
				printf("\nThat color is not supported\n");
			}
		}
	index = 0;
	for (int i = 0; i < 21; i++) {
		input[i] = '\0';
	}
	prompt_loop();
}

VideoGraphicsArray *vgap;

void fill_rectangle(VideoGraphicsArray* vga, i32 x, i32 y, u32 w, u32 h, u32 color) {
    for (u32 dy = 0; dy < h; dy++) {
        for (u32 dx = 0; dx < w; dx++) {
            u32 px = x + dx;
            u32 py = y + dy;
            if (px < vga->width && py < vga->height) {
                u32 index = py * vga->width + px;
                ((u32*)vga->framebuffer)[index] = color;
            }
        }
    }
}

#define COM1 0x3F8


void draw_pixel(uint32_t* fb, int x, int y, int w, uint32_t color) {
    fb[y * w + x] = color;
}


void serial_init() {
    outPortB(COM1 + 1, 0x00);    // Disable interrupts
    outPortB(COM1 + 3, 0x80);    // Enable DLAB (set baud rate divisor)
    outPortB(COM1 + 0, 0x03);    // Set divisor to 3 (38400 baud)
    outPortB(COM1 + 1, 0x00);    // (high byte)
    outPortB(COM1 + 3, 0x03);    // 8 bits no parity one stop bit
    outPortB(COM1 + 2, 0xC7);    // Enable FIFO clear them 14-byte threshold
    outPortB(COM1 + 4, 0x0B);    // IRQs enabled RTS/DSR set
}

int serial_is_transmit_empty() {
    return inPortB(COM1 + 5) & 0x20;
}

void serial_write(char a) {
    while (!serial_is_transmit_empty());
    outPortB(COM1, a);
}

void serial_print(const char *str) {
    while (*str) {
        if (*str == '\n')
            serial_write('\r');
        serial_write(*str++);
    }
}

void serial_print_dec(uint32_t value) {
    char buf[11];
    int i = 10;
    buf[i--] = '\0';

    if (value == 0) {
        serial_write('0');
        return;
    }

    while (value > 0 && i >= 0) {
        buf[i--] = '0' + (value % 10);
        value /= 10;
    }

    serial_print(&buf[i + 1]);
}

void serial_print_hex(uint32_t value) {
    char hex_chars[] = "0123456789ABCDEF";
    serial_print("0x");
    for (int i = 7; i >= 0; i--) {
        uint8_t nibble = (value >> (i * 4)) & 0xF;
        serial_write(hex_chars[nibble]);
    }
}

void fill_the_screen(struct multiboot_info *bootinfo, uint32_t color) {
    volatile uint32_t *fb = (volatile uint32_t*)bootinfo->framebuffer_addr;
	serial_print(" step1 ");
	fb[0] = 0xFFFFFFFF;
    for (uint32_t y = 0; y < bootinfo->framebuffer_height; y++) {
		serial_print(" step2 ");
        for (uint32_t x = 0; x < bootinfo->framebuffer_width; x++) {
			serial_print(" step3 ");
            uint32_t offset = y * (bootinfo->framebuffer_pitch / 4) + x;
			serial_print(" step4 ");
            fb[offset] = color;
			serial_print(" step5 ");
        }
    }
	return;
}

void kernel_main(uint32_t magic, struct multiboot_info* bootInfo) {
	terminal_initialize();
	terminal_setcolor(14);
	printf("CorrectOS\nVersion 0.1\n");
	init_GDT();
	initIdt();
	//init_Timer(); THIS WORKS!
	initKeyboard();
	uint8_t *keyboard_input;
	uint32_t mod1 = *(uint32_t*)(bootInfo->mods_addr + 4);
	uint32_t physicalAllocStart = (mod1 + 0xFFF) & ~0xFFF;
	initMemory(bootInfo->mem_upper * 1024, physicalAllocStart);
	kmallocInit();
	//asm volatile("cli");
	char* str = (char*)liballoc_alloc(64);
	uint8_t test_sector[512];
	if (read_sector(1, test_sector) == 0) {
		printf("Drive found\n");
	} else {
		printf("No drive found, please fix and try again\n");
	}
	memMapPage(0x00010000, 0x00010000, 0x7);
	memMapPage(0x00011000, 0x00011000, 0x7);
	uint32_t fb_phys = bootInfo->framebuffer_addr;
	uint32_t fb_size = bootInfo->framebuffer_pitch * bootInfo->framebuffer_height;
	uint32_t fb_pages = (fb_size + 0xFFF) / 0x1000;

	uint32_t fb_virt = 0xE0100000;
	for (uint32_t i = 0; i < fb_pages; i++) {
		memMapPage(fb_phys + i * 0x1000, fb_virt + i * 0x1000, 0x7);
	}
	volatile uint32_t* fb = (volatile uint32_t*)fb_virt;
	serial_init();
	serial_print("hello world");
	serial_print_hex(bootInfo->framebuffer_addr);
	serial_print_dec(bootInfo->framebuffer_width);
	serial_print_dec(bootInfo->framebuffer_height);
	serial_print_dec(bootInfo->framebuffer_bpp);
	serial_print_dec(bootInfo->framebuffer_pitch);
	serial_print("Hi");
	serial_print("Hello world");
	serial_print_hex(bootInfo->framebuffer_addr);
	prompt_loop();
}