#pragma once

#define MAX_TASKS 4

void scheduler_init();
void schedule();
void create_task(void (*entry)());