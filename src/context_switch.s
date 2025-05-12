.global context_switch
context_switch:
    push {r4-r11, lr}    // Save old context
    str sp, [r0]         // Save old SP
    ldr sp, [r1]         // Load new SP
    pop {r4-r11, lr}     // Restore new context
    bx lr
