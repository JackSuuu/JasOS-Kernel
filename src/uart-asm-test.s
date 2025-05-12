@ filepath: /Users/jacksu/Desktop/PROJECTS/JasOS-kernel/src/uart-asm-test.s
.global _start
_start:
    @ Set up stack pointer
    ldr sp, =0x8000

    @ Initialize UART (PL011 at 0x101f1000)
    ldr r0, =0x101f1000   @ UART base address
    
    @ Disable UART
    mov r1, #0x00
    str r1, [r0, #0x30]   @ UART Control Register (UARTCR)
    
    @ Set baud rate (IBRD and FBRD)
    mov r1, #13           @ 115200 baud rate divisor
    str r1, [r0, #0x24]   @ UART Integer Baud Rate Divisor (UARTIBRD)
    mov r1, #1            @ Fractional part
    str r1, [r0, #0x28]   @ UART Fractional Baud Rate Divisor (UARTFBRD)
    
    @ 8 bits, 1 stop bit, no parity, FIFO enabled
    mov r1, #0x70         @ 0111 0000 binary
    str r1, [r0, #0x2C]   @ UART Line Control Register (UARTLCR_H)
    
    @ Enable UART, Tx, Rx
    ldr r1, =0x301        @ 0011 0000 0001 binary
    str r1, [r0, #0x30]   @ UART Control Register (UARTCR)
    
    @ Output a string
    ldr r4, =message
    
output_loop:
    ldrb r1, [r4], #1     @ Load next character and increment
    cmp r1, #0            @ Check for null terminator
    beq halt              @ Exit if done
    
    @ Wait until Tx FIFO is not full
wait_tx:
    ldr r2, [r0, #0x18]   @ UART Flag Register (UARTFR)
    and r2, r2, #(1<<5)   @ Check bit 5 (TXFF - Tx FIFO full)
    cmp r2, #0            @ If TXFF=0, FIFO is not full
    bne wait_tx           @ Otherwise, keep waiting
    
    @ Send the character
    str r1, [r0]          @ Write to UART Data Register (UARTDR)
    b output_loop

halt:
    @ Infinite loop
    b halt

.align 4
message:
    .asciz "UART assembly test - Hello from ARM assembly!\r\n"
