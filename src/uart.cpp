#include "uart.hpp"

volatile unsigned int* uart = (volatile unsigned int*)0x101f1000;

void uart_init() {
    // No initialization needed for QEMU's emulated UART
}

void uart_putc(char c) {
    while(uart[6] & (1 << 5));
    uart[0] = c;
}

char uart_getc() {
    while(!(uart[6] & (1 << 4)));
    return uart[0];
}

void uart_puts(const char* str) {
    while(*str) uart_putc(*str++);
}