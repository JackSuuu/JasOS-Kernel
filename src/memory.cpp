#include "memory.hpp"
#include "uart.hpp"

// Forward declarations for standard functions
void* memset(void* s, int c, unsigned int n);
int strlen(const char* s);

// Heap size (256 KB)
#define HEAP_SIZE (256 * 1024)
// Minimum allocation size (accounting for block overhead)
#define MIN_ALLOC_SIZE 16

// Define NULL if not defined
#ifndef NULL
#define NULL 0
#endif

// Heap memory
static unsigned char heap[HEAP_SIZE];
// First memory block (head of the linked list)
static MemoryBlock* first_block = NULL;

// Convert integer to string
void int_to_str(unsigned int num, char* str) {
    if (num == 0) {
        str[0] = '0';
        str[1] = '\0';
        return;
    }

    int i = 0;
    char temp[16];
    
    while (num > 0) {
        temp[i++] = '0' + (num % 10);
        num /= 10;
    }
    
    for (int j = 0; j < i; j++) {
        str[j] = temp[i - j - 1];
    }
    str[i] = '\0';
}

// Initialize the memory manager
void memory_init() {
    // Create initial block covering the entire heap
    first_block = reinterpret_cast<MemoryBlock*>(heap);
    first_block->size = HEAP_SIZE - sizeof(MemoryBlock);
    first_block->used = false;
    first_block->next = NULL;
}

// Allocate memory
void* memory_alloc(unsigned int size) {
    // Ensure minimum allocation size
    if (size < MIN_ALLOC_SIZE) {
        size = MIN_ALLOC_SIZE;
    }
    
    // Align size to 4 bytes
    if (size % 4 != 0) {
        size += 4 - (size % 4);
    }
    
    // Find suitable free block
    MemoryBlock* current = first_block;
    while (current) {
        if (!current->used && current->size >= size) {
            // Found a suitable block
            
            // Split block if it's significantly larger than needed
            if (current->size >= size + sizeof(MemoryBlock) + MIN_ALLOC_SIZE) {
                // Calculate new block position (after allocated memory)
                unsigned char* new_block_addr = reinterpret_cast<unsigned char*>(current) + 
                                                sizeof(MemoryBlock) + size;
                MemoryBlock* new_block = reinterpret_cast<MemoryBlock*>(new_block_addr);
                
                // Setup new block
                new_block->size = current->size - size - sizeof(MemoryBlock);
                new_block->used = false;
                new_block->next = current->next;
                
                // Update current block
                current->size = size;
                current->next = new_block;
            }
            
            // Mark block as used
            current->used = true;
            
            // Return pointer to the memory after the block header
            return reinterpret_cast<void*>(reinterpret_cast<unsigned char*>(current) + 
                                           sizeof(MemoryBlock));
        }
        
        current = current->next;
    }
    
    // No suitable block found
    return NULL;
}

// Free allocated memory
void memory_free(void* ptr) {
    if (!ptr) return;
    
    // Get block header from pointer
    MemoryBlock* block = reinterpret_cast<MemoryBlock*>(
        reinterpret_cast<unsigned char*>(ptr) - sizeof(MemoryBlock)
    );
    
    // Mark block as free
    block->used = false;
    
    // Coalesce with next block if it's free
    while (block->next && !block->next->used) {
        // Merge blocks
        block->size += sizeof(MemoryBlock) + block->next->size;
        block->next = block->next->next;
    }
    
    // Coalesce with previous block if it's free
    MemoryBlock* prev = first_block;
    while (prev && prev->next != block) {
        prev = prev->next;
    }
    
    if (prev && !prev->used) {
        // Merge blocks
        prev->size += sizeof(MemoryBlock) + block->size;
        prev->next = block->next;
    }
}

// Get memory statistics
MemoryStats memory_get_stats() {
    MemoryStats stats;
    stats.total_memory = HEAP_SIZE;
    stats.used_memory = 0;
    stats.free_memory = 0;
    stats.block_count = 0;
    stats.used_blocks = 0;
    stats.free_blocks = 0;
    
    MemoryBlock* current = first_block;
    while (current) {
        stats.block_count++;
        
        if (current->used) {
            stats.used_memory += current->size;
            stats.used_blocks++;
        } else {
            stats.free_memory += current->size;
            stats.free_blocks++;
        }
        
        current = current->next;
    }
    
    // Account for memory used by block headers
    unsigned int header_size = stats.block_count * sizeof(MemoryBlock);
    stats.used_memory += header_size;
    
    return stats;
}

// Display memory usage
void memory_dump() {
    MemoryStats stats = memory_get_stats();
    
    uart_puts("Memory Statistics:\n");
    
    // Total memory
    uart_puts("  Total memory:  ");
    char buf[16];
    int_to_str(stats.total_memory, buf);
    uart_puts(buf);
    uart_puts(" bytes\n");
    
    // Used memory
    uart_puts("  Used memory:   ");
    int_to_str(stats.used_memory, buf);
    uart_puts(buf);
    uart_puts(" bytes (");
    int_to_str((stats.used_memory * 100) / stats.total_memory, buf);
    uart_puts(buf);
    uart_puts("%)\n");
    
    // Free memory
    uart_puts("  Free memory:   ");
    int_to_str(stats.free_memory, buf);
    uart_puts(buf);
    uart_puts(" bytes (");
    int_to_str((stats.free_memory * 100) / stats.total_memory, buf);
    uart_puts(buf);
    uart_puts("%)\n");
    
    // Block counts
    uart_puts("  Block count:   ");
    int_to_str(stats.block_count, buf);
    uart_puts(buf);
    uart_puts(" (");
    int_to_str(stats.used_blocks, buf);
    uart_puts(buf);
    uart_puts(" used, ");
    int_to_str(stats.free_blocks, buf);
    uart_puts(buf);
    uart_puts(" free)\n");
    
    // Memory map visualization
    uart_puts("\nMemory Map:\n");
    uart_puts("[");
    
    MemoryBlock* current = first_block;
    int map_width = 50; // Width of the map in characters
    unsigned int block_pos = 0;
    
    while (current) {
        // Calculate block's position in the map
        unsigned int block_size = current->size + sizeof(MemoryBlock);
        unsigned int block_end = block_pos + block_size;
        unsigned int display_width = (block_size * map_width) / HEAP_SIZE;
        if (display_width < 1) display_width = 1;
        
        // Display block
        for (unsigned int i = 0; i < display_width; i++) {
            uart_putc(current->used ? '#' : '.');
        }
        
        block_pos = block_end;
        current = current->next;
    }
    
    uart_puts("]\n");
    uart_puts("Legend: # = Used block, . = Free block\n");
} 