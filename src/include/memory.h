#include <stddef.h>
#include <stdint.h>
#define KERNEL_START 0xC0000000
#define PAGE_FLAG_PRESENT (1 << 0)
#define PAGE_FLAG_WRITE (1 << 1)
#define KERNEL_MALLOC 0xD0000000
#define PAGE_FLAG_OWNER (1 << 9)
#define REC_PAGEDIR ((uint32_t*)0xFFFFF000)
#define REC_PAGETABLE(i) ((uint32_t*) (0xFFC00000 + ((i) << 12)))

extern uint32_t initial_page_dir[1024];
extern int mem_num_vpages;

uint32_t pmmAllocPageFrame();
void* pmm_alloc_contiguous(size_t size_in_bytes);
void pmm_free_contiguous(void* ptr, size_t size);
void initMemory(uint32_t memHigh, uint32_t physicalAllocStart);
extern uint32_t initial_page_dir[1024];
void pmm_init(uint32_t memLow, uint32_t memHigh);
void invalidate(uint32_t vaddr);
void initMemory(uint32_t memHigh, uint32_t physicalAllocStart);
uint32_t* memGetCurrentPageDir();
void memChangePageDir(uint32_t* pd);
void syncPageDirs();
void memMapPage(uint32_t virutalAddr, uint32_t physAddr, uint32_t flags);
