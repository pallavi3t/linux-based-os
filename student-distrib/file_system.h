#ifndef FILE_SYSTEM_H
#define FILE_SYSTEM_H

/*

File System: Regions of 4KB blocks
Block Types: 
    - Boot Block: Only one, first 64B contains metadata about the whole file system
    - Inode: tells you where the data is
    - Data block: stores the data in the file 
        - Inodes + data blocks tells you information about the content of the file

Boot Block Structure (4KB, divided into 64B blocks)
    # Dir. Entries (4B)
    # Inodes N (4B)
    # Data Blocks D (4B)
    52B Reserved
    64B Dir. Entries/Dentries

Directory Entries/Dentry Structure: Contains metadata about each file
    File Name (32B)
    File Type (4B)
    Inode # (4B)
    24B Reserved

Inode Structure (4KB)
    Length in B (4B) - Length % 4kB = # of data blocks 
    0th Data Block # (4B)
    1st Data Block # (4B)
    etc.

Given data block index d, N Inodes, ptr to beginning of file system -> moves ptr to index d
    ptr + (1 + N + d) * 4KB

# of Files in File System:
    Limiting factor is space in boot block: each file -> 1 dentry
        - 4KB (size of boot block)/64B (size of each dentry) = (2^2 x 2^10)/2^6 = 2^6 = 64
        - 64 - 1 (first 64B for metadata about file system) = 63 free 64B blocks to store info for files
        - 1 file ".": 62 -> file

Maximum Size of a File:
    1 Inode per file
    How many data blocks can 1 file have?
        - 4KB (size of Inode)/4B (size of data block) = 1024
        - 1024 - 1 (1 4B for length) = 1023 data blocks
    1024 x 4KB = 2^10 x 2^2 x 2^10 = 2^22 B
    
    Length - 32 bits - 2^32 B

    Take min(# data block x data block size, max length size)

Total Size of the File System:
    (D (# of data blocks) + N (# of Inodes) + 1 (Boot Block)) * 4KB

Max Size of File System:
    All files present + all files have max file size 
    max size = (max D + max N + 1) * 4KB
        max N = 63 

Start address of the module is where the filesystem image will be

*/

#include "types.h"
#include "lib.h"
#include "sys_calls.h"

/* Constants */
#define FILE_NAME_SIZE      32
#define DENTRY_RESERVED     24
#define BBLOCK_RESERVED     52
#define BBLOCK_DIR_ENTRIES  63
#define NUM_DATA_BLOCKS     1023
#define FOUR_KB             4096
#define BBLOCK_OFFSET       1
#define N                   64
#define ONE_KB              1024

/* Directory Entry/Dentry Structure */
typedef struct dentry_t
{
    unsigned char file_name[FILE_NAME_SIZE];
    uint32_t file_type; // 0 for user-level access to RTC, 1 for directory, 2 for regular file 
    uint32_t inode_num; // Meaningful for regular files, ignored for RTC and directory
    uint8_t reserved[DENTRY_RESERVED];
} dentry_t;

/* INode Structure */
typedef struct inode_t
{
    uint32_t length; // File's size in bytes
    uint32_t data_blocks_num[NUM_DATA_BLOCKS]; // Data blocks that make up the file 
} inode_t;

/* Boot Block Structure */
typedef struct bblock_t
{
    uint32_t num_dir_entries;
    uint32_t num_inodes;
    uint32_t num_data_blocks;
    uint8_t reserved[BBLOCK_RESERVED];
    dentry_t dir_entries[BBLOCK_DIR_ENTRIES];
} bblock_t;

void file_system_init();

/* File System Routines */
int32_t read_dentry_by_name(const uint8_t * fname, dentry_t * dentry);
int32_t read_dentry_by_index(uint32_t index, dentry_t * dentry);
int32_t read_data(uint32_t inode, uint32_t offset, uint8_t * buf, uint32_t length);

/* File operations */
int32_t file_read(int32_t fd, void * buf, int32_t nbytes);
int32_t file_write(int32_t fd, const void * buf, int32_t nbytes);
int32_t file_open(const uint8_t * filename);
int32_t file_close(int32_t fd);

/* Directory operations */
int32_t directory_read(int32_t fd, void * buf, int32_t nbytes); 
int32_t directory_write(int32_t fd, const void * buf, int32_t nbytes);
int32_t directory_open(const uint8_t * filename);
int32_t directory_close(int32_t fd);

/* Global Variables */
struct bblock_t * bblock_ptr; 
unsigned int ptr_to_start;
unsigned int module_addr;

#endif
