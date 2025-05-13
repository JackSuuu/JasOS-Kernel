#include "uart.hpp"

/*
 * PL011 UART Controller implementation for QEMU/VersatilePB
 * Based on PrimeCell UART (PL011) Technical Reference Manual
 */

// Base physical address of UART0
#define UART0_BASE      0x101F1000

// Register offsets from base address
#define UART_DR         0x00   // Data Register
#define UART_RSR        0x04   // Receive Status Register
#define UART_FR         0x18   // Flag Register
#define UART_ILPR       0x20   // IrDA Low-Power Counter Register
#define UART_IBRD       0x24   // Integer Baud Rate Register
#define UART_FBRD       0x28   // Fractional Baud Rate Register
#define UART_LCRH       0x2C   // Line Control Register
#define UART_CR         0x30   // Control Register
#define UART_IFLS       0x34   // Interrupt FIFO Level Select Register
#define UART_IMSC       0x38   // Interrupt Mask Set/Clear Register
#define UART_RIS        0x3C   // Raw Interrupt Status Register
#define UART_MIS        0x40   // Masked Interrupt Status Register
#define UART_ICR        0x44   // Interrupt Clear Register
#define UART_DMACR      0x48   // DMA Control Register

// Flag Register bits
#define FR_RXFE         (1 << 4)   // Receive FIFO empty
#define FR_TXFF         (1 << 5)   // Transmit FIFO full
#define FR_RXFF         (1 << 6)   // Receive FIFO full
#define FR_TXFE         (1 << 7)   // Transmit FIFO empty
#define FR_BUSY         (1 << 3)   // UART is busy transmitting data

// Line Control Register bits
#define LCRH_WLEN_8BIT  (0x3 << 5) // 8 bit word length
#define LCRH_FEN        (1 << 4)   // Enable FIFOs
#define LCRH_STP2       (1 << 3)   // Two stop bits
#define LCRH_EPS        (1 << 2)   // Even parity select
#define LCRH_PEN        (1 << 1)   // Parity enable
#define LCRH_BRK        (1 << 0)   // Send break

// Control Register bits
#define CR_UARTEN       (1 << 0)   // UART enable
#define CR_TXE          (1 << 8)   // Transmit enable
#define CR_RXE          (1 << 9)   // Receive enable

// Helper macros for register access
#define UART_REG(offset) (*(volatile unsigned int*)(UART0_BASE + (offset)))

// Initialize the UART
void uart_init() {
    // 1. Disable the UART before configuration
    UART_REG(UART_CR) = 0x0;
    
    // 2. Wait for the end of transmission or reception of the current character
    while (UART_REG(UART_FR) & FR_BUSY);
    
    // 3. Flush the transmit FIFO by setting the FEN bit to 0 in the Line Control Register
    UART_REG(UART_LCRH) &= ~LCRH_FEN;
    
    // 4. Configure the baud rate (115200 at 24MHz clock)
    // Divisor = 24MHz / (16 * 115200) = 13.0208...
    // Integer part = 13
    // Fractional part = 0.0208... * 64 = 1.33... → 1
    UART_REG(UART_IBRD) = 13;   // Integer part
    UART_REG(UART_FBRD) = 1;    // Fractional part
    
    // 5. Configure line settings: 8 bits, 1 stop bit, no parity, FIFOs enabled
    UART_REG(UART_LCRH) = LCRH_WLEN_8BIT | LCRH_FEN;
    
    // 6. Mask all interrupts
    UART_REG(UART_IMSC) = 0;
    
    // 7. Clear all pending interrupts
    UART_REG(UART_ICR) = 0x7FF;
    
    // 8. Finally, enable the UART, with RX and TX enabled
    UART_REG(UART_CR) = CR_UARTEN | CR_TXE | CR_RXE;
}

// Send a character
void uart_putc(char c) {
    // Wait until there is space in the transmit FIFO
    while (UART_REG(UART_FR) & FR_TXFF);
    
    // Write the character to the data register
    UART_REG(UART_DR) = c;
}

// Get a character
char uart_getc() {
    // Wait until there is data in the receive FIFO
    while (UART_REG(UART_FR) & FR_RXFE) {
        // Check for any errors
        if (UART_REG(UART_RSR)) {
            UART_REG(UART_RSR) = 0; // Clear errors
        }
    }
    
    // Read and return the received character
    char c = (char)(UART_REG(UART_DR) & 0xFF);
    
    return c;
}

// Output a string
void uart_puts(const char* str) {
    while (*str) {
        uart_putc(*str++);
    }
}
