#include "i8259.h"
#include "lib.h"
#include "rtc.h"


/*
 * Helper functions
 */
int rtc_set_frequency(int freq);

//variables 
volatile int rtc_interrupt_occured;


/* void rtc_init()
 * Input: void
 * Output: void
 * Side effects: initialize the rtc by unmasking the rtc IRQ 
 */
void rtc_init(){

//source: https://wiki.osdev.org/RTC#Programming_the_RTC

    //NOTE TO SELF: VITUALIZE RTC

    cli();

    outb(REG_B, RTC_PORT);              // select register B, and disable NMI
    char prev = inb(CMOS_PORT);	        // read the current value of register B

    outb(REG_B, RTC_PORT);              // set the index again (a read will reset the index to register D)
    outb(prev | 0x40, CMOS_PORT);	// write the previous value ORed with 0x40. This turns on bit 6 of register B

    rtc_interrupt_occured = 0;
    enable_irq(RTC_IRQ);

    rtc_set_frequency(2);

    sti();
}

/* void rtc_handler()
 * Input: void
 * Output: void
 * Side effects: send end of interrupt for rtc 
 */
void rtc_handler(){

    cli();

    send_eoi(RTC_IRQ);

    outb(REG_C, RTC_PORT);      // select register C
    inb(CMOS_PORT);             // just throw away previous interrupt

    rtc_interrupt_occured = 1;

    //test_interrupts(); 

    sti();
}


/*
 * System Calls
 */

/* void rtc_open()
 * Input: file 
 * Output: 0 on success, -1 on error
 * Side effects: send end of interrupt for rtc 
 */
int32_t rtc_open(const uint8_t* filename){

    //set up any data necessary  to handle the given type of file(RTC device)
    rtc_set_frequency(2);
    rtc_interrupt_occured = 0;
    return 0;
}

/* void rtc_read()
 * Input: void
 * Output: 0 when an interrupt has occured
 * Side effects: send end of interrupt for rtc 
 */
int32_t rtc_read(int32_t fd, void* buf, int32_t nbytes){

    while (rtc_interrupt_occured == 0){
        /* wait till interrupt is set */
    }

    //clear interrupt
    rtc_interrupt_occured = 0;

    return 0;

}

/* void rtc_write()
 * Input: 4-byte int specifying interrupt rate in Hz
 * Output: # of bytes written
 *         OR -1 on error
 * Side effects: send end of interrupt for rtc 
 */
int32_t rtc_write(int32_t fd, const void* buf, int32_t nbytes){

    // 4 byte int necessary
    if (buf == NULL || nbytes != 4) //input has to be 4 bytes
        return -1;
    
    if (rtc_set_frequency(*((int*)buf)) == -1)
        return -1;
        
    return nbytes;

}

/* void rtc_close()
 * Input: void
 * Output: 0 on success, -1 on error
 * Side effects: send end of interrupt for rtc 
 */
int32_t rtc_close(int32_t fd){
    
    return 0;

}

/*
 * Helper functions
 */

/* void rtc_set_frequency()
 * Input: frequency in Hz we want to set our RTC to 
 * Output: void
 * Side effects: sets the frequency of the RTC 
 */
int rtc_set_frequency(int freq){
    //source: https://wiki.osdev.org/RTC#Programming_the_RTC

    char prev, rate;

    //check if freq is less than 2 or greater than max of 1024
    //check if freq is exponent of 2
    if ((freq == 0) || (freq > MAX_FREQ) || ((freq & (freq - 1)) != 0))
        return -1;

    //find rate from input freq
    int mask = BASE_RATE_MASK;
    for (rate = 15; rate >= 6; rate--){ //F represents highest freq = 1024, 6 represents lowest freq = 2
        if (mask == freq)
            break;
        mask <<= 1;
    }

    //printf("\n freq = %d; rate = %d >>> ", freq, rate);

    cli();
    outb(REG_A, RTC_PORT);              // set index to register A, disable NMI
    prev = inb(CMOS_PORT);              // get initial value of register A
    outb(REG_A, RTC_PORT);              // reset index to A
    outb((prev & 0xF0) | rate, CMOS_PORT);//write only our rate to A. Note, rate is the bottom 4 bits.
    sti();

    return 0;

}
