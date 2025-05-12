#include <stdint.h>
#include "timer.hpp"
#include "scheduler.hpp"

// ARM Timer (SP804)
volatile uint32_t* timer = (volatile uint32_t*)0x101E2000;

void timer_init(uint32_t interval) {
    // Enable interrupts in the CPU
    asm volatile("mrs r0, cpsr");
    asm volatile("bic r0, r0, #0x80");  // Clear the I bit
    asm volatile("msr cpsr_c, r0");

    // Configure PIC (Interrupt Controller) at 0x10140000
    volatile uint32_t* pic = (volatile uint32_t*)0x10140000;
    pic[8] = 1 << 4;  // Enable timer 0 IRQ (bit 4)
    
    // Configure timer
    timer[2] = interval;       // Load value
    timer[0] = 0;              // Current value
    timer[3] = 0b10100010;     // Enable, periodic, 32-bit, interrupt enabled
}

extern "C" void timer_handler() {
    timer[3] |= (1 << 5);      // Clear interrupt
    schedule();                // Trigger scheduler
}