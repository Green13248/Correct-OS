ENTRY(_start)

SECTIONS
{
    /* Multiboot header at start of file and virtual address 0 */
    .multiboot2 0x0 : {
        KEEP(*(.multiboot2))
    }

    .boot 0xC0000000 : {
        . = ALIGN(0x1000);
        *(.boot)
    }

    .text : {
        . = ALIGN(0x1000);
        *(.text)
    }

    .rodata : {
        . = ALIGN(0x1000);
        *(.rodata)
    }

    .data : {
        . = ALIGN(0x1000);
        *(.data)
    }

    .bss : {
        . = ALIGN(0x1000);
        *(.bss)
        *(COMMON)
    }

    /* Define stack */
    stack_bottom = .;
    . = . + 0x20000; /* 128 KiB stack */
    stack_top = .;

    _kernel_end = .;
}