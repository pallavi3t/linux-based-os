#include "x86_desc.h"
#include "idt_exceptions.h"
#include "keyboard.h"
#include "rtc.h"
#include "idt.h"
#include "i8259.h"
#include "sys_calls.h"
#include "scheduling.h"


/* void idt_init()
 * Input: void
 * Output: void
 * Side effects: idt values are set
 */
void idt_init()
{
    lidt(idt_desc_ptr);     /* load IDT with given descriptor (idt_desc_ptr)*/
    int i;
    /* Intel pg 156
     * -------------------------------------------
     * 15   14 13   12 ...  8   7 6 5  4  ...  0
     * P     DPL    0 D 1 1 0   0 0 0  (reserved)
     * -------------------------------------------
     * DPL - hardware interrupt handlers and exception handlers
     *       set to 0 (prevent user-level apps from calling into these routines)
     *     - system call handler set to 3 (accessible from user space)
     * D (size of gate) - 1 = 32 bits | 0 = 16 bit
     * present
    */
    for (i = 0; i < NUM_VEC; i++){         /* NUM_VEC = 256 */
        idt[i].seg_selector = KERNEL_CS;    /* kernel code segment descriptor*/
        idt[i].reserved4 = 0;           /* bits 5-7? */
        idt[i].reserved3 = 0;           /* bit 8 */
        idt[i].reserved2 = 1;
        idt[i].reserved1 = 1;
        idt[i].size = 1;                /* bit 11 (D) */
        idt[i].reserved0 = 0;           /* bit 12 */
        idt[i].dpl = 0;
        idt[i].present = 1;

        /* for entries below 32, set present bit to 1 */
        if (i < NUM_EXCEPTIONS){
            idt[i].present = 1; // want to set present to 1 for all exceptions
            idt[i].reserved3 = 0x1; // this lets us know its an exception
        }

        /* system calls */
        if (i == SYS_CALL_VECTOR){
            idt[i].reserved3 = 1;           /* bit 8 */
            idt[i].size = 1;    
            idt[i].dpl = 3;
        }
    }

    /* Set up exception entries */
    SET_IDT_ENTRY(idt[0], divide_by_zero_error);
    SET_IDT_ENTRY(idt[1], debug_exception);
    SET_IDT_ENTRY(idt[2], non_maskable_interrupt);
    SET_IDT_ENTRY(idt[3], breakpoint_exception);
    SET_IDT_ENTRY(idt[4], overflow_exception);
    SET_IDT_ENTRY(idt[5], bound_range_exceeded_exception);
    SET_IDT_ENTRY(idt[6], invalid_opcode_exception);
    SET_IDT_ENTRY(idt[7], device_not_available);
    SET_IDT_ENTRY(idt[8], double_fault_exception);
    SET_IDT_ENTRY(idt[9], coprocessor_segment_overrun);
    SET_IDT_ENTRY(idt[10], invalid_tss_exception);
    SET_IDT_ENTRY(idt[11], segment_not_present);
    SET_IDT_ENTRY(idt[12], stack_fault_exception);
    SET_IDT_ENTRY(idt[13], general_protection_exception);
    SET_IDT_ENTRY(idt[14], page_fault_exception);
    // SET_IDT_ENTRY(idt[15], e15);
    SET_IDT_ENTRY(idt[16], x87_floating_point_error);
    SET_IDT_ENTRY(idt[17], alignment_check_exception);
    SET_IDT_ENTRY(idt[18], machine_check_exception);
    SET_IDT_ENTRY(idt[19], SIMD_floating_point_exception);
    // SET_IDT_ENTRY(idt[20], e20);
    // SET_IDT_ENTRY(idt[21], e21);
    // SET_IDT_ENTRY(idt[22], e22);
    // SET_IDT_ENTRY(idt[23], e23);
    // SET_IDT_ENTRY(idt[24], e24);
    // SET_IDT_ENTRY(idt[25], e25);
    // SET_IDT_ENTRY(idt[26], e26);
    // SET_IDT_ENTRY(idt[27], e27);
    // SET_IDT_ENTRY(idt[28], e28);
    // SET_IDT_ENTRY(idt[29], e29);
    // SET_IDT_ENTRY(idt[30], e30);
    // SET_IDT_ENTRY(idt[31], e31);

    /* Connect PIC, keyboard and RTC interrupts */
    // idt[PIT_VEC].present = 1;
    SET_IDT_ENTRY(idt[PIT_VEC],pit_handler);
    SET_IDT_ENTRY(idt[SYS_CALL_VECTOR], system_call_handler);
    // SET_IDT_ENTRY(idt[IRQ1_KEYBOARD], keyboard_handler);
    // SET_IDT_ENTRY(idt[IRQ8_RTC], rtc_handler);
    SET_IDT_ENTRY(idt[IRQ1_KEYBOARD], set_keyboard);
    SET_IDT_ENTRY(idt[IRQ8_RTC], set_rtc);

}
