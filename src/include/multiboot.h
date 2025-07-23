#include <stdint.h>

struct multiboot_aout_symbol_table
{
  uint32_t tabsize;
  uint32_t strsize;
  uint32_t addr;
  uint32_t reserved;
};

struct multiboot_elf_section_header_table
{
  uint32_t num;
  uint32_t size;
  uint32_t addr;
  uint32_t shndx;
};

struct multiboot_info {
    uint32_t flags;

    // Available memory from BIOS
    uint32_t mem_lower;
    uint32_t mem_upper;

    // "root" partition
    uint32_t boot_device;

    // Kernel command line
    uint32_t cmdline;

    // Boot-Module list
    uint32_t mods_count;
    uint32_t mods_addr;
union {
        struct {
            uint32_t tabsize;
            uint32_t strsize;
            uint32_t addr;
            uint32_t reserved;
        } aout_sym;

        struct {
            uint32_t num;
            uint32_t size;
            uint32_t addr;
            uint32_t shndx;
        } elf_sec;
    } u; // (valid if bit 4 or 5 is set)

    // Memory map (valid if bit 6 is set)
    uint32_t mmap_length;
    uint32_t mmap_addr;

    // Drives (valid if bit 7 is set)
    uint32_t drives_length;
    uint32_t drives_addr;

    // ROM config table (valid if bit 8 is set)
    uint32_t config_table;

    // Bootloader name (valid if bit 9 is set)
    uint32_t boot_loader_name;

    // APM table (valid if bit 10 is set)
    uint32_t apm_table;

    // VBE info (valid if bit 11 is set)
    uint32_t vbe_control_info;
    uint32_t vbe_mode_info;
    uint16_t vbe_mode;
    uint16_t vbe_interface_seg;
    uint16_t vbe_interface_off;
    uint16_t vbe_interface_len;

    // Framebuffer info (valid if bit 12 is set)
    uint64_t framebuffer_addr;
    uint32_t framebuffer_pitch;
    uint32_t framebuffer_width;
    uint32_t framebuffer_height;
    uint8_t  framebuffer_bpp;
    uint8_t  framebuffer_type;
    uint8_t  reserved[2];
};

struct multiboot_mmap_entry
{
  uint32_t size;
  uint32_t addr_low;
  uint32_t addr_high;
  uint32_t len_low;
  uint32_t len_high;
#define MULTIBOOT_MEMORY_AVAILABLE              1
#define MULTIBOOT_MEMORY_RESERVED               2
#define MULTIBOOT_MEMORY_ACPI_RECLAIMABLE       3
#define MULTIBOOT_MEMORY_NVS                    4
#define MULTIBOOT_MEMORY_BADRAM                 5
  uint32_t type;
} __attribute__((packed, aligned(4)));
