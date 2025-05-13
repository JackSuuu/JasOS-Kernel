#ifndef MEMORY_HPP
#define MEMORY_HPP

// Memory block structure
struct MemoryBlock {
    unsigned int size;
    bool used;
    MemoryBlock* next;
};

// Memory management functions
void memory_init();
void* memory_alloc(unsigned int size);
void memory_free(void* ptr);
void memory_dump();

// Memory statistics
struct MemoryStats {
    unsigned int total_memory;
    unsigned int used_memory;
    unsigned int free_memory;
    unsigned int block_count;
    unsigned int used_blocks;
    unsigned int free_blocks;
};

MemoryStats memory_get_stats();

#endif // MEMORY_HPP 