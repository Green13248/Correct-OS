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

#define USER_CS 0x1B
#define USER_DS 0x23

void print_hex(uint8_t val) {
    char* hex = "0123456789ABCDEF";
    terminal_putchar(hex[val >> 4]);
    terminal_putchar(hex[val & 0xF]);
}

void dump_sector(uint8_t* buffer) {
    for (int i = 0; i < 512; i++) {
        print_hex(buffer[i]);
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
    printf("Executing %s...\n", program_name);
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

void prompt_loop() {
	printf("\nA>");
	asm volatile("sti");
	while (enter_pressed != 1);
	enter_pressed = 0;
	input[21] = '\0'; // ensure null-terminated string
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

		// Convert input string to integer (e.g., "0012" -> 12)
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
				printf("Loaded file: %s\n", file_buffer);
			}
		} else if (input[0] == '.' && input[1] == '/') {
			index = 0;
			for (int i = 0; i < 21; i++) input[i] = '\0';
			printf("\nProgram name:\n");
			printf("A>");
			while (enter_pressed != 1);
			execute_program(input);
		}
	index = 0; // reset for next input
	for (int i = 0; i < 21; i++) {
		input[i] = '\0';
	}
	prompt_loop();
}

void kernel_main(uint32_t magic, struct multiboot_info* bootInfo) {
	terminal_initialize();
	terminal_setcolor(4);
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
	char* str = (char*)liballoc_alloc(64);
	printf("Drive found\n");
	execute_program("program.bin");
	prompt_loop();
	for(;;);
}