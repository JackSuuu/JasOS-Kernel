ENTRY(_start)
SECTIONS
{
    . = 0x10000; /* Kernel load address */
    .text : { *(.text*) }
    .rodata : { *(.rodata*) }
    .data : { *(.data*) }
    .bss : { *(.bss*) }
}
