// Add forward declaration for main()
void main();

// Kernel entry point and UART output code
extern "C" void _start() {
    // Set stack pointer to 0x8000
    asm volatile("ldr sp, =0x8000");
    main();
    while (1); // Halt after main returns
}

// Write a character to UART
void uart_putc(char c) {
    volatile unsigned int *uart = (volatile unsigned int *)0x101f1000;
    *uart = c; // Write to UART data register
}

// Main function prints "Hello World!"
void main() {
    const char *str = "Hello World!\nWelcome to JsOS\n";
    while (*str) {
        uart_putc(*str++);
    }
}