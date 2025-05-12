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
    uart_init();
    timer_init(1000000);  // 1MHz clock
    scheduler_init();
    
    // Create a test task
    create_task(test_task);
    
    kernel_main();
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