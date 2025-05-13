#include "uart.hpp"
#include "memory.hpp"
#include "process.hpp"
#include "monitor.hpp"

// Forward declarations for standard functions
int strcmp(const char* s1, const char* s2);
void* memset(void* s, int c, unsigned int n);
char* strcpy(char* dest, const char* src);
char* strcat(char* dest, const char* src);
int strlen(const char* str);
char* strtok(char* str, const char* delim);

// Maximum number of files/directories per directory
#define MAX_FILES 16
// Maximum path length
#define MAX_PATH 128
// Maximum filename length
#define MAX_NAME 32
// Maximum file content size
#define MAX_FILE_SIZE 1024
// Maximum line length for editor
#define MAX_LINE_LENGTH 80
// NULL definition if not already defined
#ifndef NULL
#define NULL 0
#endif

// Simple memory allocation system using our new memory management
static bool use_old_malloc = true;  // Flag to use old or new memory system

// Original heap (will be deprecated)
#define OLD_HEAP_SIZE 32768  // 32 KB heap
static unsigned char old_heap[OLD_HEAP_SIZE];
static unsigned int old_heap_pos = 0;

// Original allocation function (deprecated)
void* old_kmalloc(unsigned int size) {
    if (old_heap_pos + size > OLD_HEAP_SIZE) {
        // Out of memory
        return NULL;
    }
    
    void* ptr = &old_heap[old_heap_pos];
    old_heap_pos += size;
    
    // Zero-initialize memory
    for (unsigned int i = 0; i < size; i++) {
        ((unsigned char*)ptr)[i] = 0;
    }
    
    return ptr;
}

// Allocate memory (wrapper function)
void* kmalloc(unsigned int size) {
    if (use_old_malloc) {
        return old_kmalloc(size);
    } else {
        return memory_alloc(size);
    }
}

// File system node type
enum NodeType {
    TYPE_FILE,
    TYPE_DIRECTORY
};

// File system node structure
struct FSNode {
    char name[MAX_NAME];
    NodeType type;
    struct FSNode* parent;
    
    // For directories only
    struct FSNode* children[MAX_FILES];
    int child_count;
    
    // For files only
    char* content;
    unsigned int content_size;
    unsigned int content_capacity;
};

// Global file system state
FSNode* root_dir = NULL;
FSNode* current_dir = NULL;

// Buffer for storing current path
char current_path[MAX_PATH];

// Function to create a new node
FSNode* create_node(const char* name, NodeType type, FSNode* parent) {
    FSNode* node = (FSNode*)kmalloc(sizeof(FSNode));
    
    // Initialize node
    for (int i = 0; i < MAX_NAME && name[i]; i++) {
        node->name[i] = name[i];
    }
    node->type = type;
    node->parent = parent;
    node->child_count = 0;
    
    // Initialize based on type
    if (type == TYPE_DIRECTORY) {
        // Clear children array for directories
        for (int i = 0; i < MAX_FILES; i++) {
            node->children[i] = NULL;
        }
        node->content = NULL;
        node->content_size = 0;
        node->content_capacity = 0;
    } else {
        // Allocate initial content buffer for files
        node->content = (char*)kmalloc(MAX_FILE_SIZE);
        node->content_size = 0;
        node->content_capacity = MAX_FILE_SIZE;
    }
    
    return node;
}

// Initialize the file system
void fs_init() {
    // Create root directory
    root_dir = create_node("/", TYPE_DIRECTORY, NULL);
    current_dir = root_dir;
    
    // Create initial files and directories
    
    // Create a system directory
    FSNode* system_dir = create_node("system", TYPE_DIRECTORY, root_dir);
    root_dir->children[root_dir->child_count++] = system_dir;
    
    // Create a sample README file in the root directory
    FSNode* readme = create_node("README.txt", TYPE_FILE, root_dir);
    root_dir->children[root_dir->child_count++] = readme;
    
    // Add content to README
    const char* readme_content = "Welcome to JasOS!\n\nThis is a simple operating system with a text-based interface.\n"
                          "Use the 'help' command to see available commands.\n"
                          "Type 'monitor' to start the system monitor.\n";
    int len = 0;
    while (readme_content[len]) {
        readme->content[len] = readme_content[len];
        len++;
    }
    readme->content_size = len;
}

// Command to list directory contents
void cmd_ls() {
    if (current_dir->child_count == 0) {
        uart_puts("Directory is empty\n");
        return;
    }
    
    for (int i = 0; i < current_dir->child_count; i++) {
        FSNode* node = current_dir->children[i];
        
        if (node->type == TYPE_DIRECTORY) {
            uart_puts("[DIR]  ");
        } else {
            uart_puts("[FILE] ");
            // Print file size
            char size_buf[10];
            int size = node->content_size;
            int pos = 0;
            
            // Convert size to string
            if (size == 0) {
                size_buf[0] = '0';
                size_buf[1] = 0;
            } else {
                while (size > 0 && pos < 9) {
                    size_buf[pos++] = '0' + (size % 10);
                    size /= 10;
                }
                size_buf[pos] = 0;
                
                // Reverse the string
                for (int j = 0; j < pos / 2; j++) {
                    char temp = size_buf[j];
                    size_buf[j] = size_buf[pos - j - 1];
                    size_buf[pos - j - 1] = temp;
                }
            }
            
            uart_puts(size_buf);
            uart_puts(" bytes  ");
        }
        uart_puts(node->name);
        uart_puts("\n");
    }
}

// Command to change directory
void cmd_cd(const char* path) {
    // Handle special paths
    if (strcmp(path, "/") == 0) {
        current_dir = root_dir;
        return;
    }
    
    if (strcmp(path, "..") == 0) {
        if (current_dir->parent != NULL) {
            current_dir = current_dir->parent;
        }
        return;
    }
    
    // Search for directory in current directory
    for (int i = 0; i < current_dir->child_count; i++) {
        if (strcmp(current_dir->children[i]->name, path) == 0) {
            if (current_dir->children[i]->type == TYPE_DIRECTORY) {
                current_dir = current_dir->children[i];
                return;
            } else {
                uart_puts("Not a directory: ");
                uart_puts(path);
                uart_puts("\n");
                return;
            }
        }
    }
    
    uart_puts("Directory not found: ");
    uart_puts(path);
    uart_puts("\n");
}

// Command to create a directory
void cmd_mkdir(const char* name) {
    // Check if directory already exists
    for (int i = 0; i < current_dir->child_count; i++) {
        if (strcmp(current_dir->children[i]->name, name) == 0) {
            uart_puts("Directory or file already exists: ");
            uart_puts(name);
            uart_puts("\n");
            return;
        }
    }
    
    // Check if we have space for a new directory
    if (current_dir->child_count >= MAX_FILES) {
        uart_puts("Directory is full\n");
        return;
    }
    
    // Create new directory
    FSNode* new_dir = create_node(name, TYPE_DIRECTORY, current_dir);
    current_dir->children[current_dir->child_count++] = new_dir;
    
    uart_puts("Directory created: ");
    uart_puts(name);
    uart_puts("\n");
}

// Create an empty file
void cmd_touch(const char* name) {
    // Check if file already exists
    for (int i = 0; i < current_dir->child_count; i++) {
        if (strcmp(current_dir->children[i]->name, name) == 0) {
            uart_puts("File or directory already exists: ");
            uart_puts(name);
            uart_puts("\n");
            return;
        }
    }
    
    // Check if we have space for a new file
    if (current_dir->child_count >= MAX_FILES) {
        uart_puts("Directory is full\n");
        return;
    }
    
    // Create new file
    FSNode* new_file = create_node(name, TYPE_FILE, current_dir);
    current_dir->children[current_dir->child_count++] = new_file;
    
    uart_puts("File created: ");
    uart_puts(name);
    uart_puts("\n");
}

// View file contents
void cmd_cat(const char* name) {
    // Find the file
    for (int i = 0; i < current_dir->child_count; i++) {
        FSNode* node = current_dir->children[i];
        if (strcmp(node->name, name) == 0) {
            if (node->type == TYPE_DIRECTORY) {
                uart_puts("Cannot display directory content: ");
                uart_puts(name);
                uart_puts("\n");
                return;
            }
            
            // Display file content
            if (node->content_size == 0) {
                uart_puts("(Empty file)\n");
            } else {
                uart_puts(node->content);
                uart_puts("\n");
            }
            return;
        }
    }
    
    uart_puts("File not found: ");
    uart_puts(name);
    uart_puts("\n");
}

// Define VIM editor modes
#define VIM_MODE_NORMAL   0
#define VIM_MODE_INSERT   1
#define VIM_MODE_COMMAND  2

// VIM editor functions
void display_lines(const char* name, char lines[][MAX_LINE_LENGTH], int line_count, int cursor_x, int cursor_y, int mode) {
    // Clear screen
    uart_puts("\033[2J\033[H");
    
    // Display mode
    uart_puts("-- ");
    if (mode == VIM_MODE_NORMAL) {
        uart_puts("NORMAL");
    } else if (mode == VIM_MODE_INSERT) {
        uart_puts("INSERT");
    } else if (mode == VIM_MODE_COMMAND) {
        uart_puts("COMMAND");
    }
    uart_puts(" MODE -- ");
    uart_puts(name);
    uart_puts("\n\n");
    
    // Display line numbers and content
    for (int i = 0; i < line_count; i++) {
        // Line number
        char line_num[5];
        int num = i + 1;
        int pos = 0;
        
        while (num > 0) {
            line_num[pos++] = '0' + (num % 10);
            num /= 10;
        }
        
        if (pos == 0) {
            line_num[pos++] = '0';
        }
        
        line_num[pos] = 0;
        
        // Reverse the digits
        for (int j = 0; j < pos / 2; j++) {
            char temp = line_num[j];
            line_num[j] = line_num[pos - j - 1];
            line_num[pos - j - 1] = temp;
        }
        
        // Display line number
        uart_puts(line_num);
        uart_puts(" ");
        
        // Display line content
        uart_puts(lines[i]);
        uart_puts("\n");
    }
    
    // Position cursor
    if (mode != VIM_MODE_COMMAND) {
        // Move cursor to the right position
        uart_puts("\033[");
        
        // Convert cursor_y + 3 to string (3 is offset for header)
        char y_pos[5];
        int num = cursor_y + 3;
        int pos = 0;
        
        while (num > 0) {
            y_pos[pos++] = '0' + (num % 10);
            num /= 10;
        }
        
        if (pos == 0) {
            y_pos[pos++] = '0';
        }
        
        y_pos[pos] = 0;
        
        // Reverse the digits
        for (int j = 0; j < pos / 2; j++) {
            char temp = y_pos[j];
            y_pos[j] = y_pos[pos - j - 1];
            y_pos[pos - j - 1] = temp;
        }
        
        uart_puts(y_pos);
        uart_puts(";");
        
        // Convert cursor_x + 3 to string (3 is offset for line number)
        char x_pos[5];
        num = cursor_x + 3;
        pos = 0;
        
        while (num > 0) {
            x_pos[pos++] = '0' + (num % 10);
            num /= 10;
        }
        
        if (pos == 0) {
            x_pos[pos++] = '0';
        }
        
        x_pos[pos] = 0;
        
        // Reverse the digits
        for (int j = 0; j < pos / 2; j++) {
            char temp = x_pos[j];
            x_pos[j] = x_pos[pos - j - 1];
            x_pos[pos - j - 1] = temp;
        }
        
        uart_puts(x_pos);
        uart_puts("H");
    }
}

void save_file(FSNode* file, char lines[][MAX_LINE_LENGTH], int line_count) {
    // Clear file content
    file->content_size = 0;
    
    // Save all lines
    for (int i = 0; i < line_count; i++) {
        int len = strlen(lines[i]);
        
        // Check if we have enough space
        if (file->content_size + len + 1 <= file->content_capacity) {
            // Copy line content
            for (int j = 0; j < len; j++) {
                file->content[file->content_size++] = lines[i][j];
            }
            
            // Add newline
            file->content[file->content_size++] = '\n';
        } else {
            // Not enough space
            uart_puts("\nFile too large to save completely!\n");
            return;
        }
    }
    
    // Show save message
    uart_puts("\nFile saved.\n");
}

// Simple vi-like text editor for files
void cmd_edit(const char* name) {
    // Find or create the file
    FSNode* file = NULL;
    
    // Search for existing file
    for (int i = 0; i < current_dir->child_count; i++) {
        if (strcmp(current_dir->children[i]->name, name) == 0) {
            if (current_dir->children[i]->type == TYPE_DIRECTORY) {
                uart_puts("Cannot edit a directory: ");
                uart_puts(name);
                uart_puts("\n");
                return;
            }
            file = current_dir->children[i];
            break;
        }
    }
    
    // Create new file if it doesn't exist
    if (file == NULL) {
        // Check if we have space for a new file
        if (current_dir->child_count >= MAX_FILES) {
            uart_puts("Directory is full\n");
            return;
        }
        
        file = create_node(name, TYPE_FILE, current_dir);
        current_dir->children[current_dir->child_count++] = file;
        uart_puts("New file created: ");
        uart_puts(name);
        uart_puts("\n");
    }
    
    // Clear screen
    uart_puts("\033[2J\033[H");
    
    // Start editing
    uart_puts("VIM-like Editor - ");
    uart_puts(name);
    uart_puts("\n");
    
    // Parse the file content into lines
    const int MAX_LINES = 100;
    
    char lines[MAX_LINES][MAX_LINE_LENGTH];
    int line_count = 0;
    int cursor_x = 0;
    int cursor_y = 0;
    int mode = VIM_MODE_NORMAL;
    
    // Initialize lines
    for (int i = 0; i < MAX_LINES; i++) {
        for (int j = 0; j < MAX_LINE_LENGTH; j++) {
            lines[i][j] = 0;
        }
    }
    
    // Parse existing content
    if (file->content_size > 0) {
        int line_pos = 0;
        
        for (unsigned int i = 0; i < file->content_size; i++) {
            char c = file->content[i];
            
            if (c == '\n') {
                // End of line
                lines[line_count][line_pos] = 0;
                line_count++;
                line_pos = 0;
                
                // Prevent overflow
                if (line_count >= MAX_LINES) {
                    break;
                }
            } else {
                // Add character to current line
                if (line_pos < MAX_LINE_LENGTH - 1) {
                    lines[line_count][line_pos++] = c;
                }
            }
        }
        
        // Handle the last line if it doesn't end with a newline
        if (line_pos > 0) {
            lines[line_count][line_pos] = 0;
            line_count++;
        }
    }
    
    // Ensure at least one line exists
    if (line_count == 0) {
        line_count = 1;
    }
    
    // Display help
    uart_puts("-- VIM-LIKE EDITOR --\n");
    uart_puts("h, j, k, l - Move cursor\n");
    uart_puts("i - Enter insert mode\n");
    uart_puts("ESC - Return to normal mode\n");
    uart_puts(": - Enter command mode\n");
    uart_puts(":w - Save file\n");
    uart_puts(":q - Quit\n");
    uart_puts(":wq - Save and quit\n");
    uart_puts("-------------------------\n\n");
    
    // Main editor loop
    bool running = true;
    display_lines(name, lines, line_count, cursor_x, cursor_y, mode);
    
    while (running) {
        char c = uart_getc();
        
        if (mode == VIM_MODE_NORMAL) {
            // Normal mode key handling
            switch (c) {
                case 'h': // Move left
                    if (cursor_x > 0) {
                        cursor_x--;
                        display_lines(name, lines, line_count, cursor_x, cursor_y, mode);
                    }
                    break;
                
                case 'l': // Move right
                    if (lines[cursor_y][cursor_x] != 0) {
                        cursor_x++;
                        display_lines(name, lines, line_count, cursor_x, cursor_y, mode);
                    }
                    break;
                
                case 'j': // Move down
                    if (cursor_y < line_count - 1) {
                        cursor_y++;
                        // Make sure cursor_x is valid in the new line
                        while (cursor_x > 0 && lines[cursor_y][cursor_x] == 0) {
                            cursor_x--;
                        }
                        display_lines(name, lines, line_count, cursor_x, cursor_y, mode);
                    }
                    break;
                
                case 'k': // Move up
                    if (cursor_y > 0) {
                        cursor_y--;
                        // Make sure cursor_x is valid in the new line
                        while (cursor_x > 0 && lines[cursor_y][cursor_x] == 0) {
                            cursor_x--;
                        }
                        display_lines(name, lines, line_count, cursor_x, cursor_y, mode);
                    }
                    break;
                
                case 'i': // Enter insert mode
                    mode = VIM_MODE_INSERT;
                    display_lines(name, lines, line_count, cursor_x, cursor_y, mode);
                    break;
                
                case ':': // Enter command mode
                    mode = VIM_MODE_COMMAND;
                    uart_puts("\n:");
                    
                    // Buffer for command
                    char cmd_buffer[32];
                    int cmd_pos = 0;
                    
                    while (1) {
                        char cmd_char = uart_getc();
                        
                        if (cmd_char == '\r' || cmd_char == '\n') {
                            cmd_buffer[cmd_pos] = 0;
                            break;
                        } else if (cmd_char == 8 || cmd_char == 127) { // Backspace
                            if (cmd_pos > 0) {
                                cmd_pos--;
                                uart_puts("\b \b"); // Erase character
                            }
                        } else if (cmd_pos < 31) {
                            uart_putc(cmd_char);
                            cmd_buffer[cmd_pos++] = cmd_char;
                        }
                    }
                    
                    // Process command
                    if (strcmp(cmd_buffer, "w") == 0) {
                        save_file(file, lines, line_count);
                        uart_puts("Press any key to continue...");
                        uart_getc();
                        mode = VIM_MODE_NORMAL;
                        display_lines(name, lines, line_count, cursor_x, cursor_y, mode);
                    } else if (strcmp(cmd_buffer, "q") == 0) {
                        running = false;
                    } else if (strcmp(cmd_buffer, "wq") == 0) {
                        save_file(file, lines, line_count);
                        running = false;
                    } else {
                        uart_puts("Unknown command: ");
                        uart_puts(cmd_buffer);
                        uart_puts("\nPress any key to continue...");
                        uart_getc();
                        mode = VIM_MODE_NORMAL;
                        display_lines(name, lines, line_count, cursor_x, cursor_y, mode);
                    }
                    break;
            }
        } else if (mode == VIM_MODE_INSERT) {
            // Insert mode key handling
            if (c == 27) { // ESC key
                mode = VIM_MODE_NORMAL;
                display_lines(name, lines, line_count, cursor_x, cursor_y, mode);
            } else if (c == '\r' || c == '\n') {
                // Create a new line
                if (line_count < MAX_LINES) {
                    // Shift lines down
                    for (int i = line_count; i > cursor_y + 1; i--) {
                        for (int j = 0; j < MAX_LINE_LENGTH; j++) {
                            lines[i][j] = lines[i-1][j];
                        }
                    }
                    
                    // Split current line
                    int split_pos = cursor_x;
                    
                    // Move characters after cursor to new line
                    for (int i = 0; i < MAX_LINE_LENGTH - split_pos; i++) {
                        lines[cursor_y + 1][i] = lines[cursor_y][split_pos + i];
                        
                        if (lines[cursor_y][split_pos + i] == 0) {
                            break;
                        }
                        
                        lines[cursor_y][split_pos + i] = 0;
                    }
                    
                    line_count++;
                    cursor_y++;
                    cursor_x = 0;
                    display_lines(name, lines, line_count, cursor_x, cursor_y, mode);
                }
            } else if (c == 8 || c == 127) { // Backspace
                if (cursor_x > 0) {
                    // Delete character at cursor - 1
                    for (int i = cursor_x - 1; i < MAX_LINE_LENGTH - 1; i++) {
                        lines[cursor_y][i] = lines[cursor_y][i+1];
                    }
                    cursor_x--;
                    display_lines(name, lines, line_count, cursor_x, cursor_y, mode);
                } else if (cursor_y > 0) {
                    // Merge with previous line
                    int prev_line_len = 0;
                    while (lines[cursor_y - 1][prev_line_len] != 0) {
                        prev_line_len++;
                    }
                    
                    // Check if we can merge (won't exceed max line length)
                    int current_line_len = 0;
                    while (lines[cursor_y][current_line_len] != 0) {
                        current_line_len++;
                    }
                    
                    if (prev_line_len + current_line_len < MAX_LINE_LENGTH) {
                        // Append current line to previous line
                        for (int i = 0; i < current_line_len; i++) {
                            lines[cursor_y - 1][prev_line_len + i] = lines[cursor_y][i];
                        }
                        
                        // Shift lines up
                        for (int i = cursor_y; i < line_count - 1; i++) {
                            for (int j = 0; j < MAX_LINE_LENGTH; j++) {
                                lines[i][j] = lines[i+1][j];
                            }
                        }
                        
                        // Update state
                        line_count--;
                        cursor_y--;
                        cursor_x = prev_line_len;
                        display_lines(name, lines, line_count, cursor_x, cursor_y, mode);
                    }
                }
            } else if (cursor_x < MAX_LINE_LENGTH - 1) {
                // Make space for new character
                for (int i = MAX_LINE_LENGTH - 1; i > cursor_x; i--) {
                    lines[cursor_y][i] = lines[cursor_y][i-1];
                }
                
                // Insert character
                lines[cursor_y][cursor_x] = c;
                cursor_x++;
                display_lines(name, lines, line_count, cursor_x, cursor_y, mode);
            }
        }
    }
    
    // Clear screen and return to shell
    uart_puts("\033[2J\033[H");
}

// Remove a file or directory
void cmd_rm(const char* name) {
    // Can't remove special directories
    if (strcmp(name, ".") == 0 || strcmp(name, "..") == 0 || strcmp(name, "/") == 0) {
        uart_puts("Cannot remove special directory\n");
        return;
    }
    
    // Find the file/directory
    int index = -1;
    for (int i = 0; i < current_dir->child_count; i++) {
        if (strcmp(current_dir->children[i]->name, name) == 0) {
            index = i;
            break;
        }
    }
    
    if (index == -1) {
        uart_puts("File or directory not found: ");
        uart_puts(name);
        uart_puts("\n");
        return;
    }
    
    // Check if directory is empty
    FSNode* node = current_dir->children[index];
    if (node->type == TYPE_DIRECTORY && node->child_count > 0) {
        uart_puts("Cannot remove non-empty directory\n");
        return;
    }
    
    // Remove the node
    for (int i = index; i < current_dir->child_count - 1; i++) {
        current_dir->children[i] = current_dir->children[i + 1];
    }
    current_dir->child_count--;
    
    uart_puts("Removed: ");
    uart_puts(name);
    uart_puts("\n");
}

// Get the current working directory path
void cmd_pwd(char* path_buffer) {
    // Start with empty string
    path_buffer[0] = 0;
    
    // Special case for root
    if (current_dir == root_dir) {
        strcpy(path_buffer, "/");
        return;
    }
    
    // Build path by traversing up to root
    FSNode* node = current_dir;
    char temp_path[MAX_PATH];
    for (int i = 0; i < MAX_PATH; i++) temp_path[i] = 0;
    
    while (node != root_dir) {
        // Prepend node name to path
        strcpy(temp_path, "/");
        strcat(temp_path, node->name);
        strcat(temp_path, path_buffer);
        strcpy(path_buffer, temp_path);
        
        // Move up to parent
        node = node->parent;
    }
    
    // If path is empty, we're at root
    if (path_buffer[0] == 0) {
        strcpy(path_buffer, "/");
    }
}

// Exit the kernel
void quit() {
    uart_puts("Shutting down kernel...\n");
    
    // Use ARM semihosting to exit QEMU
    asm volatile(
        "mov r0, #0x18\n"  // SYS_EXIT
        "mov r1, #0x20000\n"  // Successful exit
        "svc 0x123456\n"   // Semihosting SWI
    );
    
    // If semihosting didn't work, try direct QEMU exit
    volatile unsigned int* sysbus = (volatile unsigned int*)0x10000000;
    *sysbus = 0x20026;
    
    // If still here, infinite loop
    while(1);
}

// Tab completion helper - for tab completion feature
void show_matches(char matches[][MAX_NAME], int match_count, FSNode* dir_to_search) {
    // Show all matches
    uart_puts("\n");
    for (int i = 0; i < match_count; i++) {
        if (i > 0 && i % 4 == 0) {
            uart_puts("\n");
        }
        
        // Check if it's a directory
        bool is_dir = false;
        for (int j = 0; j < dir_to_search->child_count; j++) {
            if (strcmp(dir_to_search->children[j]->name, matches[i]) == 0) {
                if (dir_to_search->children[j]->type == TYPE_DIRECTORY) {
                    is_dir = true;
                }
                break;
            }
        }
        
        uart_puts(matches[i]);
        if (is_dir) {
            uart_puts("/");
        }
        
        // Add padding
        int padding = 20 - strlen(matches[i]) - (is_dir ? 1 : 0);
        for (int j = 0; j < padding; j++) {
            uart_putc(' ');
        }
    }
    
    // Show prompt again
    uart_puts("\n");
    cmd_pwd(current_path);
    uart_puts(current_path);
    uart_puts("> ");
}

// Tab completion logic for files and directories
void tab_complete(char* buffer, int* pos) {
    // Save original position
    int orig_pos = *pos;
    
    // Find the start of the word to complete
    int word_start = orig_pos;
    while (word_start > 0 && buffer[word_start - 1] != ' ' && buffer[word_start - 1] != '/') {
        word_start--;
    }
    
    // Extract the prefix to match
    char prefix[MAX_NAME];
    for (int i = 0; i < MAX_NAME; i++) prefix[i] = 0;
    
    for (int i = 0; i < orig_pos - word_start; i++) {
        prefix[i] = buffer[word_start + i];
    }
    
    // Find all nodes that match the prefix
    FSNode* dir_to_search = current_dir;
    
    // Check if we're dealing with a path
    char* last_slash = NULL;
    char path_prefix[MAX_PATH];
    for (int i = 0; i < MAX_PATH; i++) path_prefix[i] = 0;
    
    for (int i = word_start; i < orig_pos; i++) {
        if (buffer[i] == '/') {
            last_slash = &buffer[i];
        }
    }
    
    // If there's a path component, we need to find the directory to search in
    if (last_slash) {
        // Save the original buffer character at the slash position
        char original_char = *(last_slash + 1);
        // Temporarily terminate the string at the slash+1 position
        *(last_slash + 1) = 0;
        
        // Extract the path prefix
        for (int i = word_start; i <= last_slash - &buffer[0]; i++) {
            path_prefix[i - word_start] = buffer[i];
        }
        
        // Restore the original character
        *(last_slash + 1) = original_char;
        
        // Special case for absolute paths
        if (path_prefix[0] == '/') {
            dir_to_search = root_dir;
            
            // Skip the first slash
            char temp_path[MAX_PATH];
            for (int i = 0; i < MAX_PATH; i++) temp_path[i] = 0;
            
            int path_len = 0;
            for (int i = 1; path_prefix[i] != 0; i++) {
                temp_path[path_len++] = path_prefix[i];
            }
            
            // Find each directory in the path
            char* token = strtok(temp_path, "/");
            while (token) {
                // Skip empty tokens
                if (token[0] == 0) {
                    token = strtok(NULL, "/");
                    continue;
                }
                
                // Find the directory
                bool found = false;
                for (int i = 0; i < dir_to_search->child_count; i++) {
                    if (strcmp(dir_to_search->children[i]->name, token) == 0 &&
                        dir_to_search->children[i]->type == TYPE_DIRECTORY) {
                        dir_to_search = dir_to_search->children[i];
                        found = true;
                        break;
                    }
                }
                
                if (!found) {
                    // Path not found, can't complete
                    return;
                }
                
                token = strtok(NULL, "/");
            }
        } else {
            // Relative path, start from current directory
            // This is simplified - would need more work for complex paths
            char* token = strtok(path_prefix, "/");
            while (token) {
                // Find the directory
                bool found = false;
                for (int i = 0; i < dir_to_search->child_count; i++) {
                    if (strcmp(dir_to_search->children[i]->name, token) == 0 &&
                        dir_to_search->children[i]->type == TYPE_DIRECTORY) {
                        dir_to_search = dir_to_search->children[i];
                        found = true;
                        break;
                    }
                }
                
                if (!found) {
                    // Path not found, can't complete
                    return;
                }
                
                token = strtok(NULL, "/");
            }
        }
        
        // Update the prefix to only include the part after the last slash
        for (int i = 0; i < MAX_NAME; i++) prefix[i] = 0;
        
        int new_prefix_len = orig_pos - (last_slash - &buffer[0] + 1);
        for (int i = 0; i < new_prefix_len; i++) {
            prefix[i] = *(last_slash + 1 + i);
        }
    }
    
    // Array to store matches
    char matches[MAX_FILES][MAX_NAME];
    int match_count = 0;
    
    // Find all matching entries
    for (int i = 0; i < dir_to_search->child_count; i++) {
        FSNode* node = dir_to_search->children[i];
        
        // Check if node name starts with prefix
        bool is_match = true;
        for (int j = 0; prefix[j] != 0; j++) {
            if (node->name[j] != prefix[j]) {
                is_match = false;
                break;
            }
        }
        
        if (is_match) {
            // Add to matches
            for (int j = 0; j < MAX_NAME; j++) {
                matches[match_count][j] = node->name[j];
            }
            match_count++;
            
            if (match_count >= MAX_FILES) {
                break;
            }
        }
    }
    
    // If no matches, do nothing
    if (match_count == 0) {
        return;
    }
    
    // If one match, complete it
    if (match_count == 1) {
        // Calculate how much to add
        int to_add = 0;
        while (matches[0][to_add] != 0) {
            to_add++;
        }
        
        // Only add the part that's not already in the buffer
        int prefix_len = 0;
        while (prefix[prefix_len] != 0) {
            prefix_len++;
        }
        
        to_add -= prefix_len;
        
        // Add the completion to the buffer
        for (int i = 0; i < to_add; i++) {
            buffer[orig_pos + i] = matches[0][prefix_len + i];
            uart_putc(matches[0][prefix_len + i]);
        }
        
        // Add directory slash if it's a directory
        for (int i = 0; i < dir_to_search->child_count; i++) {
            if (strcmp(dir_to_search->children[i]->name, matches[0]) == 0) {
                if (dir_to_search->children[i]->type == TYPE_DIRECTORY) {
                    buffer[orig_pos + to_add] = '/';
                    uart_putc('/');
                    to_add++;
                }
                break;
            }
        }
        
        // Update position
        *pos = orig_pos + to_add;
    } else {
        // Multiple matches, find common prefix
        int common_len = 0;
        bool has_common = true;
        
        while (has_common) {
            char c = matches[0][common_len];
            if (c == 0) {
                break;
            }
            
            for (int i = 1; i < match_count; i++) {
                if (matches[i][common_len] != c) {
                    has_common = false;
                    break;
                }
            }
            
            if (has_common) {
                common_len++;
            }
        }
        
        // Only add the part that's not already in the buffer
        int prefix_len = 0;
        while (prefix[prefix_len] != 0) {
            prefix_len++;
        }
        
        int to_add = common_len - prefix_len;
        
        // Add the common prefix
        for (int i = 0; i < to_add; i++) {
            buffer[orig_pos + i] = matches[0][prefix_len + i];
            uart_putc(matches[0][prefix_len + i]);
        }
        
        // Update position
        *pos = orig_pos + to_add;
        
        // Show all matches
        show_matches(matches, match_count, dir_to_search);
    }
}

// Create a test process for demonstration
void test_process_func() {
    // This is just a placeholder for demonstration
    // In a real OS, this would be a real process that does something
    for (int i = 0; i < 1000; i++) {
        // Perform some work
        for (int j = 0; j < 10000; j++) {
            // Busy work
        }
        // Yield to other processes
        process_yield();
    }
}

// Command to run the system monitor
void cmd_monitor() {
    // Initialize and run the system monitor
    monitor_init();
    monitor_update();
    
    // Simple command loop for the monitor
    char cmd[64];
    int cmd_pos = 0;
    char cmd_name[32];
    char cmd_arg[32];
    
    while (1) {
        // Get command
        uart_puts("\nmonitor> ");
        
        // Read command
        cmd_pos = 0;
        for (int i = 0; i < 64; i++) cmd[i] = 0;
        for (int i = 0; i < 32; i++) cmd_name[i] = 0;
        for (int i = 0; i < 32; i++) cmd_arg[i] = 0;
        
        while (1) {
            char c = uart_getc();
            
            if (c == '\r' || c == '\n') {
                uart_puts("\n");
                cmd[cmd_pos] = 0;
                break;
            } else if (c == 8 || c == 127) { // Backspace
                if (cmd_pos > 0) {
                    cmd_pos--;
                    uart_puts("\b \b"); // Erase character
                }
            } else if (cmd_pos < 63) {
                uart_putc(c);
                cmd[cmd_pos++] = c;
            }
        }
        
        // Parse command name and arguments
        int i = 0;
        int arg_start = 0;
        
        // Skip leading spaces
        while (cmd[i] == ' ' && i < cmd_pos) i++;
        
        // Extract command name
        int name_start = i;
        while (cmd[i] != ' ' && cmd[i] != 0) i++;
        int name_end = i;
        
        // Copy command name
        for (i = name_start; i < name_end && (i - name_start) < 31; i++) {
            cmd_name[i - name_start] = cmd[i];
        }
        cmd_name[i - name_start] = 0;
        
        // Skip spaces after command
        while (cmd[i] == ' ' && i < cmd_pos) i++;
        
        // Extract argument (if any)
        arg_start = i;
        while (cmd[i] != 0) i++;
        
        // Copy argument
        for (i = arg_start; i < cmd_pos && (i - arg_start) < 31; i++) {
            cmd_arg[i - arg_start] = cmd[i];
        }
        cmd_arg[i - arg_start] = 0;
        
        // Process command
        if (cmd_pos == 0) {
            monitor_update(); // Refresh display on empty command
            continue;
        }
        
        // Check for exit command
        if (strcmp(cmd_name, "exit") == 0) {
            uart_puts("Exiting monitor...\n");
            break;
        }
        
        // Process all other commands
        monitor_process_command(cmd_name, cmd_arg);
    }
}

// Command to create a test process
void cmd_testproc() {
    int pid = process_create("testproc", test_process_func, 5);
    if (pid >= 0) {
        uart_puts("Created test process with PID ");
        char buf[16];
        int i = 0;
        int tmp = pid;
        if (tmp == 0) {
            buf[i++] = '0';
        } else {
            while (tmp > 0) {
                buf[i++] = '0' + (tmp % 10);
                tmp /= 10;
            }
        }
        buf[i] = 0;
        
        // Print in reverse
        for (int j = i - 1; j >= 0; j--) {
            uart_putc(buf[j]);
        }
        uart_puts("\n");
    } else {
        uart_puts("Failed to create test process\n");
    }
}

// Timer tick simulation function
void simulate_timer_tick() {
    static unsigned int tick_count = 0;
    
    // Update process timer (simulate 10ms timer)
    process_timer_tick(10);
    
    // Schedule processes every 10 ticks (100ms)
    if (tick_count % 10 == 0) {
        process_schedule();
    }
    
    tick_count++;
}

// Simple text-based kernel using only UART for I/O
extern "C" void _start_cpp() {
    // Initialize UART
    uart_init();
    uart_puts("\n\n");
    // Display colorful JasOS logo - using ANSI color codes
    uart_puts("\033[1;36m");  // Bright cyan color
    uart_puts("___        __        ________      ______      ________  \n");
    uart_puts("\033[1;34m");  // Bright blue color
    uart_puts("     |\"  |      /\"\"\\      /\"       )    /    \" \\    /\"       ) \n");
    uart_puts("\033[1;35m");  // Bright magenta color
    uart_puts("     ||  |     /    \\    (:   \\___/    // ____  \\  (:   \\___/  \n");
    uart_puts("\033[1;33m");  // Bright yellow color
    uart_puts("     |:  |    /' /\\  \\    \\___  \\     /  /    ) :)  \\___  \\    \n");
    uart_puts("\033[1;32m");  // Bright green color
    uart_puts("  ___|  /    //  __'  \\    __/  \\\\   (: (____/ //    __/  \\\\   \n");
    uart_puts("\033[1;31m");  // Bright red color
    uart_puts(" /  :|_/ )  /   /  \\\\  \\  /\" \\   :)   \\        /    /\" \\   :)  \n");
    uart_puts("\033[1;37m");  // Bright white color
    uart_puts("(_______/  (___/    \\___)(_______/     \\\"_____/    (_______/   \n");
    uart_puts("                                                               \n");
    uart_puts("\033[0m\n");   // Reset to default color
    
    uart_puts("JasOS Kernel v0.2 (UART ONLY)\n");
    uart_puts("This version uses only UART for I/O.\n");
    uart_puts("The display initialization is skipped.\n\n");
    
    // Initialize memory management
    memory_init();
    use_old_malloc = true; // Use old malloc for initial filesystem setup
    
    // Initialize filesystem
    fs_init();
    
    // Switch to new memory management
    use_old_malloc = false;
    
    // Initialize process management
    process_init();
    
    uart_puts("Starting simple UART shell...\n");
    uart_puts("Type 'help' for available commands.\n");
    
    // Initialize cmd buffer
    char cmd[64];
    for (int i = 0; i < 64; i++) cmd[i] = 0;
    int cmd_pos = 0;
    
    // For parsing commands with arguments
    char cmd_name[32];
    char cmd_arg[32];
    
    // Simple UART shell
    while (1) {
        // Simulate a timer tick (would be hardware interrupt in a real OS)
        simulate_timer_tick();
        
        // Show prompt with current directory
        cmd_pwd(current_path);
        uart_puts(current_path);
        uart_puts("> ");
        
        // Read command
        cmd_pos = 0;
        for (int i = 0; i < 64; i++) cmd[i] = 0;
        for (int i = 0; i < 32; i++) cmd_name[i] = 0;
        for (int i = 0; i < 32; i++) cmd_arg[i] = 0;
        
        while (1) {
            char c = uart_getc();
            
            // Handle special characters
            if (c == '\t') {
                // Tab completion
                tab_complete(cmd, &cmd_pos);
                continue;
            } else if (c == '\r' || c == '\n') {
                uart_puts("\n");
                cmd[cmd_pos] = 0;
                break;
            } else if (c == 8 || c == 127) { // Backspace
                if (cmd_pos > 0) {
                    cmd_pos--;
                    uart_puts("\b \b"); // Erase character
                }
            } else if (cmd_pos < 63) {
                uart_putc(c);
                cmd[cmd_pos++] = c;
            }
        }
        
        // Parse command name and arguments
        int i = 0;
        int arg_start = 0;
        
        // Skip leading spaces
        while (cmd[i] == ' ' && i < cmd_pos) i++;
        
        // Extract command name
        int name_start = i;
        while (cmd[i] != ' ' && cmd[i] != 0) i++;
        int name_end = i;
        
        // Copy command name
        for (i = name_start; i < name_end && (i - name_start) < 31; i++) {
            cmd_name[i - name_start] = cmd[i];
        }
        cmd_name[i - name_start] = 0;
        
        // Skip spaces after command
        while (cmd[i] == ' ' && i < cmd_pos) i++;
        
        // Extract argument (if any)
        arg_start = i;
        while (cmd[i] != 0) i++;
        
        // Copy argument
        for (i = arg_start; i < cmd_pos && (i - arg_start) < 31; i++) {
            cmd_arg[i - arg_start] = cmd[i];
        }
        cmd_arg[i - arg_start] = 0;
        
        // Process command
        if (cmd_pos == 0) {
            continue;
        } else if (strcmp(cmd_name, "help") == 0) {
            uart_puts("Available commands:\n\n");
            
            // System Information Commands
            uart_puts("======== System Information ========\n");
            uart_puts("  help     - Show this help\n");
            uart_puts("  version  - Show kernel version\n");
            uart_puts("  info     - Show system info\n");
            uart_puts("  clear    - Clear screen\n\n");
            
            // File System Commands
            uart_puts("======== File System ========\n");
            uart_puts("  ls       - List directory contents\n");
            uart_puts("  cd <dir> - Change directory\n");
            uart_puts("  pwd      - Print working directory\n");
            uart_puts("  mkdir <n> - Create directory\n");
            uart_puts("  touch <n> - Create empty file\n");
            uart_puts("  cat <file> - Display file contents\n");
            uart_puts("  edit <file> - Edit file\n");
            uart_puts("  rm <n>   - Remove file or directory\n\n");
            
            // System Management Commands
            uart_puts("======== System Management ========\n");
            uart_puts("  monitor  - Start system monitor\n");
            uart_puts("  memdump  - Show memory statistics\n");
            uart_puts("  ps       - List processes\n");
            uart_puts("  testproc - Create a test process\n");
            uart_puts("  kill <pid> - Terminate a process\n");
            uart_puts("  exit     - Quit (halt system)\n");
        } else if (strcmp(cmd_name, "version") == 0) {
            uart_puts("JasOS Kernel v0.2 (UART ONLY)\n");
            uart_puts("Advanced features: Memory Management, Process Management\n");
        } else if (strcmp(cmd_name, "clear") == 0) {
            // Clear screen (ANSI escape sequence)
            uart_puts("\033[2J\033[H");
        } else if (strcmp(cmd_name, "info") == 0) {
            uart_puts("JasOS Kernel Information:\n");
            uart_puts("  Version: 0.2 (UART ONLY)\n");
            uart_puts("  Memory: 256 KB heap\n");
            uart_puts("  Processes: Max 16 processes\n");
            uart_puts("  Filesystem: Simple in-memory filesystem\n");
        } else if (strcmp(cmd_name, "ls") == 0) {
            cmd_ls();
        } else if (strcmp(cmd_name, "cd") == 0) {
            cmd_cd(cmd_arg);
        } else if (strcmp(cmd_name, "pwd") == 0) {
            cmd_pwd(current_path);
            uart_puts(current_path);
            uart_puts("\n");
        } else if (strcmp(cmd_name, "mkdir") == 0) {
            cmd_mkdir(cmd_arg);
        } else if (strcmp(cmd_name, "touch") == 0) {
            cmd_touch(cmd_arg);
        } else if (strcmp(cmd_name, "cat") == 0) {
            cmd_cat(cmd_arg);
        } else if (strcmp(cmd_name, "edit") == 0) {
            cmd_edit(cmd_arg);
        } else if (strcmp(cmd_name, "rm") == 0) {
            cmd_rm(cmd_arg);
        } else if (strcmp(cmd_name, "exit") == 0) {
            quit();
            break;
        } else if (strcmp(cmd_name, "monitor") == 0) {
            cmd_monitor();
        } else if (strcmp(cmd_name, "memdump") == 0) {
            memory_dump();
        } else if (strcmp(cmd_name, "ps") == 0) {
            process_dump();
        } else if (strcmp(cmd_name, "testproc") == 0) {
            cmd_testproc();
        } else if (strcmp(cmd_name, "kill") == 0) {
            // Convert arg to integer
            int pid = 0;
            for (i = 0; cmd_arg[i] >= '0' && cmd_arg[i] <= '9'; i++) {
                pid = pid * 10 + (cmd_arg[i] - '0');
            }
            
            process_terminate(pid);
            uart_puts("Process ");
            for (i = 0; cmd_arg[i] >= '0' && cmd_arg[i] <= '9'; i++) {
                uart_putc(cmd_arg[i]);
            }
            uart_puts(" terminated\n");
        } else {
            uart_puts("Unknown command: ");
            uart_puts(cmd_name);
            uart_puts("\n");
        }
    }
}

// Standard library function implementations
int strcmp(const char* s1, const char* s2) {
    while (*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    return *(const unsigned char*)s1 - *(const unsigned char*)s2;
}

void* memset(void* s, int c, unsigned int n) {
    unsigned char* p = (unsigned char*)s;
    while (n--) {
        *p++ = (unsigned char)c;
    }
    return s;
}

char* strcpy(char* dest, const char* src) {
    char* d = dest;
    while ((*d++ = *src++) != 0);
    return dest;
}

char* strcat(char* dest, const char* src) {
    char* d = dest;
    // Find the end of the destination string
    while (*d) d++;
    // Copy the source string
    while ((*d++ = *src++) != 0);
    return dest;
}

int strlen(const char* str) {
    int len = 0;
    while (str[len]) {
        len++;
    }
    return len;
}

char* strtok(char* str, const char* delim) {
    static char* last_token = NULL;
    
    // If str is NULL, continue from last token
    if (str == NULL) {
        str = last_token;
    }
    
    // Skip leading delimiters
    while (*str != 0) {
        bool is_delim = false;
        for (int i = 0; delim[i] != 0; i++) {
            if (*str == delim[i]) {
                is_delim = true;
                break;
            }
        }
        
        if (!is_delim) {
            break;
        }
        
        str++;
    }
    
    // If we reached the end of the string, return NULL
    if (*str == 0) {
        last_token = str;
        return NULL;
    }
    
    // Find the end of the token
    char* token_start = str;
    while (*str != 0) {
        bool is_delim = false;
        for (int i = 0; delim[i] != 0; i++) {
            if (*str == delim[i]) {
                is_delim = true;
                break;
            }
        }
        
        if (is_delim) {
            *str = 0;
            last_token = str + 1;
            return token_start;
        }
        
        str++;
    }
    
    // We reached the end of the string
    last_token = str;
    return token_start;
}