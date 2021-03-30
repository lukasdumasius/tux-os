#include "system_call.h"
#include "paging.h" // added, needed to access page_directory, page_directory_remain, page_table

#define executable_file_magic_num_p0 0x7F
#define executable_file_magic_num_p1 0x45
#define executable_file_magic_num_p2 0x4C
#define executable_file_magic_num_p3 0x46

#define PHYSICAL_VID_MEM_PAGE 0xB8
#define FOUR_KB 4096

// file scope variable
// DEFINITELY NEED TO DOUBLE CHECK HERE AFTER WE HAVE PCB -- Yan 10/29
static int32_t process_total = 0; // The way for us to get current PCB, might have better solution? -- Yan 11/4
// Noet cur_pid = total - 1, total will never be -1 cuz I make sure that in execute
static file_operation_table_t file = {file_read, file_write, file_open, file_close};
static file_operation_table_t direcctory = {dir_read, dir_write, dir_open, dir_close};
static file_operation_table_t std_in = {terminal_read, bad_call, bad_call, bad_call};
static file_operation_table_t std_out = {bad_call, terminal_write, bad_call, bad_call};
static file_operation_table_t rtc = {rtc_virtual_read, rtc_virtual_write, rtc_open, rtc_close};
// REFERENCE: Aamir's supplemental session for bad_call, though never would be invoked
//      Making all static cuz 3.3 Hint suggested, though I think unnecesarry...

// Yan will start on these execute and halt on 10/29
// Adding simple test to check execute & halt -- Yan 11/3

// temp variable for updating the arguments of a process -- Arsam 11/11
static uint8_t temp_args[ARG_LENGTH];

int current_pid;

/* bad_call
 *DESCRIPTION: similar to a plave holder in the file operation table, check above for more details. Normal program would not proceed here?
 *INPUTS: N1
 *OUPTUTS:  N1
 *RETURN VALUE: always -1
 *SIDE EFFECTS: N1
 */
int bad_call()
{
    return -1;
}

/* helper_pass_pid()
 *DESCRIPTION: This is a helper function for handlers to know what the current pid is. Crucial is file_read() and dir_read().
 *OUPTUTS:  N1
 *RETURN VALUE: always (cur_pid + 1)
 *SIDE EFFECTS: N1
 */
int32_t helper_pass_pid()
{
    return process_total;
}

/*
 * scheduler:
 * DESCRIPTION: change the current user process being carried out
 * INPUT: none
 * OUTPUT: updating the paging for video mem in virtual mem, 
 *         changing user program paging
 * RETURN: none
 */
void scheduler()
{
    // update pit count
    pit_count++;
    pit_count = (pit_count % 3);

    // We artificially create 3 terminals if not created
    if (test_count < 3)
    {
        terminal_switch(test_count);
        terminal_switch(0);
        test_count++;
        return;
    }

    pcb_t *old_pcb = (pcb_t *)(MEM_8MB - (current_pid + 1) * MEM_8KB);
    asm volatile("movl %%esp, %0"
                 : "=r"(old_pcb->esp));
    asm volatile("movl %%ebp, %0"
                 : "=r"(old_pcb->ebp));

    // current_pid is the one next scheduled
    current_pid = terminal_process[pit_count];

    // update paging for the current pid's terminal in physcial mem
    if (current_pid == terminal_process[terminal_active_id])
    {
        page_table[PHYSICAL_VID_MEM_PAGE].present = 1;
        page_table[PHYSICAL_VID_MEM_PAGE].read_write = 1;
        page_table[PHYSICAL_VID_MEM_PAGE].user_supervisor = 0;
        page_table[PHYSICAL_VID_MEM_PAGE].page_base_addr = PHYSICAL_VID_MEM_PAGE;
    }
    else
    {
        page_table[PHYSICAL_VID_MEM_PAGE].present = 1;
        page_table[PHYSICAL_VID_MEM_PAGE].read_write = 1;
        page_table[PHYSICAL_VID_MEM_PAGE].user_supervisor = 0;
        page_table[PHYSICAL_VID_MEM_PAGE].page_base_addr = PHYSICAL_VID_MEM_PAGE + ((pit_count + 1) * FOUR_KB);
    }

    // update paging for current pid
    map_paging(current_pid);

    // tss.ss0 = KERNEL_DS;
    tss.esp0 = MEM_8MB - (current_pid)*MEM_8KB - 0x4; // we restore tss's kernel stack ptr to parent process

    // get current pid's pcb
    pcb_t *pcb = (pcb_t *)(MEM_8MB - (current_pid + 1) * MEM_8KB);

    // context switch
    asm volatile(
        "movl %0, %%esp   ;"
        "movl %1, %%ebp   ;"
        :
        : "r"(pcb->esp), "r"(pcb->ebp) // restore to the currently halting process's esp, ebp
        : "memory");                   // clobbered registers
}

/* int32_t halt(uint8_t status)
 * Description: First find the current process's PCB through static process_total, then get parent pid.
 *             Restore to the parent kernel stack, and remap paging to user parent user program.
 *             If parent exits, we return to parent, if not, we re-execute shell
 * Inputs: 
 *      status -- return value of user program, need to pass to execute
 * Outputs: N1
 * Return: To parent process/execute shell again
 */
int32_t halt(uint8_t status)
{
    // E.g. total = 2, we have P0(parent to return to), P1(cur_process), we first get cur PCB
    // pcb_t *pcb = (pcb_t *)(MEM_8MB - (process_total)*MEM_8KB); // 8MB - (i+1) * 8KB
    pcb_t *pcb = (pcb_t *)(MEM_8MB - (current_pid + 1) * MEM_8KB); // 8MB - (i+1) * 8KB

    // Check Appendix E
    tss.ss0 = KERNEL_DS;
    tss.esp0 = MEM_8MB - (pcb->parent_pid) * MEM_8KB - 0x4; // we restore tss's kernel stack ptr to parent process
    // E.g. using the above example, parent_pid should be 0, which is correcct!

    // restore paging for parent
    //      REMEMBER WE still use Virtual Addr at 128MB, but points at parent(above) program's physical addr

    pde_4KB_t *temp = (pde_4KB_t *)(&(page_directory_remain[((208 * (0x100000)) >> 22) - 1])); //*** Added for fish program,
    temp->present = 0;                                                                         //0 b/c dont need  anymore                                             //    lets disable the vidmap page
                                                                                               // (208 * (0x100000)) >> 22) to grab
                                                                                               // top 10 bits of 208MB, -1 bc offset for 'remain'

    map_paging(pcb->parent_pid); // restore paging for the parent process

    // decrease process_count
    process_total--;

    // update current terminal process pid, and current pid
    terminal_process[terminal_active_id] = pcb->parent_pid;
    current_pid = terminal_process[terminal_active_id];

    // Clear all FDs in the currrent process
    // Not necessary since I clear in excute but just a precaution as Discussion 11 suggested -- 11/5
    int i;
    for (i = 0; i < 8; i++)
    {
        if (pcb->fd_array[i].flags == FD_IN_USE)
        {
            pcb->fd_array[i].file_operations_table_ptr.close(i);
        }
        pcb->fd_array[i].flags = FD_FREE;
    }

    // Updated by Yan: 11/15
    // if exeception happened in id_table.c, then set return value to 256 according to shell.c
    // Cannot directly do halt(256) cuz 8-bit limit
    // REFERENCE: https://piazza.com/class/ke1tq32mm6t5rv?cid=997
    uint32_t real_status = (pcb->exception_flag == YES_EXECEPTION) ? 256 : (uint32_t)status;
    pcb->exception_flag = NO_EXCEPTION;

    // Per the hint document requirement:
    //      If we are halting the last process, then start "shell" again
    // if ((terminal_active_id != 0 && (terminal_process[terminal_active_id] == 0)) || (process_total > 0 && (terminal_process[terminal_active_id] == 0)))
    if ((terminal_process[terminal_active_id] < 0))
    {
        // printf("total: %d, process: %d \n", process_total, terminal_process[terminal_active_id]);
        sti();
        execute((uint8_t *)"shell");
    }
    // restore to prev process
    else
    {
        // THIS BELOW STEP IS VERY IMPORTANT BUT VERY HARD TO UNDERSTAND, LMK if you have questions -- 11/4

        // E.g. take the last slide in Discussion 10 as an example, the below code is abput to go throught STEP 8
        //      We want to restore to the execute("ls")'s ESP/EBP(saved in step 4) cuz step 7 modifies ESP/EBP
        //      First we do "LEAVE" to restore to the ESP/EBP cuz C function automatically push EBP
        //          REFERENCE: https://www.tenouk.com/Bufferoverflowc/Bufferoverflow2a.html
        //      Finally we do "RET" to go to STEP9

        // *************************************************************************************
        // My way to understand it is that:
        //      3-10 are a happily married couple: 3 call, 10 return
        //      4-9 are a happily married couple: 4 call, 9 return
        //      5-8 are a happily married couple: 5 call, 8 return
        //      1, 6 will be single forever cuz 1 is running all the time and 6 never need IRET
        // *************************************************************************************

        // REFERENCE: https://wiki.osdev.org/Inline_Assembly
        //      %0, %1, %2 correspondis to the 3 c variables
        //      "r" after second ":" indicates input; "=r" after first ":" indicates output
        // printf("the current return val is %d \n", status);
        asm volatile(
            "movl %0, %%eax   ;"
            "movl %1, %%esp   ;"
            "movl %2, %%ebp   ;"
            "leave               ;"
            "ret                 ;"
            :
            : "r"(real_status), "r"(pcb->esp), "r"(pcb->ebp) // restore to the currently halting process's esp, ebp
            : "eax", "esp", "ebp");                          // clobbered registers
    }
    return 1;
}

/* int32_t execute(const uint8_t *command)
 * Description: First we parse the command, then we validate the command. After that we initialize paging for that executable file.
 *          Next we load the executable file from FS to specified location in memory. Then we initialized the PCB for the current process.
 *          Also need to store relevant info in the current PCB. Remember the fd array should also be initialized in PCB. Finally we do a context
 *          switch to switch to the user program.
 * Inputs: 
 *      command -- executable_image + arguments 
 * Outputs: Only error message
 * Return: 0-255 if success, error if otherwise
 */
int32_t execute(const uint8_t *command)
{
    // printf("reached execute \n");
    // Parse
    // E.g command = "cat test_file.txt", we only want to get "cat" as real_command FOR NOW -- Yan 10/29
    uint8_t real_command[32]; // file has 32 char maximum
    int idx = parse_args(command, (uint8_t *)real_command);

    // cp4 update: new function to get the arguments of the typed command to use in getargs syscall
    // PCB struct was updated for this -- Arsam 11/10
    int i;
    for (i = 0; i < ARG_LENGTH; i++)
    {
        temp_args[i] = 0;
    }
    parse_arguments_of_program(command, idx);

    // Executable check
    //      If real_command existe in FS, if it's executable? Chech below for how and more details
    if (check_file_validity(real_command) != SYS_SUCCESS)
    {
        // !! Check ece391shell.c for exact what error code we should return !! -- 11/5
        return SYS_FAIL;
    }
    // We only allow 6 programs to be running, so compare to 6
    if (process_total == 10)
    {
        printf("You have reached maximum programs allowed by BuffTux, please type 'exit'! \n");
        return 10; // Indicating error code, see shell.c for details, so 10
    }
    // Paging
    // Now real_command is an existed file and validated to be executable, we set up the paging for it
    // Map Virtual (128MB) <=> (8MB) in Physical for shell
    // Note for the ith program(idx i) Virtual (128MB) <=> (8MB + 4MB * i) in Physical
    // USE "INFO MEM" to check for correctness!!! -URW cuz user privilege
    process_total++;
    // increase cur_pid; E.g. we are creating P3, then total should be 4
    map_paging(process_total - 1); // the pid to map to = total -1, need to add -1

    // User-level Program Loader
    // Now we have allocated the paging for the program, need to copy the command file to dedicated virtual memory
    // NOTICE HERE I USE THE RETURN VALUE TO STORE BITS 27:24, MIGHT HAVE BETTER WAY -- Yan 11/4
    uint32_t entry_point_address = user_level_program_loader(real_command);
    // printf("the return entry point addr is: %#x \n", entry_point_address);

    // Create PCB
    // NEED TO IMPLEMENT FD array in PCB, left for you guys to do(read, write), remember first 2 fd are input/output -- Yan 10/30
    pcb_t *cur_PCB = (pcb_t *)(MEM_8MB - (process_total)*MEM_8KB); // 8MB - (i+1) * 8KB E.g. PCB for process 0 starts at 8MB - 8KB

    // Now total is 1, increased before paging
    // IN CP3, since we only have 2 process. Then parent_pid = pid -1 , but not necessarily always the case
    cur_PCB->pid = process_total - 1; // E.g. for 0th shell, pid is should be 0 while total is 1
    // cur_PCB->parent_pid = process_total - 2; // Need to account for negative situation -- Yan 11/3, Fixed in halt Yan 11/4

    cur_PCB->parent_pid = terminal_process[terminal_active_id];
    // updating the terminal process array for the current terminal id
    terminal_process[terminal_active_id] = process_total - 1;

    // updating current pid
    current_pid = terminal_process[terminal_active_id];

    // E.g. we have P0, we are creating P1, now total = 2, then parent PCB for P1 is P0, which should be at (8MB -  1* 8KB)

    // Store current process's stack cuz we need it in the above HALT()
    //      REFERENCE: https://stackoverflow.com/questions/24592879/save-cpu-registers-to-variables-in-gcc
    register int ebp asm("ebp");
    cur_PCB->ebp = ebp;
    register int esp asm("esp");
    cur_PCB->esp = esp;

    // other fields don't matter for std_in && std_out rn
    cur_PCB->fd_array[0].file_operations_table_ptr = std_in;
    cur_PCB->fd_array[0].flags = FD_IN_USE;
    cur_PCB->fd_array[1].file_operations_table_ptr = std_out;
    cur_PCB->fd_array[1].flags = FD_IN_USE;
    cur_PCB->exception_flag = NO_EXCEPTION;

    // All rest fd_array[2:7] entry should be set to available
    for (i = 2; i < 8; i++)
    {
        // other fields don't matter rn
        cur_PCB->fd_array[i].flags = FD_FREE; // not in use
    }

    // cp4 update: updating the args for the current process -- Arsam 11/11
    for (i = 0; i < ARG_LENGTH; i++)
    {
        cur_PCB->args[i] = temp_args[i];
    }
    // process_total++;

    // Context Switch
    // Reference: Appendix E
    tss.ss0 = KERNEL_DS;                                      //kernel stack segmant
    tss.esp0 = MEM_8MB - MEM_8KB * (process_total - 1) - 0x4; //process kernel mode stack E.g. total = 1, then only have P0, then should be bottom of 8MB
    uint32_t user_stack_addr = 132 * (0x100000) - 0x4;        //132MB == 0x08400000 == 132 * (0x100000)

    //check 6.3.4 for details, 6.3.4 in what? :P

    // REFERENCE: https://gcc.gnu.org/onlinedocs/gcc/Extended-Asm.html#Volatile
    // https://stackoverflow.com/questions/3589157/what-is-r-and-double-percent-in-gcc-inline-assembly-language
    //      %0, %1, %2, %3 correspondis to the 4 c variables
    //      "r" after second ":" indicates input; "=r" after first ":" indicates output
    // Check Discussion 10's slide for orders
    asm volatile(
        "pushl   %0          ;"
        "pushl   %1          ;"

        // "pushfl                 ;"
        "pushfl ;"
        "popl %%edx ;"
        "orl $0x0200, %%edx;"
        "pushl %%edx;"

        "pushl   %2          ;"
        "pushl   %3          ;"
        "iret"
        :                                                                            // no output
        : "r"(USER_DS), "r"(user_stack_addr), "r"(USER_CS), "r"(entry_point_address) // "r" indicates input, corresponding to above, check above links for more details -- Yan 11/3
        : "cc", "edx");                                                              // flags might be clobbered

    // Temp retur value, this return value is fake cuz never would reach here, needs double check -- Yan 11/4
    return 1;
}

/* int parse_args(const uint8_t *command, uint8_t *real_command)
 * Description: Given a command, get the part before the 1st space and save to the location pointed by the second argument
 * Inputs: 
 *      command -- given command. E.g. "cat abx.txt"
 *      real_command -- where to write result to. E.g. "cat"
 * Outputs: N1
 * Return: Length of real_command(just for debugging use)
 */
int parse_args(const uint8_t *command, uint8_t *real_command)
{
    int i;
    int index = 0;
    while (command[index] == 32)
    {
        index++;
    }
    for (i = 0; i < 32; i++) // exe file has max 32 char/B
    {
        // NOTE: the below "32" is not length but the ascii # of " " SPACE
        // when see SPACE, "terminate" the string, cuz we already find the command
        real_command[i] = (command[index] != 32) ? command[index] : '\0';
        // return length of real_command, this is for previous single function debugging, not important rn -- Yan 11/4
        if (command[index] == 32 || command[index] == '\0')
            return index;
        index++;
    }
    return index; // i must be 32 here
}

/* int check_file_validity(uint8_t *real_command)
 * Description: Given a command, we check if it exists in FS and if so, if it's executable.
 * Inputs: 
 *      real_command -- the command we get from above parse. E.g. "cat"
 * Outputs: Only error message
 * Return: success if the file exists and executable, fail if otherwise
 * SIDE-NOTE: check below implemented_command_1 part -- Yan 11/3
 */
int check_file_validity(uint8_t *real_command)
{
    // Executable check
    // PSUEDO CODE AS FOLLOWS:
    //      if real_command == NOT_EXIST_IN_FS or read_file(real_command).first_4_bytes != ".ELF": return FALSE  else: continue
    //      ".ELF" cuz only executable start with ".ELF"
    uint8_t magic_buf[4];
    dentry_t file_dentry;
    if (read_dentry_by_name(real_command, &file_dentry) == FS_FAIL)
        return SYS_FAIL;
    if (read_data(file_dentry.inode_num, 0, magic_buf, 4) == FS_FAIL)
    {
        // printf("read first 4 bytes in command file has failed \n");
        return SYS_FAIL;
    }
    // Check for magic number in executable file's header (Appendix C) (.elf)
    if (magic_buf[0] != executable_file_magic_num_p0 || magic_buf[1] != executable_file_magic_num_p1 || magic_buf[2] != executable_file_magic_num_p2 || magic_buf[3] != executable_file_magic_num_p3)
    {
        // printf("This existed file is not executable in TUX_BUFF's FS \n");
        return SYS_FAIL;
    }
    //  !!! THIS IS TO PREVENT UNKNOWN ERROE, CUZ WE HAVEN't D1 THE REST 4 SYSTEM CALSS
    //          E.g. If don't prevent it: in cat, the buf will be NULL and FS will break when opening...
    //          Will implement more boundary check for FS when all SC d1, leave it this way for now -- Yan 11/4
    // int8_t implemented_command_1[] = "ls";
    // int8_t implemented_command_2[] = "testprint";
    // int8_t implemented_command_3[] = "shell";
    /*    if (strncmp((int8_t *)real_command, implemented_command_1, (uint32_t)2) != 0 && strncmp((int8_t *)real_command, implemented_command_2, (uint32_t)9) != 0 && strncmp((int8_t *)real_command, implemented_command_3, (uint32_t)5) != 0)
    {
        printf("TUX_BUFF hasn't implemented some of the system calls in this user program LOL");
        return SYS_FAIL;
    }*/
    return SYS_SUCCESS;
}

/* int map_paging(int pid)
 * Description: We initialize mapping for the user program at 128MB(copied in below function) to physical addr. 
 *          The physical addr is determined by the given pid..
 * Inputs:  pid of the process's memory to map to
 * Outputs: N1
 * Return: always success
 * Side Effects: Remember to FLUSH TLB
 */
// Note: for the ith program(idx i) Virtual (128MB) <=> (8MB + 4MB * i) in Physical
// USE "INFO MEM" to check for correctness!!! URW cuz user privilege
int map_paging(int pid)
{
    uint32_t program_pde_idx = 32; // 128MB = 32 * 4MB(22-bit); another way to verify is that 0x8000000(128MB) << 22 = 32
    // -1 cuz we need idx in the remain, NOT overall idx, check x86_desc.S for details
    page_directory_remain[program_pde_idx - 1].present = 1;
    page_directory_remain[program_pde_idx - 1].read_write = 1;
    page_directory_remain[program_pde_idx - 1].user_supervisor = 1; // 0 for both kernel and video, 1 for user-level
    page_directory_remain[program_pde_idx - 1].page_write_through = 0;
    page_directory_remain[program_pde_idx - 1].page_cashed_disabled = 1; // 0 for only video memory; 1 for program-code/kernel
    page_directory_remain[program_pde_idx - 1].accessed = 0;
    page_directory_remain[program_pde_idx - 1].dirty = 0;
    page_directory_remain[program_pde_idx - 1].page_size = 1;  // indicate 4MB pde
    page_directory_remain[program_pde_idx - 1].global_bit = 0; // set for only kernel
    page_directory_remain[program_pde_idx - 1].available = 0;
    page_directory_remain[program_pde_idx - 1].page_attribute_table = 0;
    page_directory_remain[program_pde_idx - 1].reserved = 0;             // again below -1 cuz we need idx in the remain, NOT overall idx
    page_directory_remain[program_pde_idx - 1].page_base_addr = 2 + pid; // 8MB + 4MB * cur_pid ==> 2 + 1 * cur_pid = 2 + (total-1) = 1 + total
    flush_TLB();                                                         // defined in x86-desc.S -- Yan 10/29
    return SYS_SUCCESS;
}

/* uint32_t user_level_program_loader(uint8_t *real_command)
 * Description: We copy the executable file to memory at 0x8048000 (Appendix C). I added a feature to get bit[27:24] as the 1st line of instruction(Appendix C)
 * Inputs:  
 *      real_command -- the executable filename we get from validation(E.g. "cat")
 * Outputs: N1
 * Return: The addr of 1st line of this user program's instruction -- Just my personal preference, don't have to do it HERE -- Yan 11/3
 */
// My design is that it will return 32-bit starting addr for user program(1st line of instruction)
// Pay attention to bit 24:27
uint32_t user_level_program_loader(uint8_t *real_command)
{
    uint8_t info_buf[4]; // for bits[27:24]
    dentry_t sys_file_dentry;
    // doesn't need file check cuz we already validate real_command before
    read_dentry_by_name(real_command, &sys_file_dentry);
    // We get the bits 24-27, this will the 0th line of intruction of user program
    if (read_data(sys_file_dentry.inode_num, 24, info_buf, 4) == FS_FAIL)
    {
        printf("read first 4 bytes in command file has failed");
        return SYS_FAIL;
    }
    // DEBUGGING INFO, LEAVE it this way rn for future checks -- Yan 11/4
    // bits 27-24 in shell ==> 0x080482E8, indeed near 0x08048000 like document suggested
    // printf("%#x \n", info_buf[0]); // E8
    // printf("%#x \n", info_buf[1]); // 82
    // printf("%#x \n", info_buf[2]); // 04
    // printf("%#x \n", info_buf[3]); // 08
    uint32_t entry_point = (info_buf[3] << 24) + (info_buf[2] << 16) + (info_buf[1] << 8) + (info_buf[0]); // THIS IS THE FIRST INSTRUCTION ADDR, FUTURE USE -- Yan 10/29
    // printf("%#x \n", entry_point);                                                                         // should be 80482E8 for shell

    // Now we copy the command file to the virtual memory 0x8048000
    int32_t temp_res = read_data(sys_file_dentry.inode_num, 0, (void *)0x08048000, 8192); // 8192B = 8KB, I set this large(maximum of our FS) just to make sure we will copy everything
    temp_res += 0;                                                                        // Should be okay cuz I did boundary check in file_read()
    // temp_res should be 0 cuz EOF, only for debugging use -- YAN 10.29					 // 0x08048000 - location of where to copy program image
    // printf("temp result is %d \n", temp_res);
    return entry_point;
}

/* int32_t read(int32_t fd, void *buf, int32_t nbytes)
 * Description: Get the current PCB through PID(total -1), then we find the correspoding function table by the fd. 
 *          And execute the read function specified by that fd's function table.
 * Inputs: 
 *      fd -- file descriptor, assigned by open()
 *      buf -- where to read to
 *      nbytes -- how many bytes to read to 
 * Outputs: N1
 * Return: Consistent with read_return_value in FS/RTC/Terminal
 */
int32_t read(int32_t fd, void *buf, int32_t nbytes)
{
    // DEBUGGING INFO, LEAVE it this way rn for future checks -- Yan 11/4
    // printf("problem is read\n");
    // if (fd == 0)
    // {
    //     // printf("reach terminal read");
    //     return terminal_read(0, buf, nbytes);
    // }
    // return 0;

    pcb_t *pcb = (pcb_t *)(MEM_8MB - (process_total)*MEM_8KB);
    // check for fd
    if ((fd < 0) || (fd > 7) || pcb->fd_array[fd].flags == FD_FREE)
    {
        return SYS_FAIL;
    }
    // 8MB - (i+1) * 8KB E.g. PCB for P0(total=1) starts at 8MB - 8KB

    return pcb->fd_array[fd].file_operations_table_ptr.read(fd, buf, nbytes);
}

/* int32_t write(int32_t fd, void *buf, int32_t nbytes)
 * Description: Get the current PCB through PID(total -1), then we find the correspoding function table by the fd. 
 *          And execute the write function specified by that fd's function table.
 * Inputs: 
 *      fd -- file descriptor, assigned by open()
 *      buf -- where to write to
 *      nbytes -- how many bytes to write to 
 * Outputs: N1
 * Return: Consistent with write_return_value in FS/RTC/Terminal
 */
int32_t write(int32_t fd, const void *buf, int32_t nbytes)
{
    // If execute is correct, should spamming the screen -- Yan 11/2
    //  printf("testing \n");

    // 8MB - (i+1) * 8KB E.g. PCB for P0(total=1) starts at 8MB - 8KB
    pcb_t *pcb = (pcb_t *)(MEM_8MB - (process_total)*MEM_8KB);
    if ((fd < 0) || (fd > 7) || pcb->fd_array[fd].flags == FD_FREE)
    {
        // printf("reached here %d \n", fd);
        return SYS_FAIL;
    }
    // DEBUGGING INFO, LEAVE it this way rn for future checks -- Yan 11/4
    // printf("problem is write\n");
    // printf("cur fd is : %d \n", process_total);
    // printf("cur fd is : %d \n", fd);
    // printf("cur fd is : %u \n", pcb->pid);
    // printf("cur fd is : %u \n", pcb->fd_array[1].flags);
    // if (fd == 1)
    // {
    //     // printf("reach terminal write");
    //     return terminal_write(1, buf, nbytes);
    // }
    // return 0;
    return pcb->fd_array[fd].file_operations_table_ptr.write(fd, buf, nbytes);
}

/* int32_t open(const uint8_t *filename)
 * Description: First we read the file using read_dentry_by_name(). Then we locate the current PCB, next we find an 
 *              available fd in that PCB for the file, we set attributes(inode, offsets, type...). And finally
 *              we open the file using the function ptr table we just assigned.
 * Inputs: 
 *      filename -- the file we need to open 
 * Outputs: Only error message
 * Return: assigned fd number if success, -1 if fail
 */
int32_t open(const uint8_t *filename)
{
    // First check if this file is valid
    // My fav way is to use (name => dentry), but you can use (idx => dentry) as well -- Yan 11/3
    dentry_t open_dentry;
    if (strlen((int8_t *)filename) > 32)
        return SYS_FAIL;
    if (read_dentry_by_name(filename, &open_dentry) == FS_FAIL)
    {
        // printf("this file doesn't exist in our FS \n");
        return SYS_FAIL;
    }

    // Now we try to find an available FD in current PCB
    int i;
    // 8MB - (i+1) * 8KB E.g. PCB for P0(total=1) starts at 8MB - 8KB
    pcb_t *pcb = (pcb_t *)(MEM_8MB - (process_total)*MEM_8KB);
    for (i = 2; i < 8; i++)
    {
        // if 1 fd is available, then mark it in use
        if (pcb->fd_array[i].flags == FD_FREE)
        {
            pcb->fd_array[i].file_position = 0; // Might need to double check for offset? RTC/Terminal? 11/5 Yan
            pcb->fd_array[i].flags = FD_IN_USE;
            pcb->fd_array[i].inode_num = open_dentry.inode_num;
            // Remember types in FS, 0 => RTC, 1 => dir, 2 => regular file
            if (open_dentry.file_type == 0)
                pcb->fd_array[i].file_operations_table_ptr = rtc;
            if (open_dentry.file_type == 1)
            {
                pcb->fd_array[i].file_operations_table_ptr = direcctory;
                pcb->fd_array[i].file_position = 0;
            }
            if (open_dentry.file_type == 2)
            {
                pcb->fd_array[i].file_operations_table_ptr = file;
                pcb->fd_array[i].file_position = 0;
            }
            // open should return the FD assigned if any. (E.g. check ece391ls.c for details.)
            // printf("this is the fd we find for the file %d\n", i);
            pcb->fd_array[i].file_operations_table_ptr.open(filename);
            // printf("cur process: %d, and cur fD is %d \n", process_total - 1, i);
            return i; // i is the fs assigned to this file in the current PCB
        }
    }
    // printf("All FD in use rn \n");
    return SYS_FAIL;
}

/* int32_t close(int32_t fd)
 * Description: Get the current PCB through PID(total -1), then we find the correspoding function table by the fd. 
 *          We clear that fd in the fd_array then call corresponding close(FS/RTC/TERMINAL).
 * Inputs: 
 *      fd -- file descriptor, assigned by open()
 * Outputs: N1
 * Return: Consistent with close_return_value in FS/RTC/Terminal
 */
int32_t close(int32_t fd)
{
    // 8MB - (i+1) * 8KB E.g. PCB for P0(total=1) starts at 8MB - 8KB
    pcb_t *pcb = (pcb_t *)(MEM_8MB - (process_total)*MEM_8KB);

    // HERE fd < 2 cuz we should never allow to close terminal R/W
    //      Check syserr.c test 5 for details 11/9
    if ((fd < 2) || (fd > 7) || pcb->fd_array[fd].flags == FD_FREE)
    {
        return SYS_FAIL;
    }
    // printf("closed fd is %d \n", fd);
    // Mark fd available again
    pcb->fd_array[fd].file_position = 0;
    pcb->fd_array[fd].flags = FD_FREE;
    // Call the close func of that file(fd)
    return pcb->fd_array[fd].file_operations_table_ptr.close(fd);
}

// Requried by CP4
/* getargs(uint8_t *buf, int32_t nbytes)
 * DESCRIPTION: copies the arguments given to the current file in a 
 *              user space buffer
 * INPUTS: user space pointer to a char buffer, size of buffer
 * OUTPUTS: updates the user space buffer accordingly
 * RETURN: 0 for succes, -1 for failure
 */
int32_t getargs(uint8_t *buf, int32_t nbytes)
{
    /* validate the arguments to the function */
    if (buf == NULL)
    {
        return SYS_FAIL;
    }

    /* get the pcb of the current process */
    pcb_t *curr_pcb = (pcb_t *)(MEM_8MB - (process_total)*MEM_8KB);

    /* check if there are any arguments for the command typed */
    /* if there are n1 then just return failure */
    /* no arguments have the first char (index 0) in the buffer as a null value */
    if (curr_pcb->args[0] == '\0')
    {
        return SYS_FAIL;
    }

    /* get the length of the arguments string to avoid calling strlen more than once */
    uint32_t len = strlen((void *)curr_pcb->args);

    /* check if the arguments fit in the buffer provided */
    /* if they don't return failure */
    if (len > nbytes)
    {
        return SYS_FAIL;
    }

    /* copy the arguments into the user space buffer */
    memset(buf, 0, nbytes);
    memcpy(buf, curr_pcb->args, len);

    /* if this step is reached return success */
    return SYS_SUCCESS;
}

/* parse_arguments_of_program(const uint8_t* command, int idx)
 * DESCRIPTION: parses the written command to store arguments given
 * INPUTS: pointer to string which holds the typed command, index 
 *         at which the file name ended
 * OUTPUTS: updates the args string in the pcb of the current process
 * RETURN: 0 by default
 */
int parse_arguments_of_program(const uint8_t *command, int idx)
{
    /* get the pcb of the current process */
    //    pcb_t *curr_pcb = (pcb_t *)(MEM_8MB - (process_total)*MEM_8KB);

    /* local variable to hold index */
    int i;

    /* local variable to hold length of string to avoid calling strlen multiple times */
    int string_length = strlen((void *)command);

    /* parse through the spaces between the file name and the first argument */
    while ((command[idx] == 32) && (idx < string_length))
    {
        idx++;
    }

    /* if there are no arguments, the args string is only a null value */
    /* that is the first char (index 0) in the buffer is a null value */
    if (idx == string_length)
    {
        temp_args[0] = 0;
        return 0;
    }

    /* otherwise, copy the arguments into the the args string */
    for (i = 0, idx; idx < string_length; idx++, i++)
    {
        temp_args[i] = command[idx] != '\n' ? command[idx] : 0;
    }

    /* initialize the rest of the args string to null values */
    //    for (; i < ARG_LENGTH; i++) {
    //        curr_pcb->args[i] = '\0';
    //    }

    /* default return */
    return 0;
}

// we're going to map to
// 208*(0x100000) = 208 MB
/* int32_t vidmap(uint8_t **screen_start)
 * DESCRIPTION: The vidmap all maps the text-mode video memory into user 
 * 				space at a pre-set virtual address.
 * INPUTS: pointer to location to store the new pointer at
 * OUTPUTS: writes  the virtual mem pointer pointing to passed location
 * RETURN: returns the virtual mem pointer pointing to new location ("door to access")of video
 */
int32_t vidmap(uint8_t **screen_start)
{

    uint32_t i;        //used to iterate through page table entries
    uint32_t temp_ptr; // temp ptr for our final return ptr
    uint8_t *temp_ptr_8;
    uint32_t screen_start_as_unsigned = (uint32_t)(screen_start);
    if ((screen_start == NULL))
    {
        return SYS_FAIL;
    }

    if (!((screen_start_as_unsigned > 128 * (0x100000)) && (screen_start_as_unsigned < 132 * (0x100000)))) // greather than 128 * (0x100000)
                                                                                                           // and less than  132 * (0x100000)
                                                                                                           // because we want to check single user (128 MB to 132 MB)
                                                                                                           // page, user shouldnt send any other pointer
    {
        return -1; //return fail if it doesnt fall there, so -1
    }

    //initialize new PT entry
    for (i = 0; i < NUM_PAGE_TABLE_ENTRY; i++) //0 to start iterating at 0
    {
        page_table_2_vidmap[i].present = (i == 0) ? 1 : 0;                       //lets use first spot
        page_table_2_vidmap[i].read_write = (i == 0) ? 1 : 0;                    // explained above
        page_table_2_vidmap[i].user_supervisor = 1;                              // 0 for both kernel and video, 1 for user
        page_table_2_vidmap[i].page_write_through = 0;                           // 0 because unnecesarry
        page_table_2_vidmap[i].page_cashed_disabled = 0;                         // 0 because unnecesarry
        page_table_2_vidmap[i].accessed = 0;                                     // 0 because unnecesarry
        page_table_2_vidmap[i].dirty = 0;                                        // 0 because unnecesarry
        page_table_2_vidmap[i].page_attribute_table = 0;                         // 0 because unnecesarry
        page_table_2_vidmap[i].global_bit = 0;                                   // 0 because unnecesarry
        page_table_2_vidmap[i].available = 0;                                    // 0 because unnecesarry
        page_table_2_vidmap[i].page_base_addr = page_table[0xB8].page_base_addr; // points to same physical video
    }                                                                            // copy as original, so 0xB8

    //update PD entry for above PT
    pde_4KB_t *temp = (pde_4KB_t *)(&(page_directory_remain[((208 * (0x100000)) >> 22) - 1])); //(208 * (0x100000) = 208MB, -1 for offset because 'remain'
    temp->present = 1;                                                                         //1 because now present
    temp->read_write = 1;                                                                      // 1 because necesarry
    temp->page_size = 0;                                                                       //indicate page table following (4kb pages), so 0
    temp->user_supervisor = 1;                                                                 //indicate user accessible, so 1
    temp->page_table_base_addr = ((uint32_t)(&page_table_2_vidmap) >> 12);                     //move by 12 bits to use just top 20

    temp_ptr = 208 * (0x100000); //(208 * (0x100000) = 208MB
    temp_ptr_8 = (uint8_t *)temp_ptr;
    *screen_start = temp_ptr_8;

    flush_TLB();

    return temp_ptr;
}

int32_t set_handler(int32_t signum, void *handler)
{
    return SYS_FAIL;
}
int32_t sigreturn(void)
{
    return SYS_FAIL;
}
