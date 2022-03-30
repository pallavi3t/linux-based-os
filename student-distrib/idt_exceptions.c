#include "lib.h"
#include "idt_exceptions.h"


/* divide_by_zero_error(void)
 * INPUT: void
 * OUTPUT: none
 * FUNCTION: print error info for divide by zero
 *
*/
void divide_by_zero_error(void)
{
    clear();
    printf("EXCEPTION: Divide by Zero");
    while (1);

}

/* debug_exception(void)
 * INPUT: void
 * OUTPUT: none
 * FUNCTION: print error info for debug exception
 *
*/
void debug_exception(void)
{
    clear();
    printf("EXCEPTION: DEBUG");
    while (1);

}

/* non_maskable_interrupt(void)
 * INPUT: void
 * OUTPUT: none
 * FUNCTION: print error info for NMI Interrupt
 *
*/
void non_maskable_interrupt(void)
{
    clear();
    printf("EXCEPTION: NMI Interrupt");
    while (1);

}

/* breakpoint_exception(void)
 * INPUT: void
 * OUTPUT: none
 * FUNCTION: print error info for breakpoint exception
 *
*/
void breakpoint_exception(void)
{
    clear();
    printf("EXCEPTION: Breakpoint");
    while (1);

}

/* overflow_exception(void)
 * INPUT: void
 * OUTPUT: none
 * FUNCTION: print error info for overflow exception
 *
*/
void overflow_exception(void)
{
    clear();
    printf("EXCEPTION: Overflow");
    while (1);

}

/* bound_range_exceeded_exception(void)
 * INPUT: void
 * OUTPUT: none
 * FUNCTION: print error info for bound range exceeded error
 *
*/
void bound_range_exceeded_exception(void)
{
    clear();
    printf("EXCEPTION: BOUND Range Exceeded");
    while (1);

}

/* invalid_opcode_exception(void)
 * INPUT: void
 * OUTPUT: none
 * FUNCTION: print error info for invalid opcode
 *
*/
void invalid_opcode_exception(void)
{
    clear();
    printf("EXCEPTION: Invalid Opcode");
    while (1);

}

/* device_not_available(void)
 * INPUT: void
 * OUTPUT: none
 * FUNCTION: print error info for device not available
 *
*/
void device_not_available(void)
{
    clear();
    printf("EXCEPTION: Device Not Available");
    while (1);

}

/* double_fault_exception(void)
 * INPUT: void
 * OUTPUT: none
 * FUNCTION: print error info for double fault exception
 *
*/
void double_fault_exception(void)
{
    clear();
    printf("EXCEPTION: Double Fault");
    while (1);

}

/* coprocessor_segment_overrun(void)
 * INPUT: void
 * OUTPUT: none
 * FUNCTION: print error info for coprocessor segment overrun
 *
*/
void coprocessor_segment_overrun(void)
{
    clear();
    printf("EXCEPTION: Coprocessor Segment Overrun");
    while (1);

}

/* invalid_tss_exception(void)
 * INPUT: void
 * OUTPUT: none
 * FUNCTION: print error info for invalid tss exception
 *
*/
void invalid_tss_exception(void)
{
    clear();
    printf("EXCEPTION: Invalid TSS");
    while (1);

}

/* segment_not_present(void)
 * INPUT: void
 * OUTPUT: none
 * FUNCTION: print error info for segment not present
 *
*/
void segment_not_present(void)
{
    clear();
    printf("EXCEPTION: Segment not Present");
    while (1);

}

/* stack_fault_exception(void)
 * INPUT: void
 * OUTPUT: none
 * FUNCTION: print error info for stack fault exception
 *
*/
void stack_fault_exception(void)
{
    clear();
    printf("EXCEPTION: Stack Fault");
    while (1);

}

/* general_protection_exception(void)
 * INPUT: void
 * OUTPUT: none
 * FUNCTION: print error info for general procection exception
 *
*/
void general_protection_exception(void)
{
    clear();
    printf("EXCEPTION: General Protection");
    while (1);

}

/* page_fault_exception(void)
 * INPUT: void
 * OUTPUT: none
 * FUNCTION: print error info for page fault exception
 *
*/
void page_fault_exception(void)
{
    // clear();
    printf("EXCEPTION: Page Fault");
    while (1);

}

/* x87_floating_point_error(void)
 * INPUT: void
 * OUTPUT: none
 * FUNCTION: print error info for x87 FLOP Error
 *
*/
void x87_floating_point_error(void)
{
    clear();
    printf("EXCEPTION: x87 Floating Point Error");
    while (1);

}

/* alignment_check_exception(void)
 * INPUT: void
 * OUTPUT: none
 * FUNCTION: print error info for aligment check exception
 *
*/
void alignment_check_exception(void)
{
    clear();
    printf("EXCEPTION: Alignment Check");
    while (1);

}

/* machine_check_exception(void)
 * INPUT: void
 * OUTPUT: none
 * FUNCTION: print error info for machine check exception
 *
*/
void machine_check_exception(void)
{
    clear();
    printf("EXCEPTION: Machine Check");
    while (1);

}

/* SIMD_floating_point_exception(void)
 * INPUT: void
 * OUTPUT: none
 * FUNCTION: print error info for SIMD FLOP exception
 *
*/
void SIMD_floating_point_exception(void)
{
    clear();
    printf("EXCEPTION: SIMD Floating Point");
    while (1);
}

/* sys_call_handler(void)
 * INPUT: void
 * OUTPUT: none
 * FUNCTION: temporary syscall handler
*/
void sys_call_handler(void)
{
  clear();
  printf("SYSCALL was called");
  while(1);
}

// /* Intel Reserved Exception 15
//  * INPUT: none
//  * OUTPUT: none
//  * FUNCTION: print intel reserved error
//  */
// void e15()
// {
//     clear();
//     printf("Intel Reserved Exception 15");
//     while (1);
// }

void keyboard_exception(void)
{
    keyboard_handler();
}

void rtc_exception(void)
{
    rtc_handler();
}
