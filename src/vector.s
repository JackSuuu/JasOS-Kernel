.global _start
.section .vectors

_start:
    ldr pc, =reset_handler
    ldr pc, =undefined_handler
    ldr pc, =svc_handler
    ldr pc, =prefetch_abort_handler
    ldr pc, =data_abort_handler
    ldr pc, =unused_handler
    ldr pc, =irq_handler
    ldr pc, =fiq_handler

reset_handler:
    ldr sp, =0x8000
    bl _start_cpp
    b .

undefined_handler:
    b .

svc_handler:
    push {r0-r12, lr}
    mov r0, r7
    mov r1, r0
    bl syscall_handler
    pop {r0-r12, lr}
    movs pc, lr

prefetch_abort_handler:
    b .

data_abort_handler:
    b .

unused_handler:
    b .

irq_handler:
    push {r0-r12, lr}
    bl handle_irq
    pop {r0-r12, lr}
    subs pc, lr, #4

fiq_handler:
    b .

.global handle_irq
handle_irq:
    bl timer_handler
    bx lr