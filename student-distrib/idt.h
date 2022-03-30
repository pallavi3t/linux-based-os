#ifndef IDT_H
#define IDT_H

#define SYS_CALL_VECTOR 0x80
#define IRQ1_KEYBOARD 0x21
#define IRQ8_RTC 0x28

#define PIT_VEC 0x20

void idt_init();

extern void exception_handler();

void set_keyboard();
void set_rtc();

#endif
