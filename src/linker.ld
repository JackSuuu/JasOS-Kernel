ENTRY(_start)
SECTIONS {
    . = 0x10000;  /* Load address for versatilepb */
    
    /* Vector table must be first */
    .vectors : { 
        *(.vectors) 
    }
    
    /* Standard sections */
    .text : { 
        *(.text) 
    }
    .rodata : { 
        *(.rodata) 
    }
    .data : { 
        *(.data) 
    }
    .bss : { 
        *(.bss) 
        *(COMMON) 
    }
    
    /* Add padding to align to the next 4K boundary */
    . = ALIGN(4K);
}