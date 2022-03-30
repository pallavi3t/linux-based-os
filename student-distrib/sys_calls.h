#ifndef _SYS_CALLS_H
#define _SYS_CALLS_H

#include "file_system.h"
#include "terminal.h"
#include "paging.h"
#include "types.h"
#include "lib.h"
#include "rtc.h"

#define ELF_BYTES               40
#define ELF_STRING_SIZE         3
#define KERNEL_ADDR_8MB         0x800000
#define EIGHT_KB                8192 // 8kB = 2^13 
#define USER_START              32
#define MAX_TASKS               6
#define EIGHT_MB                0x800000
#define FOUR_MB                 0x400000
#define BEG_OFFSET              0
#define PROGRAM_IMAGE           0x08048000
#define INDEX_27                27
#define INDEX_26                26
#define INDEX_25                25
#define INDEX_24                24
#define INDEX_27_OFF            24
#define INDEX_26_OFF            16
#define INDEX_25_OFF            8
#define USER_RW_P               0x87
#define FILE_DESC_TBL_ENTRIES   8
#define NOT_PRESENT             0
#define FILE_DESC_TBL_OFFSET    2
#define STDIN_INDEX             0
#define STDOUT_INDEX            1
#define RTC_TYPE                0
#define DIR_TYPE                1
#define FILE_TYPE               2
#define ONE_OFFSET              1
#define MAX_COMMAND             32
#define MAX_ARGS                128
#define SKIP_ELF                4
#define USER_RW_4KB             0x7
#define USER_RW_AND_P           0x7
#define VID_MAP_IDX             1023
#define USER_TBL_IDX            0
#define VID_MAP_VALUE           0xFFC00000

/*
 * System Calls
 */

/* Halt Sys Call */
int32_t halt(uint8_t status);

/* Execute Sys Call */
int32_t execute(const uint8_t * command);

/* Read Sys Call */
int32_t read(int32_t fd, void * buf, int32_t nbytes);

/* Write Sys Call */
int32_t write(int32_t fd, const void * buf, int32_t nbytes);

/* Open Sys Call */
int32_t open(const uint8_t * filename);

/* Close Sys Call */
int32_t close(int32_t fd);

/* Getargs Sys Call */
int32_t getargs(uint8_t * buf, int32_t nbytes);

/* Vidmap Sys Call */
int32_t vidmap(uint8_t ** screen_start);

/* Set_handler Sys Call */
int32_t set_handler(int32_t signum, void * handler_address);

/* Sigreturn Sys Call */
int32_t sigreturn(void);

/* Invalid Function Call */
int32_t bad_call();



typedef struct fops
{
    int32_t (*open)(const uint8_t * filename);
    int32_t (*read)(int32_t fd, void * buf, int32_t nbytes);
    int32_t (*write)(int32_t fd, const void * buf, int32_t nbytes);
    int32_t (*close)(int32_t fd);
} fops;

typedef struct file_desc_entry_t
{
    uint32_t flags;
    uint32_t file_position;
    uint32_t inode;
    fops * f_ops;
} file_desc_entry_t;

typedef struct pcb_t
{
    uint8_t parent_process_id;
    uint8_t process_id;
    file_desc_entry_t file_desc_table[FILE_DESC_TBL_ENTRIES];
    uint32_t esp;
    uint32_t ebp;
    uint8_t command_arg_buffer[MAX_ARGS];
    uint32_t command_arg_buffer_length;
} pcb_t;


/* 
 * Helper Functions
 */
/* Get the pid */
int32_t get_pid(void);
/* Set up */
pcb_t * setup_pcb();


/* Assembly Functions */
extern void flush_tlb();
extern void context_switch(uint32_t eip);
extern void system_call_handler(void);

#endif 
