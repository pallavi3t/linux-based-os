#include "cursor.h"
#include "terminal.h"
#include "lib.h"


/* Source: https://wiki.osdev.org/Text_Mode_Cursor
 * void enable_cursor()
 * Input: none
 * Output: none
 * Side effects: create cursor in terminal
*/
void enable_cursor()
{
	outb(0x0A, VGA_STATUS);
	outb((inb(VGA_DATA) & 0xC0) | CUR_H, VGA_DATA);
 
	outb(0x0B, VGA_STATUS);
	outb((inb(VGA_DATA) & 0xE0) | CUR_L, VGA_DATA);
}

/* Source: https://wiki.osdev.org/Text_Mode_Cursor
 * void disable_cursor()
 * Input: none
 * Output: none
 * Side effects: disables cursor
*/
void disable_cursor(){
    outb(0x0A, VGA_STATUS);
	outb(0x20, VGA_DATA);
}


/* Source: https://wiki.osdev.org/Text_Mode_Cursor
 * void update_cursor()
 * Input: none
 * Output: none
 * Side Effects: update location of cursor based on screen location
*/
void update_cursor()
{
  uint16_t pos = get_screen_y() * NUM_COLS + get_screen_x();

  outb(CUR_L, VGA_STATUS);
  outb((uint8_t)(pos & BITMASK_LOW_8), VGA_DATA);          // low cursor bits
  outb(CUR_H, VGA_STATUS);
  outb((uint8_t)((pos >> 8) & BITMASK_LOW_8), VGA_DATA);   // shift by 8 to get high cursor bits
}



/* Source: https://wiki.osdev.org/Text_Mode_Cursor
 * void update_cursor_s(int row, int col)
 * Input: (row, col) cursor location
 * Output: none
 * Side Effects: update location of cursor based on (row, col)
 *               also updates screen location
*/
void update_cursor_s(int row, int col)
{
    set_screen_x(row);
    set_screen_y(col);
    update_cursor();
  //uint16_t pos = col * NUM_COLS + row;

//   outb(CUR_L, VGA_STATUS);
//   outb((uint8_t)(pos & BITMASK_LOW_8), VGA_DATA);          // low cursor bits
//   outb(CUR_H, VGA_STATUS);
//   outb((uint8_t)((pos >> 8) & BITMASK_LOW_8), VGA_DATA);   // shift by 8 to get high cursor bits
}



/* Source: https://wiki.osdev.org/Text_Mode_Cursor
 * uint16_t get_cursor_position(void)
 * Input: none
 * Output: pos = y * VGA_WIDTH + x 
 *         y = pos / VGA_WIDTH 
 *         x = pos % VGA_WIDTH
 * Side effects: none
*/
uint16_t get_cursor_position(void)
{
  uint16_t pos = 0;
  outb(CUR_L, VGA_STATUS);
  pos |= inb(VGA_DATA);
  outb(CUR_H, VGA_STATUS);
  pos |= ((uint16_t)inb(VGA_DATA)) << 8;
  return pos;
} 
