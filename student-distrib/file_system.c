#include "file_system.h"

uint32_t cur_dentry_idx = 0;

/*
 * 
 *   DESCRIPTION: Initializes the pointers for the file system
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Sets pointers to point to the start of the file system
 */   
void file_system_init()
{
    bblock_ptr = (struct bblock_t *)module_addr; // Boot block is the first 4KB block of the file system 
    ptr_to_start = module_addr;
}

/*
 * 
 *   DESCRIPTION: Copies a dentry if the name is found
 *   INPUTS: fname - name of file
 *              dentry - dentry to copy to
 *   OUTPUTS: none
 *   RETURN VALUE: 0 if success, -1 if fail
 *   SIDE EFFECTS: 
 */   
int32_t read_dentry_by_name(const uint8_t * fname, dentry_t * dentry)
{
    int i;
    int32_t result = -1;

    for (i = 0; i < BBLOCK_DIR_ENTRIES; i++)
    {
        if (strncmp((int8_t *)(bblock_ptr->dir_entries[i].file_name), (int8_t *)fname, FILE_NAME_SIZE) == 0)
        {
            strcpy((int8_t *)dentry->file_name, (int8_t *)bblock_ptr->dir_entries[i].file_name);
            dentry->file_type = bblock_ptr->dir_entries[i].file_type;
            dentry->inode_num = bblock_ptr->dir_entries[i].inode_num;
            result = 0;
            break;
        }
    }
    return result;
}

/*
 * 
 *   DESCRIPTION: Copies a dentry if the index is valid
 *   INPUTS: index - index of file in dentry array
 *             dentry - dentry to copy to
 *   OUTPUTS: none
 *   RETURN VALUE: 0 if success, -1 if fail
 *   SIDE EFFECTS: 
 */   
int32_t read_dentry_by_index(uint32_t index, dentry_t * dentry)
{
    if (index < 0 || index >= BBLOCK_DIR_ENTRIES)
    {
        return -1;
    }
    strcpy((int8_t *)dentry->file_name, (int8_t *)bblock_ptr->dir_entries[index].file_name);
    dentry->file_type = bblock_ptr->dir_entries[index].file_type;
    dentry->inode_num = bblock_ptr->dir_entries[index].inode_num;
    return 0;
}

/*
 * 
 *   DESCRIPTION: Copies data into a buffer
 *   INPUTS: inode - inode number
 *           offset - position in the file to start reading from (in bytes)
 *           buf - buffer to place read bytes into
 *           length - number of bytes to read up to
 *   OUTPUTS: 
 *   RETURN VALUE: num_bytes_read - number of bytes that were read and placed in buffer
 *                 -1 if invalid 
 *   SIDE EFFECTS: 
 */   
int32_t read_data(uint32_t inode, uint32_t offset, uint8_t * buf, uint32_t length)
{
    /* Check that inode index is valid */
    if (inode < 0 || inode >= bblock_ptr->num_inodes)
    {
        return -1; // Invalid inode 
    }

    int32_t num_bytes_read; // counter to hold number of bytes that have been read 
    unsigned int inode_ptr = ptr_to_start + (inode+BBLOCK_OFFSET)*FOUR_KB; // ptr to inode for file
    uint32_t file_length = ((struct inode_t *)(inode_ptr))->length; // EOF
    
    /* Check that offset is within bounds of file */
    if (offset >= file_length)
    {
        return 0; // Reached EOF, no bytes read 
    }

    uint32_t dblock_num = offset / FOUR_KB; // Original data block index within inode
    uint32_t dblock_byte = offset % FOUR_KB; // Which byte within data block to start at 
    
    for (num_bytes_read = 0; num_bytes_read < length; num_bytes_read++)
    {
        if (num_bytes_read + offset == file_length) // Reached EOF
        {
            return num_bytes_read;
        }

        // ptr + (1 + N + d) * 4KB 
        // N - number of inodes
        // d - index of data block 
        uint32_t d = ((struct inode_t *)(inode_ptr))->data_blocks_num[dblock_num]; // Index of data block 
        // Need to add an offset of which byte within data block to start at and count which byte is currently being read 
        buf[num_bytes_read] = *((uint32_t *)(ptr_to_start + (BBLOCK_OFFSET + N + d)*FOUR_KB + dblock_byte));
        dblock_byte += 1;
        
        if (dblock_byte + offset == FOUR_KB) // Handle edge case where bytes split between data blocks 
        {
            dblock_byte = 0; // Start at beginning of next data block
            dblock_num += 1; // Move to next data block
        }
    } 

    return num_bytes_read;
}

/*
 * 
 *   DESCRIPTION: Reads nbytes bytes of data from the file and places it into buf 
 *   INPUTS: fd: index of into file descriptor table
 *              buf: buffer to copy bytes to
 *              nbytes: number of bytes to read 
 *   OUTPUTS: 
 *   RETURN VALUE: Returns number of bytes that were read 
 *   SIDE EFFECTS: 
 */   
int32_t file_read(int32_t fd, void * buf, int32_t nbytes)
{
    // Check that fd value is within indices 
    if (fd < 0 || fd >= FILE_DESC_TBL_ENTRIES || buf == NULL)
    {
        return -1; // Invalid fd
    }

    uint32_t pcb_ptr = KERNEL_ADDR_8MB - (EIGHT_KB * (get_pid() + ONE_OFFSET));
    pcb_t * pcb = (pcb_t *)pcb_ptr;

    uint32_t bytes_read = read_data(pcb->file_desc_table[fd].inode, pcb->file_desc_table[fd].file_position, buf, nbytes);
    pcb->file_desc_table[fd].file_position += bytes_read;
    if (bytes_read == -1)
    {
        return -1;
    }

    return bytes_read;
}

/*
 * 
 *   DESCRIPTION: Does nothing, system is read-only
 *   INPUTS: 
 *   OUTPUTS: 
 *   RETURN VALUE: Returns -1 
 *   SIDE EFFECTS: 
 */   
int32_t file_write(int32_t fd, const void * buf, int32_t nbytes)
{
    return -1;
}

/*
 * 
 *   DESCRIPTION: Initialize temporary structures
 *   INPUTS: 
 *   OUTPUTS: 
 *   RETURN VALUE: Returns 0
 *   SIDE EFFECTS: 
 */   
int32_t file_open(const uint8_t * filename)
{
    dentry_t dentry;
    strcpy((int8_t *)dentry.file_name, (int8_t *)filename);
    uint32_t result = read_dentry_by_name(filename, &dentry);
    if (result == -1) // Invalid file
    {
        return -1;
    }
    return 0; 
}

/*
 * 
 *   DESCRIPTION: Undo what was done in file_open
 *   INPUTS: 
 *   OUTPUTS: 
 *   RETURN VALUE: Returns 0 
 *   SIDE EFFECTS: 
 */   
int32_t file_close(int32_t fd)
{
    return 0;
}

/*
 * 
 *   DESCRIPTION: Reads files filename by filename, including "."
 *   INPUTS: fd: index of dentry to read 
 *              buf: buffer to copy filename to
 *              nbytes: number of bytes to read 
 *   OUTPUTS: 
 *   RETURN VALUE: Returns -1 if invalid fd or file, returns 0 if success
 *   SIDE EFFECTS: 
 */   
int32_t directory_read(int32_t fd, void * buf, int32_t nbytes)
{
    // Check that fd value is within indices 
    if (fd < 0 || fd >= FILE_DESC_TBL_ENTRIES || buf == NULL)
    {
        return -1; // Invalid fd
    }

    dentry_t dentry;
    int i;
    int32_t result = read_dentry_by_index(cur_dentry_idx, &dentry); // Get dentry by index value
    uint32_t length_of_file = strlen((int8_t *)dentry.file_name); // Length of dentry file name

    if (length_of_file > nbytes)
    {
        length_of_file = nbytes;
    }

    if (result == 0) // Success
    {
        cur_dentry_idx += 1;
        for (i = 0; i < length_of_file; i++)
		{
			((char *)buf)[i] = dentry.file_name[i]; // Copy name of file to buffer 
		} 
    }
    else 
    {
        cur_dentry_idx = 0;
        return -1; // Invalid file 
    }
    
    return i;
}

/*
 * 
 *   DESCRIPTION: Does nothing
 *   INPUTS: 
 *   OUTPUTS: 
 *   RETURN VALUE: Returns -1
 *   SIDE EFFECTS: 
 */   
int32_t directory_write(int32_t fd, const void * buf, int32_t nbytes)
{
    return -1;
}

/*
 * 
 *   DESCRIPTION: Opens a directory file (note file types)
 *   INPUTS: filename: name of file to open 
 *   OUTPUTS: 
 *   RETURN VALUE: Returns -1 if invalid file, returns 0 if success
 *   SIDE EFFECTS: 
 */   
int32_t directory_open(const uint8_t * filename)
{
    dentry_t dentry;
    strcpy((int8_t *)dentry.file_name, (int8_t *)filename);
    uint32_t result = read_dentry_by_name(filename, &dentry);
    if (result == -1) // Invalid file
    {
        return -1;
    }
    cur_dentry_idx = 0;
    return 0; 
}

/*
 * 
 *   DESCRIPTION: Does nothing
 *   INPUTS: 
 *   OUTPUTS: 
 *   RETURN VALUE: Returns 0
 *   SIDE EFFECTS: 
 */   
int32_t directory_close(int32_t fd)
{
    return 0;
}
