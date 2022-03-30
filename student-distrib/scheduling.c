#include "scheduling.h"

/*
    *   1. counter is executing (pit_init)
    *   2. A pit interrupt occurs --> PIT handler executes --> calls scheduling algorithm (pit_handler)
    *   3. context switch to next process 
    *   4. return from PIT handler and execute fish (a task)
    *   5. another PIT interrupt occurs 
    *   6. cycle continues 
*/


/* PIT FUNCTIONS */
//Source: https://wiki.osdev.org/Programmable_Interval_Timer
//Source: http://www.osdever.net/bkerndev/Docs/pit.htm

/* void pit_init()
 * Input: void
 * Output: void
 * Side effects: initialize the pit by unmasking the pit IRQ and setting frequency 
 */
void pit_init(){

    // cli();

     
    int hz = 10; //set freq every 10 - 50 milliseconds
    current_term = 0;
    int divisor = PIT_DEF_FREQ / hz;                         /* Calculate our divisor */
    outb(PIT_COMMAND_BYTE, PIT_COMMAND_REG);                 /* Set our command byte 0x36 */
    outb(divisor & LOW_DIVISOR_MASK, PIT_CHANNEL0_PORT);     /* Set low byte of divisor */
    outb(divisor >> HIGH_DIVISOR_MASK, PIT_CHANNEL0_PORT);   /* Set high byte of divisor */

    enable_irq(PIT_IRQ);

    // sti();

}

/* void pit_handler()
 * Input: void
 * Output: void
 * Side effects: send end of interrupt for pit
 */
void pit_handler(){

    
    send_eoi(PIT_IRQ);
    cli();
    // check if more than one terminal is running 
    int count_processes = 0;
    if (term[0].active == 1)
    {
        count_processes += 1;
    }
    if (term[1].active == 1)
    {
        count_processes += 1;
    }
    if (term[2].active == 1)
    {
        count_processes += 1;
    }
    if (count_processes > 1)
    {
        scheduler();
    }
    sti();
    

}

/* void scheduler()
 * Input: void
 * Output: void
 * Side effects: schedules processes across the three terminals
 */
void scheduler(){
    //switch ESP/EBP
    //restore next process TSS
    //flush TLB on process switch 
    //update running video coordinates 
    //(save ESP/EBP) for next process
    //--> modify old PCB or create struct to store information 

    // *get pcb*
    uint32_t pcb_ptr = KERNEL_ADDR_8MB - (EIGHT_KB * (get_pid() + ONE_OFFSET));
    pcb_t * pcb = (pcb_t *)pcb_ptr;  
 
    // save ESP/EBP
    asm volatile( "movl %%esp, %0;"
                  "movl %%ebp, %1;"  
                :"=r"(pcb->esp),  "=r"(pcb->ebp) );

    // execute shell in other terminals
    current_term++;


    if (current_term == NUM_TERMS)
    {
        current_term = 0;
    }

    if (term[current_term].active == 0 && (current_term != current_terminal_num))
    {
        term[current_term].active = 1;
        term_switch(current_term);
        execute((uint8_t*)"shell");
    }
    else 
    {
        //switch process paging
        pg_dir[USER_START] = (uint32_t)(EIGHT_MB + (pcb->parent_process_id * FOUR_MB)) | USER_RW_P;
        flush_tlb();
    }
    
    // set TSS
    tss.ss0 = KERNEL_DS;
    tss.esp0 = KERNEL_ADDR_8MB - (EIGHT_KB * pcb->process_id) - SKIP_ELF;

    //update running video coordinates
    uint8_t* screen_start;
    vidmap(&screen_start);

    //restore next process ESP/EBP
    asm volatile(" movl %0, %%esp    \n\
                   movl %1, %%ebp"  
                :
                :"r"(pcb->esp), "r"(pcb->ebp) );    

}

