.global _start
.section .vectors

_start:
    ldr pc, =reset_handler
    ldr pc, =svc_handler
    ldr pc, =timer_handler

reset_handler:
    ldr sp, =0x8000
    bl main
    b .

svc_handler:
    push {r0-r12, lr}
    mov r0, r7
    mov r1, r0
    bl syscall_handler
    pop {r0-r12, lr}
    movs pc, lr

timer_handler:
    push {r0-r12, lr}
    bl timer_handler
    pop {r0-r12, lr}
    subs pc, lr, #4