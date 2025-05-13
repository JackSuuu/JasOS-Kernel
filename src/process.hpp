#ifndef PROCESS_HPP
#define PROCESS_HPP

// Maximum number of processes
#define MAX_PROCESSES 16
// Maximum process name length
#define MAX_PROCESS_NAME 32
// Size of process stack (4KB per process)
#define PROCESS_STACK_SIZE 4096

// Process states
enum ProcessState {
    PROCESS_READY,     // Ready to run
    PROCESS_RUNNING,   // Currently running
    PROCESS_BLOCKED,   // Blocked (waiting for something)
    PROCESS_TERMINATED // Terminated
};

// Process control block structure
struct Process {
    char name[MAX_PROCESS_NAME];
    ProcessState state;
    unsigned int id;
    unsigned int priority;
    unsigned char* stack;
    unsigned int stack_size;
    unsigned int runtime_ms;
    unsigned int created_at;
};

// Process management functions
void process_init();
int process_create(const char* name, void (*entry_point)(), unsigned int priority);
void process_terminate(unsigned int pid);
void process_schedule();
void process_yield();
void process_dump();
Process* process_get_current();
Process* process_get_by_id(unsigned int pid);
Process* process_get_by_name(const char* name);
void process_timer_tick(unsigned int ms);

// Process Statistics
struct ProcessStats {
    unsigned int total_processes;
    unsigned int running_processes;
    unsigned int ready_processes;
    unsigned int blocked_processes;
    unsigned int terminated_processes;
    unsigned int cpu_usage;
};

ProcessStats process_get_stats();

#endif // PROCESS_HPP 