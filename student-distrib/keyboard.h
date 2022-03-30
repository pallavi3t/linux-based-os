#ifndef _KEYBOARD_H
#define _KEYBOARD_H
#include "i8259.h"
#include "lib.h"
#include "idt.h"
#include "terminal.h"

#define KEYBOARD_IRQ 1
#define SET1_LENGTH  64
#define NUM_KB_SETS 4
#define KEYBOARD_PORT 0x60
#define KB_BUFFER_SIZE 128


#define ENTER 0x1C
#define CAPS_LOCK 0x3A
#define TAB 0x0F

#define LEFT_SHIFT 0x2A
#define LEFT_SHIFT_R 0xAA
#define RIGHT_SHIFT 0x36
#define RIGHT_SHIFT_R 0xB6

#define LEFT_ALT 0x38
#define LEFT_ALT_R 0xB8

#define BACKSPACE 0x0E

#define LEFT_CTRL 0x1D
#define LEFT_CTRL_R 0x9D
// #define RIGHT_CTRL 0xE0
// #define RIGHT_CTRL_R

#define KEY_COUNT 60
#define NULL_KEY '\0'

// #define F1 59
// #define F10 68
// #define F11 87
// #define F12 88
#define F1 0x3B
#define F2 0x3C
#define F3 0x3D
#define one 0x02
#define two 0x03
#define three 0x04

#define TERMINAL_ONE 0
#define TERMINAL_TWO 1
#define TERMINAL_THREE 2


#define KEY_NOT_PRESSED 0
#define KEY_PRESSED 1

// extern unsigned char keyboard_buffer[BUFFER_SIZE];
// extern int keyboard_buffer_idx;



/* Initialize keyboard */
void keyboard_init(void);

/* Print a character from the keyboard */
void keyboard_handler(void);

// typedef struct keyboard_t{
//     int shift_flag;             // flags: 0 -- key not pressed | 1 -- key pressed
//     int caps_lock_flag;
//     int ctrl_flag;
//     int alt_flag;
//     int enter_flag;

//     unsigned char keyboard_buffer[KB_BUFFER_SIZE];   // max buffer size = 128
//     int kb_buffer_index;

//     unsigned char scancode;
// }keyboard_t;


/* call when keyboard is pressed */
void handle_keyboard_press(unsigned char scancode);


/* call when ENTER is pressed */
void handle_enter_press();

/* call when TAB is pressed */
void handle_tab_press();

void handle_wrap_around();

/* call when BACKSPACE is pressed */
void handle_backspace_press();

/* call to set keyboard mode */
void set_keyboard_mode();



///////////////////////////////////////////////////////////////



#endif
