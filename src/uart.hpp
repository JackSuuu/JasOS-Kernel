#pragma once

void uart_init();
void uart_putc(char c);
char uart_getc();
void uart_puts(const char* str);
