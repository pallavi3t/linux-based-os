#ifndef PAGING_H
#define PAGING_H

#include "types.h"
#include "x86_desc.h"

#define DIR_ENTRIES     1024 // 4GB/4MB = 2^10 = 1024 
#define DIR_ALIGN       4096 // # of entries x size of entry = 2^10 x 4 bytes = 2^2 x 2^10 = 2^12 = 4096 = 4kB
#define PTBL_ENTRIES    1024 // 4MB/4KB = 2^10 = 1024
#define PTBL_ALIGN      4096 // Each kb page is 4kb
#define PTBL_SIZE       4096 // # of entries x size of entry = 2^10 x 4 bytes = 2^2 x 2^10 = 2^12 = 4096 = 4kB
#define VID_MEM_IDX     0xB8
#define VID_MEM_ADDR    0xB8000     // 753664
#define KERNEL_ADDR     0x400000    // 4194304
#define TERM_1_IDX      0xB9
#define TERM_1_ADDR     0xB9000
#define TERM_2_IDX      0xBA
#define TERM_2_ADDR     0xBA000
#define TERM_3_IDX      0xBB
#define TERM_3_ADDR     0xBB000
#define SET_RW          0x2
#define RW_AND_P        0x3
#define SET_KERNEL_BITS 0x83
#define PTBL_IDX        0
#define KERNEL_IDX      1

void paging_init();
void set_control_reg(uint32_t * pg_dir);

uint32_t pg_tbl[PTBL_ENTRIES] __attribute__ ((aligned (PTBL_ALIGN))); // Array for page table
uint32_t pg_dir[DIR_ENTRIES] __attribute__ ((aligned (DIR_ALIGN))); // Array for page directory
uint32_t user_pg_table[PTBL_ENTRIES] __attribute__ ((aligned (PTBL_ALIGN)));

#endif
