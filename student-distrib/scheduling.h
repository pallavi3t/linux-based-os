#ifndef _SCHEDULING_H
#define _SCHEDULING_H

#include "file_system.h"
#include "terminal.h"
#include "paging.h"
#include "types.h"
#include "lib.h"
#include "sys_calls.h"

/* constants */
#define PIT_IRQ 0
#define PIT_CHANNEL0_PORT 0x40      // Channel 0 data port (read/write)
#define PIT_COMMAND_REG 0x43         // Mode/Command register (write only, a read is ignored)
#define PIT_COMMAND_BYTE 0x36
#define PIT_DEF_FREQ 1193182
#define LOW_DIVISOR_MASK 0xFF
#define HIGH_DIVISOR_MASK 8

/* variables */
// int terminals[3];

/* PIT */
void pit_init();
void pit_handler();

/* scheduling */
void scheduler();

int current_term;

#endif
