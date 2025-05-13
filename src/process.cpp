#include "process.hpp"
#include "memory.hpp"
#include "uart.hpp"

// Forward declarations
extern void* memset(void* s, int c, unsigned int n);
extern char* strcpy(char* dest, const char* src);
extern int strcmp(const char* s1, const char* s2);

// Define NULL if not defined
#ifndef NULL
#define NULL 0
#endif

// Global process table
static Process processes[MAX_PROCESSES];
// Current running process index
static int current_process = -1;
// Number of processes
static unsigned int process_count = 0;
// System uptime in milliseconds
static unsigned int system_uptime_ms = 0;
// CPU usage (percentage 0-100)
static unsigned int cpu_usage = 0;

// Helper function to convert int to string
void process_int_to_str(unsigned int num, char* str) {
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

// Initialize the process system
void process_init() {
    // Clear process table
    for (int i = 0; i < MAX_PROCESSES; i++) {
        processes[i].state = PROCESS_TERMINATED;
        processes[i].id = 0;
        processes[i].stack = NULL;
        processes[i].stack_size = 0;
        processes[i].priority = 0;
        processes[i].runtime_ms = 0;
    }
    
    // Create idle process (pid 0)
    process_create("idle", NULL, 0);
    
    // Start with the idle process
    current_process = 0;
    processes[0].state = PROCESS_RUNNING;
}

// Create a new process
int process_create(const char* name, void (*entry_point)(), unsigned int priority) {
    // Find free slot in process table
    int pid = -1;
    for (int i = 0; i < MAX_PROCESSES; i++) {
        if (processes[i].state == PROCESS_TERMINATED) {
            pid = i;
            break;
        }
    }
    
    if (pid == -1) {
        // No free slots
        return -1;
    }
    
    // Copy name
    int i = 0;
    while (name[i] && i < MAX_PROCESS_NAME - 1) {
        processes[pid].name[i] = name[i];
        i++;
    }
    processes[pid].name[i] = '\0';
    
    // Allocate stack if not idle process
    if (entry_point != NULL) {
        processes[pid].stack = (unsigned char*)memory_alloc(PROCESS_STACK_SIZE);
        if (processes[pid].stack == NULL) {
            // Memory allocation failed
            return -1;
        }
        processes[pid].stack_size = PROCESS_STACK_SIZE;
    } else {
        // Idle process doesn't need a stack
        processes[pid].stack = NULL;
        processes[pid].stack_size = 0;
    }
    
    // Initialize process
    processes[pid].state = PROCESS_READY;
    processes[pid].id = pid;
    processes[pid].priority = priority;
    processes[pid].runtime_ms = 0;
    processes[pid].created_at = system_uptime_ms;
    
    // Increment process count
    process_count++;
    
    return pid;
}

// Terminate a process
void process_terminate(unsigned int pid) {
    if (pid >= MAX_PROCESSES || processes[pid].state == PROCESS_TERMINATED) {
        return;
    }
    
    // Free the stack
    if (processes[pid].stack != NULL) {
        memory_free(processes[pid].stack);
        processes[pid].stack = NULL;
    }
    
    // Mark as terminated
    processes[pid].state = PROCESS_TERMINATED;
    
    // Decrement process count
    process_count--;
    
    // If terminating current process, force reschedule
    if ((int)pid == current_process) {
        process_schedule();
    }
}

// Simplified scheduler (round-robin)
void process_schedule() {
    // Currently we're implementing a very simple round-robin scheduler
    int next_process = current_process;
    
    // Find next ready process
    for (int i = 0; i < MAX_PROCESSES; i++) {
        next_process = (next_process + 1) % MAX_PROCESSES;
        if (processes[next_process].state == PROCESS_READY) {
            break;
        }
    }
    
    // If no ready process found, use idle process
    if (processes[next_process].state != PROCESS_READY) {
        next_process = 0;  // Idle process
    }
    
    // If different process selected, perform context switch
    if (next_process != current_process && current_process >= 0) {
        // Mark current as ready (if it was running)
        if (processes[current_process].state == PROCESS_RUNNING) {
            processes[current_process].state = PROCESS_READY;
        }
    }
    
    // Update current process
    current_process = next_process;
    processes[current_process].state = PROCESS_RUNNING;
    
    // Note: In a real OS, we would save/restore context here
    // For our simulation, we're just updating state
}

// Voluntarily yield CPU
void process_yield() {
    process_schedule();
}

// Get current process
Process* process_get_current() {
    if (current_process >= 0) {
        return &processes[current_process];
    }
    return NULL;
}

// Get process by ID
Process* process_get_by_id(unsigned int pid) {
    if (pid < MAX_PROCESSES) {
        return &processes[pid];
    }
    return NULL;
}

// Get process by name
Process* process_get_by_name(const char* name) {
    for (int i = 0; i < MAX_PROCESSES; i++) {
        if (processes[i].state != PROCESS_TERMINATED) {
            if (strcmp(processes[i].name, name) == 0) {
                return &processes[i];
            }
        }
    }
    return NULL;
}

// Update system time (should be called by timer interrupt)
void process_timer_tick(unsigned int ms) {
    system_uptime_ms += ms;
    
    // Update current process runtime
    if (current_process >= 0 && processes[current_process].state == PROCESS_RUNNING) {
        processes[current_process].runtime_ms += ms;
    }
    
    // Calculate CPU usage (excluding idle process)
    unsigned int total_runtime = 0;
    for (int i = 1; i < MAX_PROCESSES; i++) {  // Skip idle process
        if (processes[i].state != PROCESS_TERMINATED) {
            total_runtime += processes[i].runtime_ms;
        }
    }
    
    if (system_uptime_ms > 0) {
        cpu_usage = (total_runtime * 100) / system_uptime_ms;
    } else {
        cpu_usage = 0;
    }
}

// Get process statistics
ProcessStats process_get_stats() {
    ProcessStats stats;
    stats.total_processes = 0;
    stats.running_processes = 0;
    stats.ready_processes = 0;
    stats.blocked_processes = 0;
    stats.terminated_processes = 0;
    
    for (int i = 0; i < MAX_PROCESSES; i++) {
        switch (processes[i].state) {
            case PROCESS_RUNNING:
                stats.running_processes++;
                stats.total_processes++;
                break;
            case PROCESS_READY:
                stats.ready_processes++;
                stats.total_processes++;
                break;
            case PROCESS_BLOCKED:
                stats.blocked_processes++;
                stats.total_processes++;
                break;
            case PROCESS_TERMINATED:
                stats.terminated_processes++;
                break;
        }
    }
    
    stats.cpu_usage = cpu_usage;
    
    return stats;
}

// Display processes
void process_dump() {
    ProcessStats stats = process_get_stats();
    
    uart_puts("Process List:\n");
    uart_puts("--------------------------------------------------\n");
    uart_puts("PID  STATE     PRIORITY  RUNTIME   NAME\n");
    uart_puts("--------------------------------------------------\n");
    
    for (int i = 0; i < MAX_PROCESSES; i++) {
        if (processes[i].state != PROCESS_TERMINATED) {
            // PID
            char buf[16];
            process_int_to_str(processes[i].id, buf);
            if (processes[i].id < 10) uart_putc(' ');
            uart_puts(buf);
            uart_puts("   ");
            
            // State
            switch (processes[i].state) {
                case PROCESS_RUNNING:
                    uart_puts("RUNNING  ");
                    break;
                case PROCESS_READY:
                    uart_puts("READY    ");
                    break;
                case PROCESS_BLOCKED:
                    uart_puts("BLOCKED  ");
                    break;
                default:
                    uart_puts("UNKNOWN  ");
                    break;
            }
            
            // Priority
            process_int_to_str(processes[i].priority, buf);
            if (processes[i].priority < 10) uart_putc(' ');
            uart_puts(buf);
            uart_puts("        ");
            
            // Runtime
            process_int_to_str(processes[i].runtime_ms, buf);
            uart_puts(buf);
            uart_puts("ms    ");
            
            // Name
            uart_puts(processes[i].name);
            uart_puts("\n");
        }
    }
    
    // Statistics
    uart_puts("\nProcess Statistics:\n");
    uart_puts("  Total processes:  ");
    char buf[16];
    process_int_to_str(stats.total_processes, buf);
    uart_puts(buf);
    uart_puts("\n");
    
    uart_puts("  Running:  ");
    process_int_to_str(stats.running_processes, buf);
    uart_puts(buf);
    uart_puts("  Ready:  ");
    process_int_to_str(stats.ready_processes, buf);
    uart_puts(buf);
    uart_puts("  Blocked:  ");
    process_int_to_str(stats.blocked_processes, buf);
    uart_puts(buf);
    uart_puts("\n");
    
    uart_puts("  CPU Usage:  ");
    process_int_to_str(stats.cpu_usage, buf);
    uart_puts(buf);
    uart_puts("%\n");
    
    // Visual representation
    uart_puts("\nProcess Activity:\n");
    uart_puts("[");
    
    for (int i = 0; i < MAX_PROCESSES; i++) {
        if (i == current_process) {
            uart_putc('R');  // Running
        } else if (processes[i].state == PROCESS_READY) {
            uart_putc('r');  // Ready
        } else if (processes[i].state == PROCESS_BLOCKED) {
            uart_putc('b');  // Blocked
        } else {
            uart_putc('.');  // Terminated or unused
        }
    }
    
    uart_puts("]\n");
    uart_puts("Legend: R = Running, r = Ready, b = Blocked, . = Terminated\n");
} 