ENTRY(kernel_main)

SECTIONS {
    . = 1M; /* Start the kernel at the 1MB mark */
    
    .text : {
        *(.text)   /* All the code goes into the .text section */
    }

    .data : {
        *(.data)   /* Initialized data goes here */
    }
    
    .bss : {
        *(.bss)    /* Uninitialized data */
    }
}
