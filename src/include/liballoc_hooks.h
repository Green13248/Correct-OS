#ifndef LIBALLOC_HOOKS_H
#define LIBALLOC_HOOKS_H

#include <stdint.h>
#include <stddef.h>

void* liballoc_alloc(size_t pages);
int liballoc_free(void* ptr, size_t pages);
int liballoc_lock();
int liballoc_unlock();

#endif
