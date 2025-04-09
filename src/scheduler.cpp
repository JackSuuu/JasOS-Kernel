#include "scheduler.hpp"
#include "memory.hpp"

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
            uint32_t* stack = (uint32_t*)((uint8_t*)malloc(4096) + 4096);
            stack -= 16;
            stack[15] = (uint32_t)entry;
            tasks[i].sp = stack;
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