// ARM Timer (SP804)
volatile uint32_t* timer = (volatile uint32_t*)0x101E2000;

void timer_init(uint32_t interval) {
    timer[2] = interval;       // Load value
    timer[3] = 0b10100000;     // Enable, periodic, 32-bit
}

extern "C" void timer_handler() {
    timer[3] |= (1 << 5);      // Clear interrupt
    schedule();                // Trigger scheduler
}