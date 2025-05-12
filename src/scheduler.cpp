#include "scheduler.hpp"
#include "memory.hpp"
#include <stdint.h>

struct Task {
    uint32_t* sp;
    uint8_t status;
};

static Task tasks[MAX_TASKS];
static uint8_t current_task = 0;

extern "C" void context_switch(uint32_t** old_sp, uint32_t** new_sp);

void scheduler_init() {
    for(int i=0; i<MAX_TASKS; i++) tasks[i].status = 0;
}

void create_task(void (*entry)()) {
    for(int i=0; i<MAX_TASKS; i++) {
        if(tasks[i].status == 0) {
            // Allocate a smaller stack (1024 bytes)
            uint8_t* memory = static_cast<uint8_t*>(malloc(1024));
            if (!memory) return;  // Failed to allocate memory
            
            // Point to the top of the stack (ARM stacks grow downwards)
            uint32_t* stack = reinterpret_cast<uint32_t*>(memory + 1024);
            
            // Reserve space for register context (9 words: r4-r11, lr)
            stack -= 9;
            
            // Set the lr register position to point to the entry function
            // This will be restored during context switch
            stack[8] = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(entry));
            
            // Save stack pointer
            tasks[i].sp = stack;
            
            // Mark task as active
            tasks[i].status = 1;
            return;
        }
    }
}

void schedule() {
    uint8_t prev = current_task;
    do {
        current_task = (current_task + 1) % MAX_TASKS;
    } while(tasks[current_task].status != 1);
    
    context_switch(&tasks[prev].sp, &tasks[current_task].sp);
}