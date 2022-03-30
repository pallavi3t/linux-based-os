#ifndef _CURSOR_H
#define _CURSOR_H

#include "types.h"

void enable_cursor();

void disable_cursor();

void update_cursor();

void update_cursor_s(int row, int col);

uint16_t get_cursor_position(void);

#endif
