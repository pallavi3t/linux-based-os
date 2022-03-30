#include "terminal.h"
#include "keyboard.h"
#include "lib.h"
#include "cursor.h"
#include "paging.h"
#include "sys_calls.h"



/* Terminal functions */

// Global variables
//int current_terminal_num;

/* void terminal_init()
 * Input: void
 * Output: none
 * Side effects: terminal variables are set
 */
void terminal_init(void)
{
  int i;

  // Initialize all 3 terminals
  for (i = 0; i < NUM_TERMS; i++)
  {
    int j;

    term[i].cursor_x = 0;
    term[i].cursor_y = 0;
    term[i].init_flag = 0;

    for(j = 0; j < BUFFER_SIZE; j++)
    {
      term[i].keyboard_buffer[j] = 0;
    }

    term[i].keyboard_buffer_idx = 0;
    term[i].extra_lines = 0;
    set_screen(term[i].cursor_x, term[i].cursor_y);
    read_flag = 0;
    term[i].active = 0;

    // store video memory ptr for each terminal
    term[i].video_memory = (uint8_t *)VID_MEM_ADDR + ((i + 1) * _4KB); 
      // term[i].video_memory = (uint8_t *)VID_MEM_ADDR + (i + 1); 


    for (j = 0; j < NUM_ROWS * NUM_COLS; j++)
    {
      // clear video memory
      *(uint8_t *)(term[i].video_memory + (j << 1)) = ' ';
      *(uint8_t *)(term[i].video_memory + (j << 1) + 1) = ATTRIB;
      // *(uint8_t *)(VIDEO + (j << 1)) = ' ';


      // set attributes for each terminal
      // switch(i)
      // {
      //   case TERMINAL_ONE:
      //     *(uint8_t *)(term[i].video_memory + (j << 1) + 1) = ATTRIB_1;

      //   case TERMINAL_TWO:
      //     *(uint8_t *)(term[i].video_memory + (j << 1) + 1) = ATTRIB_2;

      //   case TERMINAL_THREE:
      //     *(uint8_t *)(term[i].video_memory + (j << 1) + 1) = ATTRIB_3;
      // }

    }
  // current_terminal_num = i;
  // execute((uint8_t *)"shell");
  }

  /* Execute the first program ("shell") ... */
  //clear();
  current_terminal_num = 0;
  //execute((uint8_t *)"shell");
}


/* void terminal_switch()
 * Input: none
 * Output: none
 * Side effects: cycles which terminal is on 
 */
void terminal_switch()
{
  int old_terminal_num = current_terminal_num;
  current_terminal_num++;
  if(current_terminal_num==3)
  {
    current_terminal_num=0;
  }
  memcpy((uint8_t *) term[old_terminal_num].video_memory, (uint8_t*)VIDEO, 2*NUM_COLS*NUM_ROWS);
  // memcpy((uint8_t *) VIDEO + ((1 + old_terminal_num) << 12), (uint8_t*)VIDEO, (1 << 12));

  memcpy((uint8_t*)VIDEO, (uint8_t *) term[current_terminal_num].video_memory, 2*NUM_COLS*NUM_ROWS);
  // memcpy((uint8_t*)VIDEO, (uint8_t *) VIDEO + ((1 + current_terminal_num) << 12), (1 << 12));

  set_screen(term[current_terminal_num].cursor_x,term[current_terminal_num].cursor_y);
  update_cursor();
  
}

/* void term_switch(int term_num)
 * Input: none
 * Output: none
 * Side effects: switch to term_num terminal, return 0 on success
 */
void term_switch(int term_num)
{
  if (current_terminal_num == term_num){
    return;
  }

  if (current_terminal_num >=0 && current_terminal_num < NUM_TERMS)
  {
    // save vid mem for old terminal state
    memcpy(term[current_terminal_num].video_memory, (uint8_t*)VIDEO, 2*NUM_COLS*NUM_ROWS);
    current_terminal_num = term_num;

    //clear();
    memcpy((uint8_t*)VIDEO, term[current_terminal_num].video_memory, 2*NUM_COLS*NUM_ROWS);
    set_screen(term[current_terminal_num].cursor_x,term[current_terminal_num].cursor_y);
    update_cursor();
    //return 0;
  }

  // if (term[current_terminal_num].init_flag == 0)
  // {
  //   term[current_terminal_num].init_flag = 1;
  //   execute((uint8_t *)"shell");
  // }

  return;
  
  
}


/* void terminal_open()
 * Input: void
 * Output: 0 on success, -1 on error
 * Side effects: send end of interrupt for keyboard
 */
int32_t terminal_open(const uint8_t* filename){

  return 0; // return success

}



/* void terminal_read()
 * Input: void
 * Output: data from one line that has been terminated by pressing Enter
 *         or as much as fits in one such line
 *         OR 0 if initial file position is at or beyond end of file,
 *         OR -1 on error
 * Side effects: send end of interrupt for keyboard
 */
int32_t terminal_read(int32_t fd, void* buf, int32_t nbytes){  
  while (read_flag != 1)
  {
    // if (term[0].init_flag == 0 || term[1].init_flag == 0 || term[2].init_flag == 0)
    // {
    //   execute_shell();
    // }
    continue;
  }
  read_flag = 0;
  // buf needs to be cast from void
  unsigned char* buffer = (unsigned char*) buf;
  int char_counter;
  int i;
  int last_enter=-1;
  // catch errors with passed in parameters
  if(buffer == NULL) return -1;
  if(nbytes <= 0) return -1;

  // mp3.2 tips say to handle buffer overflow
  if(nbytes>BUFFER_SIZE)
  {
    nbytes = BUFFER_SIZE; // limit nbytes to max buffer size
  }

  for(i = 0; i < BUFFER_SIZE; i++)
  {
    if(term[current_terminal_num].keyboard_buffer[i] =='\n')
    {
      last_enter =i; //find where last enter is so we can print the lines correctly
      break;
    }
    if(term[current_terminal_num].keyboard_buffer[i] != NULL)
    {
    char_counter++;
    }
    else{
      break; //once NULL is hit, no more characters in buffer    }
    }
  }


  for(i = 0; i < nbytes; i++)
    {
      if(i <= last_enter){
      buffer[i] = term[current_terminal_num].keyboard_buffer[i]; //copy everything up to last enter
    }
    else
    {
      buffer[i] = 0; // fill rest with 0s
    }
  }

// fix keyboard_buffer to account for last enter pressed
int max_size = (BUFFER_SIZE-1)-last_enter; // want the max numb of characters left in buffer. We subtract one since this is index
for(i = 0; i < max_size; i++)
{
  term[current_terminal_num].keyboard_buffer[i] = term[current_terminal_num].keyboard_buffer[i+last_enter+1];
}
while(i<BUFFER_SIZE)
{
  term[current_terminal_num].keyboard_buffer[i] = 0;
  i++;
}
term[current_terminal_num].keyboard_buffer_idx -= (last_enter+1);

return last_enter+1;
}



/* void terminal_write()
 * Input: void
 * Output: # of bytes written
 *         OR -1 on error
 * Side effects: data displayed on the screen
 */
int32_t terminal_write(int32_t fd, const void* buf, int32_t nbytes)
{
// cli();

// buf needs to be cast from void
unsigned char* buffer = (unsigned char*) buf;

// catch errors with passed in parameters
if(buffer==NULL) return -1;
if(nbytes<0) return -1;


int byte_counter;
int i;
byte_counter = 0;

for(i=0;i<nbytes;i++)
{
  // if(term.keyboard_buffer[i] != NULL) // don't print for NULL
  // {
  //   //print from buffer and increment the counter
  //   putc(buffer[i]);
  //   byte_counter++;
  // }
  //print from buffer and increment the counter

  putc(buffer[i]);
  byte_counter += 1;
}

// sti();

return byte_counter;
}



/* void terminal_close()
 * Input: void
 * Output: 0 on success, -1 on error
 * Side effects: send end of interrupt for keyboard
 */
int32_t terminal_close(int32_t fd)
{

return 0; // return success

}



/* int append_to_buffer(key)
 * Input: key to append to buffer
 * Output: none
 * Side effects: modifies content keyboard buffer and increments keyboard_buffer_idx
*/

int append_to_buffer(unsigned char key)
{
  if(key==NEWLINE && term[current_terminal_num].keyboard_buffer_idx == KB_BUFFER_SIZE)
  {
    return -1;
  }

  if(key>LAST_CHAR){
    return -1;
  }
  if(key!= NEWLINE && term[current_terminal_num].keyboard_buffer_idx==KB_BUFFER_SIZE-1)
  {
    return -1;
  }

  if(term[current_terminal_num].keyboard_buffer_idx < BUFFER_SIZE)
  {
    term[current_terminal_num].keyboard_buffer[term[current_terminal_num].keyboard_buffer_idx] = key;
    term[current_terminal_num].keyboard_buffer_idx++;
    return 0;
  }

  else
  {
    return -1; // Failure, could not append
  }
}



/* void clear_keyboard_buffer()
 * Input: none
 * Output: none
 * Side effects: set vals in buffer to '\0' and reset index
*/
void clear_keyboard_buffer()
{
    int i;
    for (i = 0; i < BUFFER_SIZE; i++){
        term[current_terminal_num].keyboard_buffer[i] = NULL_KEY;
    }
    term[current_terminal_num].keyboard_buffer_idx = 0;
}
