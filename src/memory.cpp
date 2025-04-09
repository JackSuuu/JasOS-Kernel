#include "memory.hpp"

#define HEAP_SIZE 0x10000
static uint8_t heap[HEAP_SIZE];
static size_t heap_ptr = 0;

void* malloc(size_t size) {
    if(heap_ptr + size > HEAP_SIZE) return nullptr;
    void* ptr = &heap[heap_ptr];
    heap_ptr += size;
    return ptr;
}

void free(void* ptr) {
    // Simple allocator doesn't support free
}