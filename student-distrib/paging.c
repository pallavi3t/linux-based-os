#include "paging.h"

/*
 * paging_init
 *   DESCRIPTION: Creates the virtual page table and page directory and enables paging
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Initializes the page directory to hold a page for the kernel and the page table that contains a page for video memory
 */   
void paging_init()
{
    int i;

    /* Initialize the page table consisting of 1024, 4kB pages so that initially each page is not present but RW bit is 1 */
    for (i = 0; i < PTBL_ENTRIES; i++)
    {
        pg_tbl[i] = (i * PTBL_SIZE) | SET_RW;
    }
    // Set the table at video memory index to point to physical video memory address and that RW and present bit are 1
    pg_tbl[VID_MEM_IDX] = VID_MEM_ADDR | RW_AND_P;

    // Add terminal page entries into page table (each page is 4kB)
    pg_tbl[TERM_1_IDX] = TERM_1_ADDR | RW_AND_P;
    pg_tbl[TERM_2_IDX] = TERM_2_ADDR | RW_AND_P;
    pg_tbl[TERM_3_IDX] = TERM_3_ADDR | RW_AND_P;


    /* Initialize the page directory consisting of the page table and 1023 4MB pages so that each page is not present but RW bit is 1*/
    for (i = 0; i < DIR_ENTRIES; i++)
    {
        pg_dir[i] = SET_RW;
    } 
    // Set the first index to be the page table 
    pg_dir[PTBL_IDX] = (uint32_t)pg_tbl | RW_AND_P;
    // Set the second index to be the kernel 
    pg_dir[KERNEL_IDX] = KERNEL_ADDR | SET_KERNEL_BITS;

    // Set the control registers to enable paging 
    set_control_reg(pg_dir);
}
