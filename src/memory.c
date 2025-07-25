#include <stdint.h>
#include <stddef.h>
#include "include/multiboot.h"
#include "include/VGA.h"
#include "include/memory.h"
#include "include/util.h"

#define NUM_PAGES_DIRS 256
#define NUM_PAGE_FRAMES (0x1000000000 / 0x1000 / 8)

static uint32_t pageFrameMin;
static uint32_t pageFrameMax;
static uint32_t totalAlloc;
static uint32_t pageDirs[NUM_PAGES_DIRS][1024] __attribute__((aligned(4096)));
static uint8_t pageDirUsed[NUM_PAGES_DIRS];
int mem_num_vpages;

uint8_t physicalMemoryBitmap[NUM_PAGE_FRAMES / 8]; //Dynamically, bit array

void pmm_init(uint32_t memLow, uint32_t memHigh) {
    pageFrameMin = CEIL_DIV(memLow, 0x1000);
    pageFrameMax = memHigh / 0x1000;
    totalAlloc = 0;

    memset(physicalMemoryBitmap, 0, sizeof(physicalMemoryBitmap));
}

uint32_t* memGetCurrentPageDir() {
    uint32_t pd;
    asm volatile("mov %%cr3, %0": "=r"(pd));
    pd += KERNEL_START;
    return(uint32_t*)pd;
}

void memChangePageDir(uint32_t *pd) {
    pd = (uint32_t*)(((uint32_t)pd) - KERNEL_START);
    asm volatile("mov %0, %%eax \n mov %%eax, %%cr3 \n" :: "m"(pd));
}

void syncPageDirs(){
    for (int i = 0; i < NUM_PAGES_DIRS; i++){
        if (pageDirUsed[i]){
            uint32_t* pageDir = pageDirs[i];

            for (int i = 768; i < 1023; i++){
                pageDir[i] = initial_page_dir[i] & ~PAGE_FLAG_OWNER;
            }
        }
    }
}


void memMapPage(uint32_t virutalAddr, uint32_t physAddr, uint32_t flags){
    uint32_t *prevPageDir = 0;

    if (virutalAddr >= KERNEL_START){
        prevPageDir = memGetCurrentPageDir();
        if (prevPageDir != initial_page_dir){
            memChangePageDir(initial_page_dir);
        }
    }

    uint32_t pdIndex = virutalAddr >> 22;
    uint32_t ptIndex = virutalAddr >> 12 & 0x3FF;
    
    uint32_t* pageDir = REC_PAGEDIR;
    uint32_t* pt = REC_PAGETABLE(pdIndex);

    if (!(pageDir[pdIndex] & PAGE_FLAG_PRESENT)){
        uint32_t ptPAddr = pmmAllocPageFrame(); //Make a pointer to a page that has been allocated
        pageDir[pdIndex] = ptPAddr | PAGE_FLAG_PRESENT | PAGE_FLAG_WRITE | PAGE_FLAG_OWNER | flags; //init page dir and add flags
        invalidate(virutalAddr);

        //This loops takes our values and actually inits them
        for (uint32_t i = 0; i < 1024; i++){
            pt[i] = 0;
        }
    }

    pt[ptIndex] = physAddr | PAGE_FLAG_PRESENT | flags;
    mem_num_vpages++;
    invalidate(virutalAddr);

    if (prevPageDir != 0){
        syncPageDirs();

        if (prevPageDir != initial_page_dir){
            memChangePageDir(prevPageDir);
        }
    }
}


//First fit allocator
//Basically look through each page and see if it is open or not, if so return a pointer to said open address to allocate to, else if all fails return a 0 pointer
uint32_t pmmAllocPageFrame() {
    uint32_t start = pageFrameMin / 8 + ((pageFrameMin & 7) != 0 ? 1:0);
    uint32_t end = pageFrameMax / 8 - ((pageFrameMin & 7) != 0 ? 1:0);
    for (uint32_t b = start; b < end; b++) {
        uint8_t byte = physicalMemoryBitmap[b];
        if (byte == 0xFF) {
            continue;
        }

        for (uint32_t i = 0; i < 8; i++) {
            bool used = byte >> i & 1;
            if (!used) {
                byte ^= (-1 ^ byte) & (1 << i);
                physicalMemoryBitmap[b] = byte;
                totalAlloc++;
                uint32_t addr = (b*8*i) * 0X1000;
                return addr;
            }
        }
    }
    return 0;
}


void invalidate(uint32_t vaddr) {
    asm volatile("invlpg %0" :: "m"(vaddr));
}

void* pmm_alloc_contiguous(size_t size_in_bytes) {
    size_t pages = size_in_bytes / 0x1000;
    void* first = (void*)pmmAllocPageFrame(); // You can loop for N if needed
    return first;
}

void pmm_free_contiguous(void* ptr, size_t size) {
    // Optional: implement this if you want to support free() later
}

void initMemory(uint32_t memHigh, uint32_t physicalAllocStart) {
    initial_page_dir[0] = 0;
    invalidate(0);
    initial_page_dir[1023] = ((uint32_t) initial_page_dir - KERNEL_START) | PAGE_FLAG_PRESENT | PAGE_FLAG_WRITE;
    invalidate(0xFFFFF000);
    pmm_init(physicalAllocStart, memHigh);
    memset(pageDirs, 0, 0x1000 * NUM_PAGES_DIRS);
    memset(pageDirUsed, 0, NUM_PAGES_DIRS);
}
