#include "uart.hpp"

// UART0 on the versatilepb board
#define UART0_BASE   0x101f1000

// UART Registers
#define UART0_DR     (UART0_BASE + 0x00)  // Data Register
#define UART0_RSR    (UART0_BASE + 0x04)  // Receive Status Register/Error Clear Register
#define UART0_ECR    (UART0_BASE + 0x04)  // Error Clear Register (same address as RSR)
#define UART0_FR     (UART0_BASE + 0x18)  // Flag Register
#define UART0_ILPR   (UART0_BASE + 0x20)  // IrDA Low-Power Counter Register
#define UART0_IBRD   (UART0_BASE + 0x24)  // Integer Baud Rate Divisor
#define UART0_FBRD   (UART0_BASE + 0x28)  // Fractional Baud Rate Divisor
#define UART0_LCRH   (UART0_BASE + 0x2C)  // Line Control Register
#define UART0_CR     (UART0_BASE + 0x30)  // Control Register
#define UART0_IFLS   (UART0_BASE + 0x34)  // Interrupt FIFO Level Select Register
#define UART0_IMSC   (UART0_BASE + 0x38)  // Interrupt Mask Set/Clear Register
#define UART0_RIS    (UART0_BASE + 0x3C)  // Raw Interrupt Status Register
#define UART0_MIS    (UART0_BASE + 0x40)  // Masked Interrupt Status Register
#define UART0_ICR    (UART0_BASE + 0x44)  // Interrupt Clear Register
#define UART0_DMACR  (UART0_BASE + 0x48)  // DMA Control Register

// FR bits
#define FR_RXFE      (1 << 4)   // Receive FIFO empty
#define FR_TXFF      (1 << 5)   // Transmit FIFO full

// Direct register access macros
#define REG(addr) (*(volatile unsigned int*)(addr))

void uart_init() {
    // Note: Basic UART initialization is already done in vector.s before C++ code runs
    // This function just sends a confirmation message
    
    // Clear any existing interrupts
    REG(UART0_ICR) = 0x7FF;  // Clear all interrupts
    
    // Send confirmation message
    uart_puts("C++ UART functions ready\r\n");
}

void uart_putc(char c) {
    // Wait for the UART to be ready to transmit
    while (REG(UART0_FR) & FR_TXFF);
    
    // Write the character to the data register
    REG(UART0_DR) = c;
    
    // Small delay to ensure character is sent
    for (volatile int i = 0; i < 1000; i++);
}

char uart_getc() {
    // Wait for the UART to receive something
    while (REG(UART0_FR) & FR_RXFE);
    
    // Read from the data register
    return REG(UART0_DR);
}

void uart_puts(const char* str) {
    // Add explicit null check
    if (!str) return;
    
    while (*str) {
        uart_putc(*str++);
    }
    
    // Force flush by adding a small delay
    for (volatile int i = 0; i < 10000; i++);
}
