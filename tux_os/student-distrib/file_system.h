
#ifndef FILE_SYSTEM
#define FILE_SYSTEM

#include "types.h"
#include "lib.h" // get printf
#include "system_call.h"

#define FS_SUCCESS 0
#define FS_FAIL -1
#define BLOCK_SIZE 4096
#define MAX_NUM_FILE 63

// Detailed explanation please refer to Appendix A
//      Can also reference: https://wiki.osdev.org/VFS
typedef struct dentry
{
    uint8_t file_name[32]; // up to 32 char, 0-padded, not including terminal EOS
    uint32_t file_type;    // 0 if file with RTC, 1 if dir, 2 if regular file
    uint32_t inode_num;    // only meaningful fo rregular file, no others
    uint8_t reserved[24];  // 24B reserved
    // 32B + 4B + 4B + 24B = 64B
} dentry_t;

// the 0th block in the file system
typedef struct boot_block
{
    uint32_t num_dir_entry;   // takes 4B
    uint32_t num_inodes;      // N takes 4B
    uint32_t num_data_blocks; // D takes 4B
    uint8_t reserved[52];     // 52B reserved
    dentry_t dir_entries[63]; // 4KB/block && 64B/dentry_t => (4096B - 4B - 4B - 52B) / 64B = 63
    // 4KB for boot_block
} boot_block_t;

typedef struct inodes
{
    uint32_t length_in_bytes;      // takes 4B
    uint32_t data_block_num[1023]; // (4096B - 4B)/4B = 1023
    // 4KB per block
    // !!!!! NOTE: ONLY CERTAIN PART OF data_block_num is meaningful !
    //      E.g. if length_in_bytes = 8192 => file_size = 8KB => only 2 data blocks for this file
    //           => only data_block_num[0] and data_block_num[1] should be used
} inodes_t;

// Easy to access by offset in read_data()
typedef struct data_block
{
    uint8_t data[4096]; // 4KB = 4096B
} data_block_t;

// Helper function
void helper_pass_file_system_addr(uint32_t addr);           // Pass the starting addr of file system from kernel.c to here
int32_t puts_file_name(int8_t *s, uint32_t bytes_to_print); // Output file name (or given bytes), whichever EOL or B_T_P comes first

// operation for files, provided by system module(appendix A)
int32_t read_data(uint32_t inode, uint32_t offset, uint8_t *buf, uint32_t length);
int32_t read_dentry_by_index(uint32_t index, dentry_t *dentry);
int32_t read_dentry_by_name(const uint8_t *name, dentry_t *dentry); // Operation for both file and directory

// File operation, aligned with system_call with ece391_read()...
int32_t file_open(const uint8_t *filename);
int32_t file_close(int32_t fd);
int32_t file_write(int32_t fd, const void *buf, int32_t nbytes);
int32_t file_read(int32_t fd, void *buf, int32_t nbytes);

// Directory operation, aligned with system_call with ece391_read()...
int32_t dir_open(const uint8_t *filename);
int32_t dir_close(int32_t fd);
int32_t dir_write(int32_t fd, const void *buf, int32_t nbytes);
int32_t dir_read(int32_t fd, void *buf, int32_t nbytes);

// Test coverage for MP3.2 (6.2.4): -- Yan 10/24
int32_t print_file_and_size();
uint32_t get_file_size(uint32_t inode);

#endif
