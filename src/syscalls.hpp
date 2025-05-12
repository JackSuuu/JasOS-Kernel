#pragma once
#include <stdint.h>

#define SYS_WRITE 1
#define SYS_READ 2

extern "C" int syscall_handler(uint32_t num, uint32_t arg1);