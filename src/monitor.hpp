#ifndef MONITOR_HPP
#define MONITOR_HPP

// System monitor functions
void monitor_init();
void monitor_update();
void monitor_process_command(const char* cmd, const char* arg);
void monitor_help();

// Monitor modes
enum MonitorMode {
    MONITOR_OVERVIEW,   // General system overview
    MONITOR_MEMORY,     // Detailed memory view
    MONITOR_PROCESS,    // Detailed process view
    MONITOR_HELP        // Help screen
};

#endif // MONITOR_HPP 