.global _start
.section .vectors

_start:
    ldr pc, =reset_handler
    ldr pc, =undefined_handler
    ldr pc, =svc_handler
    ldr pc, =prefetch_abort_handler
    ldr pc, =data_abort_handler
    ldr pc, =unused_handler
    ldr pc, =irq_handler
    ldr pc, =fiq_handler

reset_handler:
    // Minimal UART init and test print
    ldr r0, =0x101f1000   // UART0 base address
    
    // Disable UART (UARTCR @ offset 0x30)
    mov r1, #0
    str r1, [r0, #0x30]
    
    // Set baud rate: IBRD=39, FBRD=4 for 38400bps @ 24MHz clock
    // UARTIBRD @ offset 0x24
    // UARTFBRD @ offset 0x28
    mov r1, #39
    str r1, [r0, #0x24]   // Store IBRD
    mov r1, #4
    str r1, [r0, #0x28]   // Store FBRD
    
    // Configure Line Control Register (UARTLCR_H @ offset 0x2C)
    // 8-bit word length (WLEN=11), FIFO enabled (FEN=1)
    mov r1, #0x70         // Bits 6:5 for WLEN=11, Bit 4 for FEN=1
    str r1, [r0, #0x2C]
    
    // Enable UART, TXE, RXE (UARTCR @ offset 0x30)
    // UARTEN=1 (bit 0), TXE=1 (bit 8), RXE=1 (bit 9)
    ldr r1, =0x0301       // Correct value for enabling UART, TXE, RXE
    str r1, [r0, #0x30]

    // Send 'H'
    mov r1, #'H'
wait_tx_H:
    ldr r2, [r0, #0x18]   // UARTFR (Flag Register @ offset 0x18)
    tst r2, #(1 << 5)     // Test TXFF (Transmit FIFO Full, bit 5)
    bne wait_tx_H         // Loop if FIFO is full
    str r1, [r0, #0x00]   // Write 'H' to UARTDR (Data Register @ offset 0x00)

    // Send 'I'
    mov r1, #'I'
wait_tx_I:
    ldr r2, [r0, #0x18]
    tst r2, #(1 << 5)
    bne wait_tx_I
    str r1, [r0, #0x00]   // Write 'I'

    // Send CR (Carriage Return)
    mov r1, #0x0D
wait_tx_CR:
    ldr r2, [r0, #0x18]
    tst r2, #(1 << 5)
    bne wait_tx_CR
    str r1, [r0, #0x00]   // Write CR

    // Send LF (Line Feed)
    mov r1, #0x0A
wait_tx_LF:
    ldr r2, [r0, #0x18]
    tst r2, #(1 << 5)
    bne wait_tx_LF
    str r1, [r0, #0x00]   // Write LF

simple_halt_loop:
    b simple_halt_loop

// Original start_cpp label, not jumped to in this minimal test
start_cpp:
    b start_cpp // Loop indefinitely

undefined_handler:
    b undefined_handler
svc_handler:
    b svc_handler
prefetch_abort_handler:
    b prefetch_abort_handler
data_abort_handler:
    b data_abort_handler
unused_handler:
    b unused_handler
irq_handler:
    b irq_handler
fiq_handler:
    b fiq_handler

// Global symbol for IRQ handling, not used by this minimal assembly test directly
// but might be expected by linked C code.
.global handle_irq
handle_irq:
    b handle_irq // Loop indefinitely

.align 4
boot_msg:
    .asciz "Minimal Test Boot Message - Should Not Be Printed By This Handler"
