#include <stdint.h>
#include <stddef.h>
#include "include/liballoc.h"
#include "include/memory.h" // Your paging code with pmmAllocPageFrame
#include "include/VGA.h"    // For debug output if needed

#define PAGE_SIZE 4096

int liballoc_lock() {
    return 0;
}

int liballoc_unlock() {
    return 0;
}

// Allocates 'pages' pages of physical memory
void* liballoc_alloc(size_t pages) {
    size_t size = pages * 0x1000; // pages to bytes
    void* ptr = pmm_alloc_contiguous(size); // You need to implement this

    if (ptr == 0) return NULL;
    return ptr;
}

// Frees previously allocated memory
int liballoc_free(void* ptr, size_t pages) {
    size_t size = pages * 0x1000;
    pmm_free_contiguous(ptr, size); // You need to implement this
    return 0;
}