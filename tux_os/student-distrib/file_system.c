#include "file_system.h"

// Yan
// 10/20/2020

// File scope variable
static boot_block_t *starting_addr; // starting address pointing to the file system module

// The below 2 static variables are replaced by FD
// static dentry_t file_dentry;        // current dentry that's being operated on
// static uint32_t cur_file_idx;       // current file that's being operated on in dir_read
// !!! The last 2 needs to be carefully examined for CP3 use -- Yan 10/24

// Helper functions
// ***********************************************************************************************************************
/* helper_pass_file_system_addr
 * 
 * Description: Use a helper function to get the starting address of file system provided to us in kernel
 * Inputs: None
 * Outputs: None
 * Side Effects: None
 */
void helper_pass_file_system_addr(uint32_t addr)
{
    starting_addr = (boot_block_t *)addr; // type casting
}

/* puts_file_name
 * 
 * Description: Use a helper function to print out info for SPECIFIC file system info
 *             This function will print a string pointed by s, and end whenever it reaches
 *             specified number of bytes or end of line.
 * Inputs: s -- pointer to stirng, bytes_to_print --  number of bytes to print
 * Outputs: How many bytes(chars) are put on screen
 * Reference: modified from lib.c
 * Side Effects: None
 */
int32_t puts_file_name(int8_t *s, uint32_t bytes_to_print)
{
    register int32_t index = 0;
    // Note max_len_file_name = 32B
    while (s[index] != '\0' && index < bytes_to_print)
    {
        putc(s[index]);
        index++;
    }
    return index;
}

// Three system module functions
// ***********************************************************************************************************************
/* read_data
 * 
 * Description: From given inode, we read the data from data blocks corresponding to thet inode.
 *              After doing some boundary check, we find the 1st and last block using offset and length
 *              Then we copy the requested data to buf
 * Inputs: 
 *      inode -- contain the file info we need to extract
 *      offset --  where to start reading the data
 *      buf -- where to copy the data to
 *      length -- how many bytes to copy(at most)
 * Outputs: 0 if read EOF, else the number of bytes copied.
 * Side Effects: None
 */
// read data in certain inode from offset and length with length, save to buf
int32_t read_data(uint32_t inode, uint32_t offset, uint8_t *buf, uint32_t length)
{
    // First do some range check
    uint32_t N = starting_addr->num_inodes;
    if (inode >= N)
    {
        printf("Read_data is CORRECT by detecting invalid inode: %u \n", inode);
        return FS_FAIL;
    }
    // Then find the correct index_node by (inode + 1), check Appendix A for reasoning
    // We have inode + 1 blocks, each block take 4KB memory ( 2**12 = 4096 address)
    inodes_t *cur_inode = (inodes_t *)((uint32_t)starting_addr + (inode + 1) * BLOCK_SIZE);

    uint32_t length_data = cur_inode->length_in_bytes;
    if (offset >= length_data)
    {
        // printf("Read_data reach EOF: %u \n", offset);

        // Yan 11/11 Update: this should indicated EOF, not FS_FAIL
        return 0; // 0 indicates reaching EOF
        // return FS_FAIL;
    }

    if (!buf)
    {
        printf("Read_data is CORRECT by detecting null : %u \n", inode);
        return FS_FAIL;
    }

    uint32_t i = 0;
    uint32_t block_num;
    data_block_t *cur_block;
    uint32_t copying_length;
    uint32_t copied_length = 0;

    // Now we need to find the first block when taking offset into consideration
    uint32_t first_block_idx = offset / BLOCK_SIZE;
    uint32_t remaining_offset = offset % BLOCK_SIZE;

    // Now we find the last block using data_length info in the inode
    uint32_t last_block_idx = (offset + length > length_data) ? (length_data / BLOCK_SIZE) : ((offset + length) / BLOCK_SIZE);

    // E.g. if length = 100000000, then we can only copy limited bytes
    length = (offset + length > length_data) ? (length_data - offset) : length;
    uint32_t return_val = length; // Yan 11/11 Update: This should be the actual length read, not 100000000 in the example above

    // Iterate through each copying block
    for (i = first_block_idx; i <= last_block_idx; i++)
    {
        // Get the current data black
        block_num = cur_inode->data_block_num[i];
        cur_block = (data_block_t *)((uint32_t)starting_addr + (N + 1 + block_num) * BLOCK_SIZE);

        // Calcualte how many bytes current copy can take
        copying_length = (length + remaining_offset > BLOCK_SIZE) ? (BLOCK_SIZE - remaining_offset) : length;
        memcpy((uint8_t *)(buf + copied_length), &cur_block->data[remaining_offset], copying_length);

        // Updat the current copied bytes to to-copy(length) bytes
        remaining_offset = 0; //remain_offset should be 0 after 1st iteration
        length -= copying_length;
        copied_length += copying_length;
    }
    // print out what we just read
    // printf("Buf: \n");
    // puts_file_name((int8_t *)buf, copied_length); // !!!! NOTE: Use Yan's modified version of PRINT, see above for reasoning
    // printf("\n");
    return return_val;
}

/* read_dentry_by_index
 * 
 * Description: From given index to the dir_entry in the boot block, we copy the corresponding info
 *              to the second argument dentry
 * Inputs: 
 *      index -- index to the dir_entry in the boot block
 *      dentry -- where we need to copy the info found to
 * Outputs: SUCCESS(0) if copy success, FAIL(-1) if invalid index
 * Side Effects: None
 * Effects: None
 * NOTE: IDX IS THE INDEX IN BOOT BLOCK, NOT INODE IDX
 */
int32_t read_dentry_by_index(uint32_t index, dentry_t *dentry)
{
    // we only have idx from 0-62, check for invalid index
    if (index >= MAX_NUM_FILE || starting_addr->dir_entries[index].file_name[0] == '\0')
    {
        // printf("\nRead_dentry_by_index is CORRECT by detecting invalid idx: %u \n", index);
        return FS_FAIL;
    }
    *dentry = starting_addr->dir_entries[index];
    // printf("idx: %u, filename: %c \n", index, dentry->file_name[0]);
    // printf("File Name is: ");
    // puts_file_name((int8_t *)dentry->file_name, 32); // 32 is the maximum char count of file_name
    // printf(" at index %u \n", index);
    return FS_SUCCESS;
}

/* read_dentry_by_name
 * 
 * Description: From given file_name, we iterate through all files and find the corresponding file
 *              and save the correct info to the second argument if the file exists.
 * Inputs: 
 *      name -- file name to search for in our file system
 *      dentry -- where we need to copy the info found to
 * Outputs: SUCCESS(0) if copy success, FAIL(-1) if invalid/non-existed file name
 * Side Effects: None
 * Effects: None
 */
int32_t read_dentry_by_name(const uint8_t *name, dentry_t *dentry)
{
    uint8_t i = 0;
    // First iterate through file_name in dir_entries in boot_block
    //      We have maximum 63 files include dir_file and reg_file
    if (name == NULL || name[0] == '\0')
        return FS_FAIL;
    for (i = 0; i < MAX_NUM_FILE; i++)
    {
        // Compare string in C: check lib.c
        // We only care about the first 32 bytes, in python "min(32, max(len(name), len(filename)))""
        uint32_t length = (strlen((int8_t *)name) >= strlen((int8_t *)starting_addr->dir_entries[i].file_name)) ? strlen((int8_t *)name) : strlen((int8_t *)starting_addr->dir_entries[i].file_name);
        length = (length > 32) ? 32 : length;

        if (strncmp((int8_t *)name, (int8_t *)starting_addr->dir_entries[i].file_name, length) == 0)
        {
            *dentry = starting_addr->dir_entries[i];
            // printf("CORRECT by finding the right file: ");
            // puts_file_name((int8_t *)name, 32); // 32 is the maximum char count of file_name
            // printf("\n");
            return FS_SUCCESS;
        }
    }
    // If we didn't find the file
    //      printf("CORRECT by not finding non-existed file: ");
    //      puts_file_name((int8_t *)name, 32); // 32 is the maximum char count of file_name
    //      printf("\n");
    return FS_FAIL;
}

// File Operations
//     ALL below 4 functions needs to be further examined(e.g. boundary test) in CP3 -- Yan 10/24
// ***********************************************************************************************************************
/* file_open
 * 
 * Description: Initialize the temporary data structure I have(cur_dentry) by finding the correct file name.
 *              I accomplished that using system module read_dentry_by_name
 * Inputs: 
 *      filename -- file name to search for in our file system
 * Outputs: SUCCESS(0) if copy success, FAIL(-1) if invalid/non-existed file name
 * Side Effects: Copy the correspoding file_info(dentry) to cur_dentry
 * Effects: None
 */
int32_t file_open(const uint8_t *filename)
{
    // return read_dentry_by_name(filename, &file_dentry);
    return FS_SUCCESS; // updated by Yan 11/10 cuz already open in system_call.c
}

/* file_close
 * 
 * Description: Reset the cur_dentry
 * Inputs: 
 *      fd -- TBD in CP3
 * Outputs: SUCCESS(0) always
 * Side Effects: reset cur_dentry
 * Effects: None
 */
int32_t file_close(int32_t fd)
{
    return FS_SUCCESS;
}

/* file_write
 * 
 * Description: Do nothing
 * Inputs: 
 *      fd, buf, nbytes -- TBD in CP3
 * Outputs: FAIL(-1) always
 * Side Effects: None
 * Effects: None
 */
int32_t file_write(int32_t fd, const void *buf, int32_t nbytes)
{
    // Read-onlt file-system, do nothing
    return FS_FAIL;
}

/* file_read
 * 
 * Description: Do basically the same as read_data, except that we "rearrange" the arguments
 *          We got the current file by file_open above. So we know what dentry we are looking
 *          at right now.
 * Inputs: 
 *      fd -- TBD in CP3
 *      buf -- where to copy the data to
 *      nbyets -- how many bytes to read/copy to buf
 * Outputs: SUCCESS(0) if copy success, FAIL(-1) if invalid/non-existed file name
 * Side Effects: None
 * Effects: None
 */
int32_t file_read(int32_t fd, void *buf, int32_t nbytes)
{
    // get filename from file_open and use enrty->inode and system module to read data
    int32_t process_total = helper_pass_pid();
    pcb_t *pcb = (pcb_t *)(MEM_8MB - (process_total)*MEM_8KB);
    // !!!!!! TBD by FD in PCB -- Yan 11/2
    //          Update: fixed Yan 11/10
    //          Still needs working 11/10
    if (buf == NULL)
        return FS_FAIL;
    int32_t return_val = read_data(pcb->fd_array[fd].inode_num, pcb->fd_array[fd].file_position, buf, nbytes);
    // printf("return val in file_read is %d", return_val);
    pcb->fd_array[fd].file_position += return_val;
    return return_val;
}

// Directory Operation
//     ALL below 4 functions needs to be further examined(e.g. boundary test) in CP3 -- Yan 10/24
// ***********************************************************************************************************************

/* dir_open
 * 
 * Description: Initialize the temporary data structure I have(cur_dentry) by finding the correct file name.
 *              I accomplished that by iterating through dentries and find the correct one and save the idx for reading use.
 * Inputs: 
 *      filename -- file name to search for in our file system
 * Outputs: SUCCESS(0) if copy success, FAIL(-1) if invalid/non-existed file name
 * Side Effects: Copy the correspoding file_info(dentry) to cur_dentry and save the filename_idx to cur_file_idx
 * Effects: None
 */
int32_t dir_open(const uint8_t *filename)
{
    // opens the directory file
    // uint8_t i = 0;
    // dentry_t dentry;
    // uint32_t length;
    // // We have maximum 63 files include dir_file and reg_file
    // for (i = 0; i < MAX_NUM_FILE; i++)
    // {
    //     // Compare string in C: check lib.c
    //     // We only care about the first 32 bytes, in python "min(32, max(len(name), len(filename)))""
    //     dentry = starting_addr->dir_entries[i];
    //     length = (strlen((int8_t *)filename) >= strlen((int8_t *)dentry.file_name)) ? strlen((int8_t *)filename) : strlen((int8_t *)dentry.file_name);
    //     length = (length > 32) ? 32 : length; // max filename length is 32
    //     if (strncmp((int8_t *)filename, (int8_t *)dentry.file_name, length) == 0)
    //     {
    //         cur_file_idx = i;
    //         break;
    //     }
    // }
    // // save the file info to cur_dentry
    // return read_dentry_by_name(filename, &file_dentry);
    // cur_file_idx = 0;

    // updated by Yan 11/10 cuz already open in system_call.c
    return FS_SUCCESS;
}

/* file_close
 * 
 * Description: Reset the cur_dentry(cur_file_idx not quite necessary) -- Yan
 * Inputs: 
 *      fd -- TBD in CP3
 * Outputs: SUCCESS(0) always
 * Side Effects: reset cur_dentry
 * Effects: None
 */
int32_t dir_close(int32_t fd)
{
    // cur_file_idx = 0;
    // updated by Yan 11/10 cuz already open in system_call.c
    return FS_SUCCESS;
}

/* file_write
 * 
 * Description: Do nothing
 * Inputs: 
 *      fd, buf, nbytes -- TBD in CP3
 * Outputs: FAIL(-1) always
 * Side Effects: None
 * Effects: None
 */
int32_t dir_write(int32_t fd, const void *buf, int32_t nbytes)
{
    return FS_FAIL;
}

/* dir_read
 * 
 * Description: This would be called multiple times during "ls". In my implementation, 
 *             it will call read_dentry_by_index using the idx we find in dir_open,
 *             and copy the correct structure to cur_dentry.
 * Inputs: 
 *      fd -- TBD in CP3
 *      buf -- where to copy the data to
 *      nbyets -- how many bytes to read/copy to buf
 * Outputs: SUCCESS(0) if file_idx valid, FAIL(-1) if invalid
 * Side Effects: None
 * Effects: None
 */
int32_t dir_read(int32_t fd, void *buf, int32_t nbytes)
{
    if (buf == NULL)
        return FS_FAIL;

    int32_t process_total = helper_pass_pid();
    pcb_t *pcb = (pcb_t *)(MEM_8MB - (process_total)*MEM_8KB);

    dentry_t file_dentry;
    if (read_dentry_by_index(pcb->fd_array[fd].file_position, &file_dentry) == FS_SUCCESS)
    {
        pcb->fd_array[fd].file_position++;
        // copy max(32, len(file_name))
        uint32_t length = (strlen((int8_t *)file_dentry.file_name) > 32) ? 32 : strlen((int8_t *)file_dentry.file_name);
        memcpy((uint8_t *)(buf), &(file_dentry.file_name), length);
        // printf("dir read is successful by opening: ");
        // puts_file_name((int8_t *)buf, 32);
        // printf(" %u ", pcb->fd_array[fd].file_position);

        return length;
    }
    // ls is done, reset to 0, NEED TO BE OPTIMIZED LATER --Yan 11/3
    pcb->fd_array[fd].file_position = 0;
    // printf("\n");
    return 0;
}

// Test coverage for MP3.2 (6.2.4): -- Yan 10/24
// ***********************************************************************************************************************
/* print_file_and_size
 * 
 * Description: This is Yan's version of "ls" program, it iterate through every dentry in the boot block 
 *              in the provided system. If a file exists, I print out its basic info. I also used a helper
 *              function to get the size from dentry's node number
 * Inputs: None
 * Outputs: Files info in the file system
 * Side Effects: None
 */
int32_t print_file_and_size()
{
    uint8_t i = 0;
    dir_open((const uint8_t *)'.');
    // We have maximum 63 files include dir_file and reg_file
    for (i = 0; i < MAX_NUM_FILE; i++)
    {
        // Compare string in C: check lib.c
        // We only care about the first 32 bytes, in python "min(32, max(len(name), len(filename)))""
        // dentry_t dentry = starting_addr->dir_entries[i];
        uint8_t buf[32];
        dir_read(0, &buf, 32);
        // if (file_dentry.file_name[0] != '\0')
        // {
        //     printf("\nType: %u  ", file_dentry.file_type);
        //     printf("Size: %uB      ", get_file_size(file_dentry.inode_num));
        //     puts_file_name((int8_t *)file_dentry.file_name, 32); // 32 is the maximum char count of file_name
        // }
    }
    // printf("\n\n");
    return FS_SUCCESS;
}

/* get_file_size
 * 
 * Description: Helper function for Yan's "ls" program, using the inode idx to get that file's size
 * Inputs: inode -- inode index
 * Outputs: the file size corresponding to that inode
 * Side Effects: None
 */
uint32_t get_file_size(uint32_t inode)
{
    inodes_t *cur_inode = (inodes_t *)((uint32_t)starting_addr + (inode + 1) * BLOCK_SIZE);
    return cur_inode->length_in_bytes;
}
