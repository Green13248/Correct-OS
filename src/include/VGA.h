
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include <stddef.h>
#include <stdint.h>

// VGA color constants
enum vga_color {
	VGA_COLOR_BLACK = 0,
	VGA_COLOR_BLUE = 1,
	VGA_COLOR_GREEN = 2,
	VGA_COLOR_CYAN = 3,
	VGA_COLOR_RED = 4,
	VGA_COLOR_MAGENTA = 5,
	VGA_COLOR_BROWN = 6,
	VGA_COLOR_LIGHT_GREY = 7,
	VGA_COLOR_DARK_GREY = 8,
	VGA_COLOR_LIGHT_BLUE = 9,
	VGA_COLOR_LIGHT_GREEN = 10,
	VGA_COLOR_LIGHT_CYAN = 11,
	VGA_COLOR_LIGHT_RED = 12,
	VGA_COLOR_LIGHT_MAGENTA = 13,
	VGA_COLOR_LIGHT_BROWN = 14,
	VGA_COLOR_WHITE = 15,
};

// Initialize the terminal
void terminal_initialize(void);

// Set the current text color
void terminal_setcolor(uint8_t color);

// Put a character at a specific location (x, y)
void terminal_putentryat(char c, uint8_t color, size_t x, size_t y);

// Put a character at the current cursor position
void terminal_putchar(char c);

// Write a buffer to the terminal
void terminal_write(const char* data, size_t size);

// Write a null-terminated string to the terminal
void terminal_writestring(const char* data);

// Basic printf implementation with %d, %x, %s
void printf(const char* format, ...);

// Integer to ASCII conversion (internal)
void itoa(unsigned int value, char* str, int base);