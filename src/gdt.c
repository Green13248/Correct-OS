#include "include/gdt.h"
#include "include/util.h"

extern void gdt_flush(uint32_t);
extern void tss_flush();

struct gdt_entry_struct gdt_entries[6];
struct gdt_ptr_struct gdt_ptr;
struct tss_entry_struct tss_entry;

void init_GDT() {
    gdt_ptr.limit = (sizeof(struct gdt_entry_struct) * 6) - 1;
    gdt_ptr.base = (unsigned int)gdt_entries;
    set_GDT_Gate(0,0,0,0,0); //Null segment (just because)
    set_GDT_Gate(1, 0, 0xFFFFFFFF, 0x9A, 0xCF); //Kernel Code segment 9A = 1001 1010, go through GDT wiki look at each bit
    set_GDT_Gate(2, 0, 0xFFFFFFFF, 0x92, 0xCF); //Kernel Data segment (9 = kernel) (2 = data)
    set_GDT_Gate(3, 0, 0xFFFFFFFF, 0xFA, 0xCF); //User code segement (F = user) (A = code)
    set_GDT_Gate(4, 0, 0xFFFFFFFF, 0xF2, 0xCF); //User data segment (F = user) (2 = data)
    writeTSS(5, 0x10, 0x0);
    gdt_flush((uint32_t)&gdt_ptr);
    tss_flush();
}

void writeTSS(uint32_t num, uint16_t ss0, uint32_t esp0) {
    uint32_t base = (uint32_t) &tss_entry;
    uint32_t limit = sizeof(tss_entry) - 1;

    set_GDT_Gate(num, base, limit, 0x89, 0x00);  // Use 0x89 for TSS (available)
    memset(&tss_entry, 0, sizeof(tss_entry));

    tss_entry.ss0 = ss0;
    tss_entry.esp0 = esp0;

    // User-mode segments (DPL=3)
    tss_entry.cs = 0x1B; // user code
    tss_entry.ss = 0x23; // user data
    tss_entry.ds = 0x23;
    tss_entry.es = 0x23;
    tss_entry.fs = 0x23;
    tss_entry.gs = 0x23;
}


void set_GDT_Gate(uint32_t num, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran) {
    gdt_entries[num].base_low = (base & 0xFFFF);
    gdt_entries[num].base_middle = (base >> 16) & 0xFF;
    gdt_entries[num].base_high = (base >> 24) & 0xFF;
    gdt_entries[num].limit = (limit & 0xFFFF);
    gdt_entries[num].flags = (limit >> 16) & 0x0F;
    gdt_entries[num].flags |= (gran & 0xF0);
    gdt_entries[num].access = access;
}