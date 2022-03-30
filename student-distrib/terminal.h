#ifndef _TERMINAL_H
#define _TERMINAL_H
#include "lib.h"

#define BUFFER_SIZE 128
#define VGA_STATUS 0x3D4
#define VGA_DATA 0x3D5
#define CUR_L 0x0F          // 15: end of scanline
#define CUR_H 0x0E          // 14: start of scanline
#define BITMASK_LOW_8 0xFF
#define LAST_CHAR 127
#define NUM_TERMS 3         // number of terminals
#define TERMINAL_ONE 0
#define TERMINAL_TWO 1
#define TERMINAL_THREE 2
#define NEWLINE '\n'

typedef struct {
    int cursor_x;
    int cursor_y;
    int extra_lines;

    unsigned char keyboard_buffer[BUFFER_SIZE];
    int init_flag;
    int keyboard_buffer_idx;
    
    uint8_t* video_memory;    // pointer to video mem for terminal
    int active;
}terminal_t;

terminal_t term[NUM_TERMS];     // array of terminal structs
int current_terminal_num;    // index of current terminal state

void terminal_init(void);

int32_t terminal_open(const uint8_t* filename);

int32_t terminal_read(int32_t fd, void* buf, int32_t nbytes);

int32_t terminal_write(int32_t fd, const void* buf, int32_t nbytes);

int32_t terminal_close(int32_t fd);

void terminal_switch();
void term_switch(int term_num);

int append_to_buffer(unsigned char key);

/* clears keyboard buffer */
void clear_keyboard_buffer();

void terminal_scroll();


void clear_screen();

uint32_t read_flag;

// int32_t launch_terminal(int terminal_number);
// int32_t save_terminal_state(int terminal_number);
// int32_t restore_terminal_state(int terminal_number);



#endif
