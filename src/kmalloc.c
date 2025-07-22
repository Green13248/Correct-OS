#include <stdint.h>
#include <stdbool.h>
#include "include/kmalloc.h"
#include "include/memory.h"
#include "include/util.h"

static uint32_t heapStart;
static uint32_t heapSize;
static uint32_t threshold;
static bool kmallocInitb = false;

void kmallocInit(uint32_t initialHeapSize) {
    heapStart = KERNEL_MALLOC;
    heapSize = 0;
    threshold = 0;
    kmallocInitb = true;

    changeHeap_size(initialHeapSize);
}

void changeHeap_size(int new_size) {
    int old_page_top = CEIL_DIV(heapSize, 0x1000);
    int new_page_top = CEIL_DIV(new_size, 0x1000);
    int diff = new_page_top - old_page_top;

    for (int i = 0; i < diff; i++) {
        uint32_t phys = pmmAllocPageFrame();
        //Set a virtual address, and make sure the page can be written to by passing the PAGE_FLAG_WRITE flag
        memMapPage(KERNEL_MALLOC + old_page_top * 0x1000 + i * 0x1000, phys, PAGE_FLAG_WRITE);
    }
}