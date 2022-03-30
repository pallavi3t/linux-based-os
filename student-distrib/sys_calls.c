#include "sys_calls.h"

/* Function Operation Jump Tables */
fops stdin_jt = {bad_call, terminal_read, bad_call, bad_call};
fops stdout_jt = {bad_call, bad_call, terminal_write, bad_call};
fops rtc_jt = {rtc_open, rtc_read, rtc_write, rtc_close};
fops file_jt = {file_open, file_read, file_write, file_close};
fops directory_jt = {directory_open, directory_read, directory_write, directory_close}; 
fops default_jt = {bad_call, bad_call, bad_call, bad_call};

/*
 * Global variables
 */
uint32_t tasks[MAX_TASKS] = {0,0,0,0,0,0};

// process id number of a task 
int32_t pid = -1;

/* int32_t halt(uint8_t status)
 * Input: status value to return 
 * Output: returns a value(status) 0-255 based on process halted 
 * Side effects:  
 */
int32_t halt(uint8_t status)
{
    // halting must respawn a base shell 
    // must return a value to parent execute
    int i;

    uint32_t pcb_ptr = KERNEL_ADDR_8MB - (EIGHT_KB * (pid + ONE_OFFSET));
    pcb_t * pcb = (pcb_t *)pcb_ptr;  

    //restore parent data
    uint32_t parent_pid = pcb->parent_process_id;
    if (pcb->process_id == parent_pid) {
        execute((uint8_t*) "shell");
    }

    // Update pid
    pcb->process_id = 0;
    pid = parent_pid;

    // restore parent paging (same as execute except parent paging info)
    // pg_dir[USER_START] = (EIGHT_MB + ((parent_pid+current_terminal_num) * FOUR_MB)) | USER_RW_P;
    pg_dir[USER_START] = (EIGHT_MB + (parent_pid * FOUR_MB)) | USER_RW_P;
    pg_dir[VID_MAP_IDX] = NULL;

    flush_tlb();

    // write parent process' info to TSS(esp0)
    tss.ss0 = KERNEL_DS;
    tss.esp0 = KERNEL_ADDR_8MB - (EIGHT_KB * parent_pid) - SKIP_ELF; 


    //close the files
    for (i = FILE_DESC_TBL_OFFSET; i < FILE_DESC_TBL_ENTRIES; i++){
        close(i);
        pcb->file_desc_table[i].flags = NOT_PRESENT;
        pcb->file_desc_table[i].inode = NOT_PRESENT;
        pcb->file_desc_table[i].f_ops = NULL;
    }

    // jump to execute return

    asm volatile(" movl %0, %%esp    \n\
                   movl %1, %%ebp    \n\
                   movl %2, %%eax    \n\
                   jmp back_to_execute"
                :
                :"r"(pcb->esp), "r"(pcb->ebp), "r" ((uint32_t)status)
                : "eax" );


    return 0;
}

/* int32_t execute(const uint8_t * command)
 * Input: command to be executed
 * Output: -1 if command cannot be executed, 
 *         256 if program died by exception 
 *         0-255 if program executes halt 
 * Side effects: 
 */
int32_t execute(const uint8_t * command)
{
    // Temporary variables 
    int i;
    int c;
    dentry_t dentry;
    uint8_t elf_buf[ELF_BYTES]; // Read if file contains ELF (indicates executable)
    char elf_correct[ELF_STRING_SIZE] = "ELF";
    uint8_t elf_string[ELF_STRING_SIZE];
    uint32_t length = 0;
    uint32_t length_of_arg = 0;
    uint8_t pname[MAX_COMMAND];
    uint8_t arg_buf[MAX_ARGS];
    uint32_t end_of_command = 0;
    uint32_t end_of_args = 0;
    uint32_t start_of_string = 0;
    uint32_t start_of_arg = 0;
    uint32_t curr_char = 0;
    unsigned int inode_ptr = 0; // Address of inode 
    uint32_t length_of_file = 0; // Length gives number of bytes
    uint32_t first_instruction = 0;
    uint32_t pcb_ptr = 0;
    uint32_t kernel_esp = 0;
    pcb_t * pcb = NULL;
    uint8_t * buf = NULL;

    /*
     * 1. Parse Command
     */ 

    if (command == NULL){
        // empty command
        // printf("command is empty");
        return -1 ;
    }
    
    // Parse command string to get file name of the program to be executed
    length = strlen((int8_t *)command);

    // Skip over spaces if in front of command
    for (c = 0; c < length; c++)
    {
        if (command[c] == ' ')
        {
            start_of_string += 1;
        }
        else
        {
            break;
        }
    }

    // Fill up temporary buffer with newlines 
    for (c = 0; c < MAX_COMMAND; c++)
    {
        pname[c] = '\0';
    }

        for (c = 0; c < MAX_ARGS; c++)
    {
        arg_buf[c] = '\0';
    }

    // Copy command into temporary buffer 
    for (c = start_of_string; c < length; c++)
    {
        if (command[c] == ' ')
        {
            break;
        }
        pname[curr_char] = command[c];
        end_of_command = c;
        curr_char += 1;
    }
    curr_char = 0;
    
    // PARSE THE ARGUMNET / OTHER ARG STUFF
    // later store argument in the PCB

    // Find index of end of argument (if there is one) or will be end of command
    for (c = length-1; c >= end_of_command; c--)
    {
        end_of_args = c;
        if (command[c] != ' ')
        {
            break;
        }
    }

    // Put arguments in buffer
    if (end_of_args != end_of_command)
    {
        for (c = end_of_command + 1; c <= end_of_args; c++)
        {
            start_of_arg = c;
            if (command[c] != ' ')
            {
                break;
            }
        }

        for (c = start_of_arg; c <= end_of_args; c++)
        {
            arg_buf[curr_char] = command[c];
            curr_char += 1;
        }
    }

    /*
     * 2. Check if file exists in file system
     * 2a. Check if file is executable 
     */ 
  
    // Check if file exists in file system
    // dentry_t dentry;
    if (read_dentry_by_name(pname, &dentry) == -1){ // Returns -1 if invalid 
        // printf("file does not exist in file sys");
        return -1; // Invalid 
    }

    // Check if file is executable 
    // uint8_t elf_buf[ELF_BYTES]; // Read if file contains ELF (indicates executable)
    if (read_data(dentry.inode_num, BEG_OFFSET, elf_buf, ELF_BYTES) == -1){ // Returns -1 if invalid 
        // printf("file is not executable");
        return -1; // Invalid 
    }

    // char elf_correct[ELF_STRING_SIZE] = "ELF";
    // uint8_t elf_string[ELF_STRING_SIZE];
    strncpy((int8_t *)elf_string, (int8_t *)elf_buf+1, ELF_STRING_SIZE); // Add 1 to remove special character and just obtain ELF if it is there
    if (strncmp((int8_t *)elf_string, (int8_t *)elf_correct, ELF_STRING_SIZE) == -1) // Returns -1 if not the same 
    {
        // printf("starting bytes not match ELF");
        return -1; // Invalid 
    }

    /*
     * 3. Paging Set-up
     */ 
    
    // create mapping
    // if (pid > 2) // Pid is greater than 0 or the shell 
    // {
    //     pid = 3;
    // }

    int all_tasks_filled = 0;
    //find the pid of our task
    for (i = 0; i < MAX_TASKS; i++){

        if (tasks[i] == 0){
            pid = i;
            all_tasks_filled = 0;
            break;
        }
        all_tasks_filled = 1;
    }

    // if there is no available one return -1, restart shell
    if (all_tasks_filled == 1){
        return -1;
    }

    // mark the process as executing
    tasks[pid] = 1;
    
    if (strncmp((int8_t *)pname, (int8_t*)"shell", length) == 0)
    {
        pid = current_terminal_num;
        // term[current_terminal_num].active = 1;
    }
    // else if (pid >= 3)
    // {
    //     pid = 3;
    // }

    pg_dir[USER_START] = (EIGHT_MB + (pid * FOUR_MB)) | USER_RW_P; 

    //flush tlb: assembly function
    flush_tlb();

    /*
     * 4. User-level Program Loader 
     */ 
    // Get file size 
    inode_ptr = ptr_to_start + (dentry.inode_num+BBLOCK_OFFSET)*FOUR_KB; // Address of inode 
    length_of_file = ((struct inode_t *)(inode_ptr))->length; // Length gives number of bytes
    buf = (uint8_t *)PROGRAM_IMAGE;
    // Get data and store it at 0x08048000 
    read_data(dentry.inode_num, BEG_OFFSET, buf, length_of_file);
    
    // Get address of first instruction
    first_instruction = (elf_buf[INDEX_27] << INDEX_27_OFF) | (elf_buf[INDEX_26] << INDEX_26_OFF) | (elf_buf[INDEX_25] << INDEX_25_OFF) | elf_buf[INDEX_24];
    // Checking first instruction value
    // clear();
    // printf("%d", first_instruction);
    // printf("%d\n", elf_buf[INDEX_27]);
    // printf("%d\n", elf_buf[INDEX_26]);
    // printf("%d\n", elf_buf[INDEX_25]);
    // printf("%d\n", elf_buf[INDEX_24]);

    /*
     * 5. Create PCB
     * 5a. create pcb and put argument in 
     * 5b. file descriptor table
     */ 
    pcb_ptr = KERNEL_ADDR_8MB - (EIGHT_KB * (pid + ONE_OFFSET));
    kernel_esp = KERNEL_ADDR_8MB - (EIGHT_KB * pid) - SKIP_ELF;
    pcb = (struct pcb_t *)pcb_ptr;

    pcb->process_id = pid;

    if (pid == 0) {
        pcb->parent_process_id = 0; // pid 0 is shell for terminal 1
    }
    else if (pid == 1) {
        pcb->parent_process_id = 1; // pid 1 is shell for terminal 2
    }
    else if (pid == 2) {
        pcb->parent_process_id = 2; // pid 2 is shell for terminal 3
    }
    else 
    {
        pcb->parent_process_id = current_terminal_num;
    }

    // Assign index 0 and 1 of file descriptor table to be stdin and stdout 
    pcb->file_desc_table[STDIN_INDEX].f_ops = &stdin_jt;
    pcb->file_desc_table[STDIN_INDEX].inode = NOT_PRESENT;
    pcb->file_desc_table[STDIN_INDEX].file_position = NOT_PRESENT;
    pcb->file_desc_table[STDIN_INDEX].flags = 1;
    
    pcb->file_desc_table[STDOUT_INDEX].f_ops = &stdout_jt;
    pcb->file_desc_table[STDOUT_INDEX].inode = NOT_PRESENT;
    pcb->file_desc_table[STDOUT_INDEX].file_position = NOT_PRESENT;
    pcb->file_desc_table[STDOUT_INDEX].flags = 1;

    for (i = FILE_DESC_TBL_OFFSET; i < FILE_DESC_TBL_ENTRIES; i++)
    {
        pcb->file_desc_table[i].f_ops = &default_jt;
        pcb->file_desc_table[i].inode = NOT_PRESENT;
        pcb->file_desc_table[i].file_position = NOT_PRESENT;
        pcb->file_desc_table[i].flags = 0;
    }

    // Finish parsing arguments 
    // Check if index of end of argument is end of command
    // If not, then copy the arguments from the indices into the argument buffer 
    if (end_of_args != end_of_command)
    {
        length_of_arg = strlen((int8_t *)arg_buf);
        for (i = 0; i < MAX_ARGS; i++)
        {
            pcb->command_arg_buffer[i] = '\0';
        }

        // copy argument buffer into pcb struct
        for (i = 0; i < length_of_arg; i++){
            pcb->command_arg_buffer[i] = arg_buf[i];
        }

        // keep track of length of buffer as well
        pcb->command_arg_buffer_length = end_of_args - start_of_arg + 1;
    }
    else
    {
        // Set the buffer length to 0 (no arguments)
        pcb->command_arg_buffer_length = 0;
    }

    /*
     * 6. Context Switch
     */ 
    // write new process info to TSS
    tss.ss0 = KERNEL_DS;
    tss.esp0 = kernel_esp;

    asm volatile( "movl %%esp, %0;"
                  "movl %%ebp, %1;"  
                :"=r"(pcb->esp),  "=r"(pcb->ebp) );
    
    context_switch(first_instruction);

    asm volatile( 
        "back_to_execute:");

    return 0; // Returns to ASM linkage
}

/* int32_t read(int32_t fd, void * buf, int32_t nbytes)
 * Input: file descriptor, buffer, number of bytes to read
 * Output: -1 on error
 *         return value from file read 
 * Side effects: reads from file
 */
int32_t read(int32_t fd, void * buf, int32_t nbytes)
{
    uint32_t pcb_ptr = KERNEL_ADDR_8MB - (EIGHT_KB * (pid + ONE_OFFSET));
    pcb_t * pcb = (pcb_t *)pcb_ptr;
    
    // Check that fd value is within indices 
    if (fd < 0 || fd >= FILE_DESC_TBL_ENTRIES || buf == NULL || fd == STDOUT_INDEX)
    {
        return -1; // Invalid fd
    }

    // file is not present
    if (pcb->file_desc_table[fd].flags == NOT_PRESENT){
        return -1;
    }

    return pcb->file_desc_table[fd].f_ops->read(fd, buf, nbytes);
}

/* int32_t write(int32_t fd, const void * buf, int32_t nbytes)
 * Input: file descriptor, buffer, number of bytes to write
 * Output: return value from write
 * Side effects: writes to file
 */
int32_t write(int32_t fd, const void * buf, int32_t nbytes)
{
    uint32_t pcb_ptr = KERNEL_ADDR_8MB - (EIGHT_KB * (pid + ONE_OFFSET));
    pcb_t * pcb = (pcb_t *)pcb_ptr;
    
    // Check that fd value is within indices 
    if (fd < 0 || fd >= FILE_DESC_TBL_ENTRIES || buf == NULL || fd == STDIN_INDEX)
    {
        return -1; // Invalid fd
    }

    // file is not present
    if (pcb->file_desc_table[fd].flags == NOT_PRESENT){
        return -1;
    }

    uint32_t num_bytes = pcb->file_desc_table[fd].f_ops->write(fd, buf, nbytes);
    return num_bytes;
}

/* int32_t open(const uint8_t * filename)
 * Input: filename: name of the file to be opened 
 * Output: Returns the file directory index if success, returns -1 if invalid 
 * Side effects: Initializes the pcb at the next available pid entry 
 */ 
int32_t open(const uint8_t * filename)
{
    if (filename[0] == '\0')
    {
        return -1;
    }

    dentry_t dentry;
    uint32_t result = read_dentry_by_name(filename, &dentry);

    if (result == -1)
    {
        return -1;
    }

    int i;
    int fd = -1;
    uint32_t pcb_ptr = KERNEL_ADDR_8MB - (EIGHT_KB * (pid + ONE_OFFSET));
    pcb_t * pcb = (pcb_t *)pcb_ptr;

    // pcb_t* pcb = setup_pcb();

    
    // Find unused file descriptor
    for (i = FILE_DESC_TBL_OFFSET; i < FILE_DESC_TBL_ENTRIES; i++)
    {
        if (pcb->file_desc_table[i].flags == 0) 
        {
            // find file type for fops
            if (dentry.file_type == RTC_TYPE) // RTC
            {
                pcb->file_desc_table[i].f_ops = &rtc_jt;
            }
            else if (dentry.file_type == DIR_TYPE) // Directory
            {
                pcb->file_desc_table[i].f_ops = &directory_jt;
            }
            else if (dentry.file_type == FILE_TYPE) // File
            {
                pcb->file_desc_table[i].f_ops = &file_jt;
            }
            
            pcb->file_desc_table[i].inode = dentry.inode_num;
            pcb->file_desc_table[i].file_position = 0;
            pcb->file_desc_table[i].flags = 1;
            fd = i;
            break;
        }
    }

    if (fd == STDOUT_INDEX || fd == STDIN_INDEX){
        return -1;
    }
    if (fd != -1)
    {
        pcb->file_desc_table[fd].f_ops->open(filename);
    }

    return fd;
}

/* int32_t close(int32_t fd)
 * Input: file descriptor to close
 * Output: -1 on error
 *          0 on success
 * Side effects: closes desired file 
 */
int32_t close(int32_t fd)
{
    uint32_t pcb_ptr = KERNEL_ADDR_8MB - (EIGHT_KB * (pid + ONE_OFFSET));
    pcb_t * pcb = (pcb_t *)pcb_ptr;
    
    // Check that fd value is within indices 
    if (fd < FILE_DESC_TBL_OFFSET  || fd >= FILE_DESC_TBL_ENTRIES)
    {
        return -1; // Invalid fd
    }

    if (pcb->file_desc_table[fd].flags == 0){
        return -1;
    }

    pcb->file_desc_table[fd].f_ops->close(fd);
    pcb->file_desc_table[fd].flags = 0;
    return 0; 
}

// Checkpoint 4
/* int32_t getargs(uint8_t * buf, int32_t nbytes)
 * Input: buf- user level buffer to copy command line args into
 *        nbytes- number of bytes to copy into buffer
 * Output: return 0 on success, -1 on failure
 * Side effects: modify buffer
 */
int32_t getargs(uint8_t * buf, int32_t nbytes)
{
    // Get pcb pointer
    uint32_t pcb_ptr = KERNEL_ADDR_8MB - (EIGHT_KB * (pid + ONE_OFFSET));
    pcb_t * pcb = (pcb_t *)pcb_ptr;

    // Check buffer is pointing to valid memory
    if (buf == NULL || pcb->command_arg_buffer_length > nbytes || pcb->command_arg_buffer_length == 0){
        return -1;
    }

    // copy command line args into user level buffer
    strncpy((int8_t*)buf, (int8_t*)pcb->command_arg_buffer, nbytes);

    return 0;
}

// Checkpoint 4
/* int32_t vidmap(uint8_t ** screen_start)
 * Input: 
 * Output: 
 * Side effects:  
 */
int32_t vidmap(uint8_t ** screen_start)
{
    // Check for valid screen_start ptr
    // return -1 if invalid
    if (screen_start == NULL || screen_start < (uint8_t **)EIGHT_MB){
        return -1;
    }
    
    // Map video memory into user space
    user_pg_table[USER_TBL_IDX] = VID_MEM_ADDR | USER_RW_AND_P;
    pg_dir[VID_MAP_IDX] = (uint32_t) user_pg_table | USER_RW_4KB;
    flush_tlb();

    *screen_start = (uint8_t *)VID_MAP_VALUE;

    return VID_MEM_ADDR;
}

// Extra Credit
/* int32_t set_handler(int32_t signum, void * handler_address)
 * Input: 
 * Output: 
 * Side effects:  
 */
int32_t set_handler(int32_t signum, void * handler_address)
{
    return 0;
}

// Extra Credit 
/* int32_t sigreturn(void)
 * Input: 
 * Output: 
 * Side effects:  
 */
int32_t sigreturn(void)
{
    return 0;
}

/* int32_t bad_call(void)
 * Input: None
 * Output: -1 
 * Side effects:  None
 */
int32_t bad_call()
{
    return -1;
}

/* int32_t get_pid(void)
 * Input: None
 * Output:pid 
 * Side effects:  getter function to return the pid
 */
int32_t get_pid(void)
{
    return pid;
}

/* int32_t setup_pcb()
 * Input: None
 * Output:pid 
 * Side effects:  Helper function to setup pcb since execute is not working
 */
pcb_t* setup_pcb()
{
    uint32_t pcb_ptr = KERNEL_ADDR_8MB - (EIGHT_KB * (pid + ONE_OFFSET));
    pcb_t * pcb = (pcb_t *)pcb_ptr;
    pcb->process_id = pid;
    int i;

    pcb->file_desc_table[STDIN_INDEX].f_ops = &stdin_jt;
    pcb->file_desc_table[STDIN_INDEX].inode = NOT_PRESENT;
    pcb->file_desc_table[STDIN_INDEX].file_position = NOT_PRESENT;
    pcb->file_desc_table[STDIN_INDEX].flags = 1;
    
    pcb->file_desc_table[STDOUT_INDEX].f_ops = &stdout_jt;
    pcb->file_desc_table[STDOUT_INDEX].inode = NOT_PRESENT;
    pcb->file_desc_table[STDOUT_INDEX].file_position = NOT_PRESENT;
    pcb->file_desc_table[STDOUT_INDEX].flags = 1;

    for (i = FILE_DESC_TBL_OFFSET; i < FILE_DESC_TBL_ENTRIES; i++)
    {
        pcb->file_desc_table[i].f_ops = &default_jt;
        pcb->file_desc_table[i].inode = NOT_PRESENT;
        pcb->file_desc_table[i].file_position = NOT_PRESENT;
        pcb->file_desc_table[i].flags = 0;
    }

    return pcb;
}

