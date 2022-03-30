#include "i8259.h"
#include "lib.h"
#include "keyboard.h"
#include "cursor.h"
#include "terminal.h"

//source: https://wiki.osdev.org/PS/2_Keyboard
static char scan_code_table[NUM_KB_SETS][SET1_LENGTH] =
{
	{'\0', '\0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\0', '\0',
	 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\0', '\0', 'a', 's',
	 'd', 'f', 'g', 'h', 'j', 'k', 'l' , ';', '\'', '`', '\0', '\\', 'z', 'x', 'c', 'v',
	 'b', 'n', 'm',',', '.', '/', '\0', '*', '\0', ' ', '\0'},

/* SHIFT */
    {'\0', '\0', '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\0', '\0',
	 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\0', '\0', 'A', 'S',
	 'D', 'F', 'G', 'H', 'J', 'K', 'L' , ':', '"', '~', '\0', '|', 'Z', 'X', 'C', 'V',
	 'B', 'N', 'M','<', '>', '?', '\0', '*', '\0', ' ', '\0'},

/* CAPS LOCK */
    {'\0', '\0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\0', '\0',
	 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '[', ']', '\0', '\0', 'A', 'S',
	 'D', 'F', 'G', 'H', 'J', 'K', 'L' , ';', '\'', '`', '\0', '\\', 'Z', 'X', 'C', 'V',
	 'B', 'N', 'M',',', '.', '/', '\0', '*', '\0', ' ', '\0'},

/* SHIFT + CAPS LOCK*/
    {'\0', '\0', '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\0', '\0',
	 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '{', '}', '\0', '\0', 'a', 's',
	 'd', 'f', 'g', 'h', 'j', 'k', 'l' , ':', '"', '~', '\0', '\\', 'z', 'x', 'c', 'v',
	 'b', 'n', 'm', '<', '>', '?', '\0', '*', '\0', ' ', '\0'}
};



//keyboard_t keyboard;
int keyboard_mode;              /* 0 - default | 1 - shift | 2 - caps | 3 - shift + caps */
// int keyboard_buffer_index;          /* index of keyboard buffer */
// unsigned char* keyboard_buffer;
int shift_flag;
int ctrl_flag;
int alt_flag;
int caps_lock_flag;
int enter_flag;
unsigned char scancode;


/* void keyboard_init()
 * Input: void
 * Output: void
 * Side effects: initialize the keyboard by unmasking the keyboard IRQ
 */
void keyboard_init(){
    enable_irq(KEYBOARD_IRQ);

    /* Initialize flags to 0 (not pressed) */
    shift_flag = 0;
    caps_lock_flag = 0;
    ctrl_flag = 0;
    alt_flag = 0;
    enter_flag = 0;

    //keyboard_buffer_index = -1;
    keyboard_mode = 0;

    // /* Initialize empty keyboard buffer */
    // int i;
    // for(i = 0; i < KB_BUFFER_SIZE; i++){
    //     keyboard_buffer[i] = '\0';
    // }

    /* get scancode */
	//terminal_init();
}



/* void keyboard_handler()
 * Input: void
 * Output: void
 * Side effects: send eoi signal for keyboard, takes a single character press and prints character to screen
 */
void keyboard_handler(){


    cli();
	scancode = inb(KEYBOARD_PORT);
    send_eoi(KEYBOARD_IRQ);
    //unsigned char temp = '\0';
    // ..._R means release
    switch (scancode){
		case LEFT_ALT:
			alt_flag = KEY_PRESSED;
			break;

		case LEFT_ALT_R:
			alt_flag = KEY_NOT_PRESSED;
			break;

        case LEFT_SHIFT:
			shift_flag = KEY_PRESSED;
			break;

        case RIGHT_SHIFT:
            shift_flag = KEY_PRESSED;
            break;

        case LEFT_SHIFT_R:
			shift_flag = KEY_NOT_PRESSED;
			break;
        case RIGHT_SHIFT_R:
            shift_flag = KEY_NOT_PRESSED;
            break;

		case LEFT_CTRL:
		    ctrl_flag = KEY_PRESSED;
		    break;

		case LEFT_CTRL_R:
		    ctrl_flag = KEY_NOT_PRESSED;
            break;

		case CAPS_LOCK:
	        if (!caps_lock_flag){   // if caps lock off, turn on
	            caps_lock_flag = KEY_PRESSED;
	          }
	        else{                   // else, turn caps lock off
	            caps_lock_flag = KEY_NOT_PRESSED;
	          }
	        break;

        case F1:
        // case one:
            if (alt_flag){
                term_switch(0);
                break;
            }
            else {
                set_keyboard_mode();
                handle_keyboard_press(scancode);     // NOTE: might need to move this, or change how
                break;  
            }

        case F2:
        // case two:
            if (alt_flag){
                term_switch(1);
                break;
            }
            else {
                set_keyboard_mode();
                handle_keyboard_press(scancode);     // NOTE: might need to move this, or change how
                break;  
            }

        case F3:
        // case three:
            if (alt_flag){
                term_switch(2);
                break;
            }
            else {
                set_keyboard_mode();
                handle_keyboard_press(scancode);     // NOTE: might need to move this, or change how
                break;  
            }

	    case TAB:
			handle_tab_press();

        case BACKSPACE:
            handle_backspace_press();
            break;
		case ENTER:
			handle_enter_press();
		    break;
        default:
			set_keyboard_mode();
            handle_keyboard_press(scancode);     // NOTE: might need to move this, or change how
            break;                          // keyboard_mode is changed
    }
    sti();

}


/* void set_keyboard_mode()
 * Input: none
 * Output: void
 * Side effects: determines correct keyboard_mode to correctly index into scan code table
 */
void set_keyboard_mode(){
    if (shift_flag && caps_lock_flag){
        keyboard_mode = 3;
    }
    else if (!shift_flag && caps_lock_flag){
        keyboard_mode = 2;
    }
    else if (shift_flag && !caps_lock_flag){
        keyboard_mode = 1;
    }
    else{
        keyboard_mode = 0;
    }
}



/* void handle_keyboard_press(unsigned char scancode)
 * Input: scancode
 * Output: void
 * Side effects: modifies what will be displayed to screen
 */
void handle_keyboard_press(unsigned char scancode){
    if (scancode >= KEY_COUNT){
        return;
    }

    unsigned key = scan_code_table[keyboard_mode][scancode];

    /* handle '\0' characters */
    if (key == NULL_KEY){
        return;
    }

    // if(alt_flag && ((scancode>=F1 && scancode<=F10) || (scancode == F11) || (scancode == F12)))
    // {
    //     printf("Pressed alt + fn");
    //     terminal_switch();
    //     return;
    // }

    // if((alt_flag && scancode == one) || (alt_flag && scancode == two) || (alt_flag && scancode ==three) || (alt_flag && scancode ==0x14))  // 0x14 == 'T'
    // {
    //     //printf("Pressed alt + fn");
    //     terminal_switch();
    //     return;
    // }
    /* CTRL option
     * CTRL + L clears screen and sets cursor at top
     */
    if ((ctrl_flag == 1 && key == 'l') || (ctrl_flag == 1 && key == 'L')){
        set_screen(0,0);
        update_cursor();
        clear();

	}


    /* normal case, key is added to buffer */
    else
    {
        if(append_to_buffer(key)==0)
		{
      	    putc(key);
						return;
    	}
        // TODO: set cursor position
	}
}


/* void handle_tab_press()
 * Input: none
 * Output: none
 * Side effects: add tab character to buffer
*/
void handle_tab_press()
{
	int i;
	for(i = 0; i < 4; i++) // tab is just 4 spaces
	{
		if(append_to_buffer(' ')==0)
		{
			putc(' ');
		}
		else
		{
			return;
		}
	}
    // TODO: set cursor position
}


/* void handle_enter_press()
 * Input: none
 * Output: none
 * Side effects: add newline character to buffer
*/
void handle_enter_press(){
    read_flag = 1;
    if(append_to_buffer('\n')==0)
    {
        putc('\n');
    }
}

/* void handle_wrap_around()
 * Input: none
 * Output: none
 * Side effects: moves to next line once characters fill line
*/
void handle_wrap_around(){
    if (term[current_terminal_num].keyboard_buffer_idx % NUM_COLS == 0){
        putc('\n');
    }
}




/* void handle_backspace_press()
 * Input: none
 * Output: none
 * Side effects: removes char from buffer
*/
void handle_backspace_press(){
    if (term[current_terminal_num].keyboard_buffer_idx > 0){
				term[current_terminal_num].keyboard_buffer_idx--;
			  term[current_terminal_num].keyboard_buffer[term[current_terminal_num].keyboard_buffer_idx] = NULL_KEY;
					backspace();
					if(term[current_terminal_num].cursor_y+2==NUM_ROWS)
					{
						scroll_up();
					}
					term[current_terminal_num].cursor_x--;
					update_cursor();
					

    }
}


/////////////////////////////////////////////////////
