.global _start

_start:
    ldr sp, =stack_top
    bl _start_cpp

loop:
    b loop

.section .bss
.align 4
stack_bottom:
.skip 4096 /* 4KB stack */
stack_top:
