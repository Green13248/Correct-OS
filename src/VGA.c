#include <stdint.h>
#include <stddef.h>
#include <stdarg.h>
#include "include/VGA.h"


static inline uint8_t vga_entry_color(enum vga_color fg, enum vga_color bg) {
	return fg | bg << 4;
}

static inline uint16_t vga_entry(unsigned char uc, uint8_t color) {
	return (uint16_t) uc | (uint16_t) color << 8;
}

// String length
size_t strlen(const char* str) {
	size_t len = 0;
	while (str[len])
		len++;
	return len;
}

#define VGA_WIDTH   80
#define VGA_HEIGHT  25
#define VGA_MEMORY  0xC00B8000 

size_t terminal_row;
size_t terminal_column;
uint8_t terminal_color;
uint16_t* terminal_buffer = (uint16_t*)VGA_MEMORY;

void terminal_initialize(void) {
	terminal_row = 0;
	terminal_column = 0;
	terminal_color = vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
	for (size_t y = 0; y < VGA_HEIGHT; y++) {
		for (size_t x = 0; x < VGA_WIDTH; x++) {
			const size_t index = y * VGA_WIDTH + x;
			terminal_buffer[index] = vga_entry(' ', terminal_color);
		}
	}
}

void terminal_setcolor(uint8_t color) {
	terminal_color = color;
}

void terminal_putentryat(char c, uint8_t color, size_t x, size_t y) {
	const size_t index = y * VGA_WIDTH + x;
	terminal_buffer[index] = vga_entry(c, color);
}
void terminal_putchar(char c) {
    if (c == '\b') {
        if (terminal_column > 0) {
            terminal_column--;
        } else if (terminal_row > 0) {
            terminal_row--;
            terminal_column = VGA_WIDTH - 1;
        }
        terminal_putentryat(' ', terminal_color, terminal_column, terminal_row);
        return;
    }

    if (c == '\n') {
        terminal_column = 0;
        terminal_row++;
    } else if (c == '\t') {
        terminal_column += 4;
        if (terminal_column >= VGA_WIDTH) {
            terminal_column = 0;
            terminal_row++;
        }
    } else {
        terminal_putentryat(c, terminal_color, terminal_column, terminal_row);
        terminal_column++;
    }

    // Handle line wrap
    if (terminal_column >= VGA_WIDTH) {
        terminal_column = 0;
        terminal_row++;
    }

    // Handle scrolling
    if (terminal_row >= VGA_HEIGHT) {
        for (size_t y = 1; y < VGA_HEIGHT; y++) {
            for (size_t x = 0; x < VGA_WIDTH; x++) {
                terminal_buffer[(y - 1) * VGA_WIDTH + x] =
                    terminal_buffer[y * VGA_WIDTH + x];
            }
        }
        for (size_t x = 0; x < VGA_WIDTH; x++) {
            terminal_buffer[(VGA_HEIGHT - 1) * VGA_WIDTH + x] =
                vga_entry(' ', terminal_color);
        }
        terminal_row = VGA_HEIGHT - 1;
    }
}

void terminal_write(const char* data, size_t size) {
	for (size_t i = 0; i < size; i++)
		terminal_putchar(data[i]);
}

void terminal_writestring(const char* data) {
	terminal_write(data, strlen(data));
}

// Convert number to string (base 10 or 16)
void itoa(unsigned int value, char* str, int base) {
	char* ptr = str;
	char* ptr1 = str;
	char tmp_char;
	unsigned int tmp_value;

	// Handle zero explicitly
	if (value == 0) {
		*ptr++ = '0';
		*ptr = '\0';
		return;
	}

	while (value != 0) {
		tmp_value = value % base;
		if (tmp_value < 10)
			*ptr++ = '0' + tmp_value;
		else
			*ptr++ = 'a' + (tmp_value - 10);
		value /= base;
	}
	*ptr = '\0';

	// Reverse string
	ptr--;
	while (ptr1 < ptr) {
		tmp_char = *ptr;
		*ptr-- = *ptr1;
		*ptr1++ = tmp_char;
	}
}

// Very basic printf: supports %d and %x only
void printf(const char* fmt, ...) {
	va_list args;
	va_start(args, fmt);

	for (size_t i = 0; fmt[i] != '\0'; i++) {
		if (fmt[i] == '%') {
			i++;
			if (fmt[i] == 'd') {
				int val = va_arg(args, int);
				char buf[32];
				itoa(val, buf, 10);
				terminal_writestring(buf);
			} else if (fmt[i] == 'x') {
				unsigned int val = va_arg(args, unsigned int);
				char buf[32];
				itoa(val, buf, 16);
				terminal_writestring("0x");
				terminal_writestring(buf);
			} else if (fmt[i] == 's') {
				char* str = va_arg(args, char*);
				terminal_writestring(str);
			} else {
				// Unknown format, print it literally
				terminal_putchar('%');
				terminal_putchar(fmt[i]);
			}
		} else {
			terminal_putchar(fmt[i]);
		}
	}

	va_end(args);
}