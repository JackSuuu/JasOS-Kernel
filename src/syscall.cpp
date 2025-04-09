#include "syscalls.hpp"
#include "uart.hpp"

extern "C" int syscall_handler(uint32_t num, uint32_t arg1) {
    switch(num) {
        case SYS_WRITE:
            uart_putc((char)arg1);
            return 0;
        case SYS_READ:
            return (int)uart_getc();
        default:
            return -1;
    }
}