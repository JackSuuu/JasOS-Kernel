#include "uart.hpp"
#include "memory.hpp"
#include "process.hpp"

// Forward declarations for standard functions
int strcmp(const char* s1, const char* s2);
void* memset(void* s, int c, unsigned int n);

// Define NULL if not defined
#ifndef NULL
#define NULL 0
#endif

// Simple version of kernel with minimal shell for testing input
extern "C" void _start_cpp() {
    // Initialize UART
    uart_init();
    uart_puts("\n\n");
    uart_puts("=======================================\n");
    uart_puts("     JasOS Kernel v0.2 (SIMPLIFIED)    \n");
    uart_puts("=======================================\n\n");
    
    uart_puts("This is a simplified kernel for testing input.\n");
    uart_puts("Memory and process management are still included.\n\n");
    
    // Initialize memory management
    memory_init();
    
    // Initialize process management
    process_init();
    
    uart_puts("Starting simple shell...\n");
    uart_puts("Commands: help, memory, process, exit\n\n");
    
    // Simple shell loop
    while (1) {
        uart_puts("> ");
        
        // Read input character by character
        char buffer[64];
        int pos = 0;
        
        for (int i = 0; i < 64; i++) {
            buffer[i] = 0;
        }
        
        while (1) {
            // Process scheduling - only once per command, not in inner loop
            process_schedule();
            
            // Get character
            char c = uart_getc();
            
            if (c == '\r' || c == '\n') {
                uart_puts("\n");
                buffer[pos] = 0;
                break;
            } else if (c == 8 || c == 127) { // Backspace
                if (pos > 0) {
                    pos--;
                    uart_puts("\b \b"); // Erase character
                }
            } else if (pos < 63) {
                uart_putc(c);
                buffer[pos++] = c;
            }
        }
        
        // Basic command processing
        if (pos == 0) {
            continue;
        } else if (strcmp(buffer, "help") == 0) {
            uart_puts("Available commands:\n");
            uart_puts("  help    - Show this help\n");
            uart_puts("  memory  - Show memory statistics\n");
            uart_puts("  process - Show process information\n");
            uart_puts("  test    - Create a test process\n");
            uart_puts("  exit    - Quit (halt system)\n");
        } else if (strcmp(buffer, "memory") == 0) {
            memory_dump();
        } else if (strcmp(buffer, "process") == 0) {
            process_dump();
        } else if (strcmp(buffer, "test") == 0) {
            int pid = process_create("test", NULL, 1);
            uart_puts("Created test process with PID ");
            
            // Convert PID to string
            char pid_str[8];
            int i = 0;
            int tmp = pid;
            if (tmp == 0) {
                pid_str[i++] = '0';
            } else {
                while (tmp > 0) {
                    pid_str[i++] = '0' + (tmp % 10);
                    tmp /= 10;
                }
            }
            pid_str[i] = 0;
            
            // Print in reverse
            for (int j = i - 1; j >= 0; j--) {
                uart_putc(pid_str[j]);
            }
            uart_puts("\n");
        } else if (strcmp(buffer, "exit") == 0) {
            uart_puts("Shutting down...\n");
            // Use ARM semihosting to exit QEMU
            asm volatile(
                "mov r0, #0x18\n"  // SYS_EXIT
                "mov r1, #0x20000\n"  // Successful exit
                "svc 0x123456\n"   // Semihosting SWI
            );
            
            // If semihosting didn't work, infinite loop
            while(1);
        } else {
            uart_puts("Unknown command: ");
            uart_puts(buffer);
            uart_puts("\n");
        }
    }
}

// Standard library function implementations
int strcmp(const char* s1, const char* s2) {
    while (*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    return *(const unsigned char*)s1 - *(const unsigned char*)s2;
}

void* memset(void* s, int c, unsigned int n) {
    unsigned char* p = (unsigned char*)s;
    while (n--) {
        *p++ = (unsigned char)c;
    }
    return s;
} 