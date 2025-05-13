#include "uart.hpp"

// Simple text-based test kernel using only UART for I/O
// No memory management or process scheduling
extern "C" void _start_cpp() {
    // Initialize UART
    uart_init();
    uart_puts("\n\n");
    uart_puts("=======================================\n");
    uart_puts("     JasOS UART TEST (UART ONLY)     \n");
    uart_puts("=======================================\n\n");
    
    uart_puts("This is a minimal test with UART only.\n");
    uart_puts("No memory management or process scheduling.\n\n");
    
    uart_puts("Type characters to echo them back:\n");
    
    // Simple echo loop
    while (1) {
        uart_puts("> ");
        
        // Read and echo characters until Enter is pressed
        char buffer[64];
        int pos = 0;
        
        while (1) {
            char c = uart_getc();
            
            // Echo the character
            uart_putc(c);
            
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
                buffer[pos++] = c;
            }
        }
        
        // Echo the full line back
        uart_puts("You typed: ");
        uart_puts(buffer);
        uart_puts("\n\n");
    }
} 