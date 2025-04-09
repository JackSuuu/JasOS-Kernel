#include "kernel.hpp"
#include "uart.hpp"
#include "scheduler.hpp"
#include "timer.hpp"

extern "C" void _start() {
    asm volatile("ldr sp, =0x8000");
    uart_init();
    timer_init(1000000);  // 1MHz clock
    scheduler_init();
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