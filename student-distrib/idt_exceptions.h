#ifndef _EXCEPTION_H
#define _EXCEPTION_H

#include "idt.h"
#include "rtc.h"
#include "keyboard.h"

/* Exceptions */
/* Interrupt 0 */
extern void divide_by_zero_error(void);

/* Interrupt 1 */
extern void debug_exception(void);

/* Interrupt 2 */
extern void non_maskable_interrupt(void);

/* Interrupt 3 */
extern void breakpoint_exception(void);

/* Interrupt 4 */
extern void overflow_exception(void);

/* Interrupt 5 */
extern void bound_range_exceeded_exception(void);

/* Interrupt 6 */
extern void invalid_opcode_exception(void);

/* Interrupt 7 */
extern void device_not_available(void);

/* Interrupt 8 */
extern void double_fault_exception(void);

/* Interrupt 9 */
extern void coprocessor_segment_overrun(void);

/* Interrupt 10 */
extern void invalid_tss_exception(void);

/* Interrupt 11 */
extern void segment_not_present(void);

/* Interrupt 12 */
extern void stack_fault_exception(void);

/* Interrupt 13 */
extern void general_protection_exception(void);

/* Interrupt 14 */
extern void page_fault_exception(void);

/* Interrupt 15 */
//extern void e15();

/* Interrupt 16 */
extern void x87_floating_point_error(void);

/* Interrupt 17 */
extern void alignment_check_exception(void);

/* Interrupt 18*/
extern void machine_check_exception(void);

/* Interrupt 19*/
extern void SIMD_floating_point_exception(void);

/* Interrupt 20*/
extern void sys_call_handler(void);

/*Add user defined interrupts
 *
*/
// extern void e20();
// extern void e21();
// extern void e22();
// extern void e23();
// extern void e24();
// extern void e25();
// extern void e26();
// extern void e27();
// extern void e28();
// extern void e29();
// extern void e30();
// extern void e31();

/* Keyboard */
extern void keyboard_exception(void);

/* RTC */
extern void rtc_exception(void);

#endif
