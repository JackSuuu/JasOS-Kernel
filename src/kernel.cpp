#include "kernel.hpp"
#include "uart.hpp"
#include "scheduler.hpp"
#include "timer.hpp"

// Test task
void test_task() {
    while(1) {
        uart_puts("[Task] Hello from task!\r\n");
        for(volatile int i=0; i<1000000; i++); // Delay
    }
}

extern "C" void _start_cpp() {
    // Output debug markers directly
    volatile unsigned int* uart_dr = (volatile unsigned int*)(0x101f1000);
    *uart_dr = 'X';
    
    // Initialize hardware
    uart_init();
    
    // Send immediate debug output
    uart_puts("UART initialized\r\n");
    *uart_dr = 'A';
    
    // Continue with normal initialization
    timer_init(1000000);  // 1MHz clock
    uart_puts("Timer initialized\r\n");
    *uart_dr = 'B';
    
    scheduler_init();
    uart_puts("Scheduler initialized\r\n");
    *uart_dr = 'C';
    
    // Create a test task
    create_task(test_task);
    uart_puts("Task created\r\n");
    *uart_dr = 'D';
    
    // Enter main kernel loop
    kernel_main();
    
    // Should never get here
    while(1);
}

void kernel_main() {
    uart_puts("\nJSOS Kernel v0.1\n");
    uart_puts("Initialization complete\n");
    
    // Start simple shell
    while(1) {
        uart_puts("> ");
        char c = uart_getc();
        uart_putc(c);
        if(c == '\r') uart_puts("\n");
    }
}