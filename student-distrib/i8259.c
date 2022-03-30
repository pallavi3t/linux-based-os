/* i8259.c - Functions to interact with the 8259 interrupt controller
 * vim:ts=4 noexpandtab
 */

#include "i8259.h"
#include "lib.h"

/* Interrupt masks to determine which interrupts are enabled and disabled */
uint8_t master_mask; /* IRQs 0-7  */
uint8_t slave_mask;  /* IRQs 8-15 */

/* void i8259_init()
 * Input: void
 * Output: void
 * Side effects: initialize the PIC, sets up master/slave values
 */

/* Initialize the 8259 PIC */
void i8259_init(void) {

    /* save mask values */
    master_mask = 0xFF; 
    slave_mask = 0xFF;

    /* mask both master and slave */
    // outb(0xFF, MASTER_DATA);
    // outb(0xFF, SLAVE_DATA);

    /* master initialization */
    outb(ICW1, MASTER_COMMAND);
    outb(ICW2_MASTER, MASTER_DATA);
    outb(ICW3_MASTER, MASTER_DATA);
    outb(ICW4, MASTER_DATA);

    /* slave initialization */
    outb(ICW1, SLAVE_COMMAND);
    outb(ICW2_SLAVE, SLAVE_DATA);
    outb(ICW3_SLAVE, SLAVE_DATA);
    outb(ICW4, SLAVE_DATA);

    outb(master_mask, MASTER_DATA);
    outb(slave_mask, SLAVE_DATA);

    enable_irq(2);

}


/* void enable_irq()
 * Input: IRQ to be unmasked
 * Output: void
 * Side effects: unmasks the requested IRQ
 */

/* Enable (unmask) the specified IRQ */
void enable_irq(uint32_t irq_num) {

    // uint8_t mask;

    if (irq_num < 8){
        master_mask = inb(MASTER_DATA) & ~(1 << irq_num); 
        outb(master_mask, MASTER_DATA);
    }
    else {
        irq_num -= 8;
        slave_mask = inb(SLAVE_DATA) & ~(1 << irq_num);
        outb(slave_mask, SLAVE_DATA);

        //send interrupt to pin on master where slave is
        // mask = inb(MASTER_DATA) & (~ICW3_SLAVE);
        // outb(mask, MASTER_DATA);
    }

}


/* void disable_irq()
 * Input: IRQ to be masked
 * Output: void
 * Side effects: masks the requested IRQ
 */

/* Disable (mask) the specified IRQ */
void disable_irq(uint32_t irq_num) {

    // uint8_t mask;

    if (irq_num < 8){
        master_mask = inb(MASTER_DATA) | (1 << irq_num);
        outb(master_mask, MASTER_DATA);
    }
    else {
        irq_num -= 8;
        slave_mask = inb(SLAVE_DATA) | (1 << irq_num);
        outb(slave_mask, SLAVE_DATA);

        //send interrupt to pin on master where slave is
        // mask = inb(MASTER_DATA) | ICW3_SLAVE;
        // outb(mask, MASTER_DATA);
    }

}


/* void send_eoi()
 * Input: IRQ number to send eoi signal of
 * Output: void
 * Side effects: sends eoi signal for requested IRQ
 */

/* Send end-of-interrupt signal for the specified IRQ */
void send_eoi(uint32_t irq_num) {

    if(irq_num >= 8)
	{
        irq_num -= 8;
        outb((EOI | irq_num), SLAVE_COMMAND);
        outb((EOI + 2), MASTER_COMMAND);
    }
    else 
    {
        outb((EOI | irq_num), MASTER_COMMAND);
    }

}
/* 
void pit_handler()
{
  return;
} */
