// Minimal UART test program

// UART0 on the versatilepb board
#define UART0 ((volatile unsigned int*)0x101f1000)

// Direct UART register offsets (in 32-bit words)
#define UART_DR     0  // Data Register
#define UART_FR     6  // Flag Register
#define UART_IBRD   9  // Integer Baud Rate Divisor
#define UART_FBRD   10 // Fractional Baud Rate Divisor
#define UART_LCRH   11 // Line Control Register
#define UART_CR     12 // Control Register

// FR register bit masks
#define FR_TXFF     (1 << 5)   // Transmit FIFO full
#define FR_RXFE     (1 << 4)   // Receive FIFO empty

void uart_init() {
    UART0[UART_CR] = 0;                  // Disable UART
    UART0[UART_IBRD] = 27;               // 115200 baud
    UART0[UART_FBRD] = 8;
    UART0[UART_LCRH] = (1 << 4) | (1 << 5) | (1 << 6);  // 8N1, FIFO enabled
    UART0[UART_CR] = (1 << 0) | (1 << 8) | (1 << 9);     // UART, Tx, Rx enabled
}

void uart_putc(char c) {
    // Wait until there's space in the FIFO
    while (UART0[UART_FR] & FR_TXFF);
    
    // Send the character
    UART0[UART_DR] = c;
}

void uart_puts(const char* str) {
    while (*str) {
        uart_putc(*str++);
    }
}

extern "C" void _start() {
    // Initialize UART
    uart_init();
    
    // Print test message
    uart_puts("\r\n\r\nJSOS UART Test\r\n");
    uart_puts("If you can see this, UART is working!\r\n");
    
    // Print some characters directly
    for (char c = 'A'; c <= 'Z'; c++) {
        uart_putc(c);
    }
    uart_puts("\r\n");
    
    // Infinite loop with heartbeat
    int count = 0;
    while (1) {
        // Print a heartbeat character every second
        uart_putc('.');
        if (++count % 80 == 0) uart_puts("\r\n");
        
        // Crude delay
        for (volatile int i = 0; i < 1000000; i++);
    }
}
