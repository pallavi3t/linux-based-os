#ifndef _RTC_H
#define _RTC_H
#include "i8259.h"
#include "lib.h"
#include "idt.h"
#include "idt_exceptions.h"

#define RTC_IRQ      8
#define REG_A        0x8A
#define REG_B        0X8B
#define REG_C        0x0C
#define RTC_PORT     0X70
#define CMOS_PORT    0X71

#define MAX_FREQ     1024
#define BASE_RATE_MASK    0X02

/* Initialize RTC */
void rtc_init(void);

/* Handle RTC interrupts */
void rtc_handler(void);

/*
 * System Calls
 */
int32_t rtc_open(const uint8_t* filename);

int32_t rtc_read(int32_t fd, void* buf, int32_t nbytes);

int32_t rtc_write(int32_t fd, const void* buf, int32_t nbytes);

int32_t rtc_close(int32_t fd);

#endif
