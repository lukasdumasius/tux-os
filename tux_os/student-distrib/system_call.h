#ifndef SYSTEM_CALL
#define SYSTEM_CALL

#include "types.h"
#include "lib.h"
#include "file_system.h"
#include "x86_desc.h"
#include "paging.h"
#include "terminal.h"
#include "rtc.h"
#include "pit.h"

// There 2 variables are not reliable, check provided source code in /syscalls for reasons and explanations
#define SYS_SUCCESS 0
#define SYS_FAIL -1

// Constants in system_call.c
#define MEM_8MB 0x800000 // 8MB => 8 *1024*1024 = 8388608 = 0x800000
#define MEM_8KB 0x2000   // 8KB => 8*1024 = 8192 = 0x2000
#define FD_IN_USE 1
#define FD_FREE 0
#define YES_EXECEPTION 1
#define NO_EXCEPTION 0

// CP4 constant for args buffer
#define ARG_LENGTH 128

extern int current_pid;

extern int terminal_process[3];

typedef struct file_operation_table
{
    // REFERENCE: https://stackoverflow.com/questions/9932212/jump-table-examples-in-c
    int32_t (*read)(int32_t fd, void *buf, int32_t nbytes);
    int32_t (*write)(int32_t fd, const void *buf, int32_t nbytes);
    int32_t (*open)(const uint8_t *filename);
    int32_t (*close)(int32_t fd);
} file_operation_table_t;

typedef struct file_descriptor
{
    file_operation_table_t file_operations_table_ptr; // Doesn't use actual ptr cuz want to don't wan to dynamically allocation ptr in ummapped memory
    uint32_t inode_num;                               // get it from dentry, MIGHT EXIST A BETTER WAY ? -- Yan 11/4
    uint32_t file_position;                           // offset in read_data in FS
    uint32_t flags;                                   // 1 is in use, 0 is not
} fd_t;

typedef struct process_control_block
{
    fd_t fd_array[8];
    // Initialize more if needed -- Yan 10/28
    uint32_t pid;
    uint32_t parent_pid; // IN CP3, parent_pid = pid -1 , but not necessarily always the case
    // REFERNECE: https://stackoverflow.com/questions/14824563/struct-inside-of-same-struct-type
    //  CANNOT use abbrevation cuz not initialized yet
    uint32_t esp;
    uint32_t ebp;
    uint8_t args[ARG_LENGTH];
    uint8_t exception_flag;
} pcb_t;

// 10 system call we are setting up
// See function interfaces for more info
int32_t halt(uint8_t status);
int32_t execute(const uint8_t *command);
int32_t read(int32_t fd, void *buf, int32_t nbytes);
int32_t write(int32_t fd, const void *buf, int32_t nbytes);
int32_t open(const uint8_t *filename);
int32_t close(int32_t fd);
int32_t getargs(uint8_t *buf, int32_t nbytes);
int32_t vidmap(uint8_t **screen_start);
int32_t set_handler(int32_t signum, void *handler);
int32_t sigreturn(void);

// helper function for execute
//      LOL I REGRET using this cuz 4 more function headers...
int parse_args(const uint8_t *command, uint8_t *real_command);
int check_file_validity(uint8_t *real_command);
int map_paging(int pid);
uint32_t user_level_program_loader(uint8_t *real_command);

// helper function for parsing the arguments of called program in execute
int parse_arguments_of_program(const uint8_t *command, int idx);

// For invalid operations
// REFERENCE: Aamir's supplemental sessionxs
int bad_call();

void scheduler();

// helper function to pass pid to FS
int32_t helper_pass_pid();

#endif
