/* Linker script to configure memory layout */
ENTRY(_start)
SECTIONS {
    . = 0x10000; /* Kernel load address for QEMU versatilepb */
    .text : {
        *(.text)   /* Code */
        *(.rodata) /* Read-only data (e.g., strings) */
    }
    .data : { *(.data) }
    .bss  : { *(.bss)  }
}