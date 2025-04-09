extern "C" void kernel_main() {
    const char* hello = "Hello, Kernel!";
    char* video_memory = (char*) 0xB8000; // Video memory location for text mode
    for (int i = 0; hello[i] != '\0'; i++) {
        video_memory[i * 2] = hello[i];      // Write the character
        video_memory[i * 2 + 1] = 0x0F;     // White text on black background
    }

    // Infinite loop to keep the kernel running
    while (1) {}
}
