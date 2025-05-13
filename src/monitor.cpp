#include "monitor.hpp"
#include "memory.hpp"
#include "process.hpp"
#include "uart.hpp"

// Forward declarations
extern int strcmp(const char* s1, const char* s2);
extern void* memset(void* s, int c, unsigned int n);

// Define NULL if not defined
#ifndef NULL
#define NULL 0
#endif

// Current monitor mode
static MonitorMode current_mode = MONITOR_OVERVIEW;

// Helper function to draw horizontal line
void monitor_draw_line(char c, int length) {
    for (int i = 0; i < length; i++) {
        uart_putc(c);
    }
    uart_puts("\n");
}

// Helper function to convert int to string
void monitor_int_to_str(unsigned int num, char* str) {
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

// Draw a bar graph (percentage based)
void monitor_draw_bar(unsigned int percentage, int width) {
    int filled = (percentage * width) / 100;
    
    uart_putc('[');
    for (int i = 0; i < width; i++) {
        if (i < filled) {
            uart_putc('#');
        } else {
            uart_putc(' ');
        }
    }
    uart_putc(']');
    
    // Add percentage
    uart_putc(' ');
    char buf[5];
    monitor_int_to_str(percentage, buf);
    uart_puts(buf);
    uart_puts("%");
}

// Initialize the system monitor
void monitor_init() {
    current_mode = MONITOR_OVERVIEW;
}

// Display system overview (default mode)
void monitor_show_overview() {
    // Get system statistics
    MemoryStats mem_stats = memory_get_stats();
    ProcessStats proc_stats = process_get_stats();
    
    // Clear screen (ANSI escape sequence)
    uart_puts("\033[2J\033[H");
    
    // Title
    uart_puts("=== JasOS System Monitor - OVERVIEW ===\n\n");
    
    // Memory overview
    uart_puts("Memory Usage:\n");
    monitor_draw_line('-', 50);
    
    // Calculate percentage
    unsigned int mem_percentage = (mem_stats.used_memory * 100) / mem_stats.total_memory;
    
    // Draw bar
    uart_puts("  ");
    monitor_draw_bar(mem_percentage, 30);
    uart_puts("\n");
    
    // Memory details
    uart_puts("  Total: ");
    char buf[16];
    monitor_int_to_str(mem_stats.total_memory, buf);
    uart_puts(buf);
    uart_puts(" bytes   Used: ");
    monitor_int_to_str(mem_stats.used_memory, buf);
    uart_puts(buf);
    uart_puts(" bytes   Free: ");
    monitor_int_to_str(mem_stats.free_memory, buf);
    uart_puts(buf);
    uart_puts(" bytes\n");
    
    // Process overview
    uart_puts("\nProcess Usage:\n");
    monitor_draw_line('-', 50);
    
    // CPU usage
    uart_puts("  CPU: ");
    monitor_draw_bar(proc_stats.cpu_usage, 30);
    uart_puts("\n");
    
    // Process counts
    uart_puts("  Total: ");
    monitor_int_to_str(proc_stats.total_processes, buf);
    uart_puts(buf);
    uart_puts("   Running: ");
    monitor_int_to_str(proc_stats.running_processes, buf);
    uart_puts(buf);
    uart_puts("   Ready: ");
    monitor_int_to_str(proc_stats.ready_processes, buf);
    uart_puts(buf);
    uart_puts("   Blocked: ");
    monitor_int_to_str(proc_stats.blocked_processes, buf);
    uart_puts(buf);
    uart_puts("\n");
    
    // Show current process
    Process* current = process_get_current();
    if (current != NULL) {
        uart_puts("\nCurrent Process: ");
        uart_puts(current->name);
        uart_puts(" (PID ");
        monitor_int_to_str(current->id, buf);
        uart_puts(buf);
        uart_puts(")\n");
    }
    
    // Commands help
    uart_puts("\nCommands: mem, proc, help, exit\n");
}

// Display detailed memory view
void monitor_show_memory() {
    // Get memory statistics
    MemoryStats stats = memory_get_stats();
    
    // Clear screen (ANSI escape sequence)
    uart_puts("\033[2J\033[H");
    
    // Title
    uart_puts("=== JasOS System Monitor - MEMORY ===\n\n");
    
    // Memory statistics
    uart_puts("Memory Statistics:\n");
    monitor_draw_line('-', 50);
    
    // Total memory
    uart_puts("  Total memory:  ");
    char buf[16];
    monitor_int_to_str(stats.total_memory, buf);
    uart_puts(buf);
    uart_puts(" bytes\n");
    
    // Used memory
    uart_puts("  Used memory:   ");
    monitor_int_to_str(stats.used_memory, buf);
    uart_puts(buf);
    uart_puts(" bytes (");
    monitor_int_to_str((stats.used_memory * 100) / stats.total_memory, buf);
    uart_puts(buf);
    uart_puts("%)\n");
    
    // Free memory
    uart_puts("  Free memory:   ");
    monitor_int_to_str(stats.free_memory, buf);
    uart_puts(buf);
    uart_puts(" bytes (");
    monitor_int_to_str((stats.free_memory * 100) / stats.total_memory, buf);
    uart_puts(buf);
    uart_puts("%)\n");
    
    // Block counts
    uart_puts("  Block count:   ");
    monitor_int_to_str(stats.block_count, buf);
    uart_puts(buf);
    uart_puts(" (");
    monitor_int_to_str(stats.used_blocks, buf);
    uart_puts(buf);
    uart_puts(" used, ");
    monitor_int_to_str(stats.free_blocks, buf);
    uart_puts(buf);
    uart_puts(" free)\n");
    
    // Memory map visualization
    uart_puts("\nMemory Map:\n");
    monitor_draw_line('-', 50);
    
    // Call memory dump to show memory map
    memory_dump();
    
    // Commands help
    uart_puts("\nCommands: overview, proc, help, exit\n");
}

// Display detailed process view
void monitor_show_process() {
    // Get process statistics
    ProcessStats stats = process_get_stats();
    
    // Clear screen (ANSI escape sequence)
    uart_puts("\033[2J\033[H");
    
    // Title
    uart_puts("=== JasOS System Monitor - PROCESS ===\n\n");
    
    // Process statistics
    uart_puts("Process Statistics:\n");
    monitor_draw_line('-', 50);
    
    // Process counts
    uart_puts("  Total processes:  ");
    char buf[16];
    monitor_int_to_str(stats.total_processes, buf);
    uart_puts(buf);
    uart_puts("\n");
    
    uart_puts("  Running:  ");
    monitor_int_to_str(stats.running_processes, buf);
    uart_puts(buf);
    uart_puts("  Ready:  ");
    monitor_int_to_str(stats.ready_processes, buf);
    uart_puts(buf);
    uart_puts("  Blocked:  ");
    monitor_int_to_str(stats.blocked_processes, buf);
    uart_puts(buf);
    uart_puts("\n");
    
    uart_puts("  CPU Usage:  ");
    monitor_int_to_str(stats.cpu_usage, buf);
    uart_puts(buf);
    uart_puts("%\n");
    
    // Process list
    uart_puts("\nProcess List:\n");
    monitor_draw_line('-', 64);
    uart_puts("PID  STATE     PRIORITY  RUNTIME   NAME\n");
    monitor_draw_line('-', 64);
    
    // Call process dump to show process list
    process_dump();
    
    // Commands help
    uart_puts("\nCommands: overview, mem, help, exit\n");
}

// Display help screen
void monitor_show_help() {
    // Clear screen (ANSI escape sequence)
    uart_puts("\033[2J\033[H");
    
    // Title
    uart_puts("=== JasOS System Monitor - HELP ===\n\n");
    
    // Help information
    uart_puts("Commands:\n");
    monitor_draw_line('-', 50);
    uart_puts("  overview - Show system overview\n");
    uart_puts("  mem      - Show detailed memory information\n");
    uart_puts("  proc     - Show detailed process information\n");
    uart_puts("  help     - Show this help screen\n");
    uart_puts("  exit     - Exit the monitor and return to shell\n");
    
    uart_puts("\nMemory Management:\n");
    monitor_draw_line('-', 50);
    uart_puts("  Heap size:   256 KB\n");
    uart_puts("  Allocation:  First-fit with block splitting/coalescing\n");
    uart_puts("  Monitors:    Used/free memory, block fragmentation\n");
    
    uart_puts("\nProcess Management:\n");
    monitor_draw_line('-', 50);
    uart_puts("  Max processes: 16\n");
    uart_puts("  Scheduling:    Simple round-robin\n");
    uart_puts("  Stack size:    4 KB per process\n");
    uart_puts("  States:        Ready, Running, Blocked, Terminated\n");
    
    // Commands help
    uart_puts("\nPress Enter or type a command to continue\n");
}

// Update monitor display based on current mode
void monitor_update() {
    switch (current_mode) {
        case MONITOR_OVERVIEW:
            monitor_show_overview();
            break;
        case MONITOR_MEMORY:
            monitor_show_memory();
            break;
        case MONITOR_PROCESS:
            monitor_show_process();
            break;
        case MONITOR_HELP:
            monitor_show_help();
            break;
        default:
            monitor_show_overview();
            break;
    }
}

// Process monitor commands
void monitor_process_command(const char* cmd, const char* /* arg */) {
    // Handle mode changes based on command
    if (strcmp(cmd, "overview") == 0) {
        current_mode = MONITOR_OVERVIEW;
    } else if (strcmp(cmd, "mem") == 0) {
        current_mode = MONITOR_MEMORY;
    } else if (strcmp(cmd, "proc") == 0) {
        current_mode = MONITOR_PROCESS;
    } else if (strcmp(cmd, "help") == 0) {
        current_mode = MONITOR_HELP;
    } else if (strcmp(cmd, "exit") == 0) {
        // Return to shell (handled by caller)
        return;
    } else {
        // Unknown command
        uart_puts("Unknown command: ");
        uart_puts(cmd);
        uart_puts("\n");
        uart_puts("Type 'help' for available commands\n");
        return;
    }
    
    // Update display based on new mode
    monitor_update();
}

// Display monitor help
void monitor_help() {
    current_mode = MONITOR_HELP;
    monitor_update();
} 