#include "lib.h"
#include "pit.h"
#include "i8259.h"
#include "lib.h"
#include "system_call.h"

/* constants for initializing the pit */
#define PIT_IRQ 0
#define PIT_CMD_PORT 0x43
#define PIT_CHANNEL0_PORT 0x40

/*
 * the command argument is the byte sent to the command port
 * this gives information in the following order:
 * bits 7-6: channel number (1,2 or 3)
 * bits 5-4: what byte to read for the data given to the channel port chosen
 *           that is in a 16 bit value, bit 4 represents LSB, bit 5 represents MSB
 *           if both are 1, then LSB is read first then MSB thus the data is sent in that order
 *           our data is 16 bit hence both bits are 1
 * bits 3-1: specify the mode of the pit (1-5), we use mode 3
 * bit  0:   specifies the counting system of the internal counter, bit is 1 for BCD counter
 *           and 0 for regular 16 bit counter
 */
#define PIT_CMD_ARG 0x36

/*
 * this number gives us interrupts at a frequency of 100 Hz 
 * the concept of mode 3 is that we get an interrupt at the frequency:
 * 1193180/number sent as data to the channel port (channel 0 for us)
 * 1193180 Hz or 1.19 MHz is the internal frequency of the pit and we want
 * an interrupt at every 10 ms, so a frequency of 100 Hz for that we have to
 * divide 1193180 by 11932 (rounded of figure) to get 100 Hz, which is the data
 * we send to channel 0 data port
 */
#define PIT_DATA 11932

int pit_count;

int test_count;

/*
 * pit_init
 * DESCRIPTION: initializes the pit
 * INPUT: none
 * OUTPUT: the pit is ready to send interrupts
 * RETURN: none
 */
void pit_init()
{
    /* clear interrupts to ensure smooth initialization */
    cli();

    /* update pit count */
    pit_count = 0;
    test_count = 0;

    /* write the command argument to the pit command port to set the 
       channel, mode, and other settings of the pit */
    outb(PIT_CMD_ARG, PIT_CMD_PORT);

    /* write the pit data to channel 0, one byte at a time */
    /* first write the LSB which is the lower 8 bits, hence the bitwise AND
       with 0xFF (gets the LSB only) */
    outb(PIT_DATA & 0xFF, PIT_CHANNEL0_PORT);

    /* right shift the number by 8 to get the bits of the MSB into the LSB or
       that is to say get the top 8 bits into the low 8 bits and send to the port */
    outb(PIT_DATA >> 8, PIT_CHANNEL0_PORT);

    /* set the interrupts again */
    sti();

    /* enable the pit irq line on the pic(s) */
    enable_irq(PIT_IRQ);
}

/*
 * pit_irq_handler
 * DESCRIPTION: handles the interrupts by the pit
 * INPUT: none
 * OUTPUT: interrupts are handled and scheduling is called
 * RETURN: none
 */
void pit_irq_handler()
{
    send_eoi(PIT_IRQ);

    // 12/6 Yan
    /* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
   To Proceed debug: Comment out 95-103 and Uncomment 105
   95-103 is already in scheduler(), this part just ensures that problem lies on Context Switch, not earlier part
   !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */

    // Mimicing 1st part of scheduler() to DEBUG
    pit_count++;
    pit_count = (pit_count % 3);
    if (test_count < 3)
    {
        // We artificially create 3 terminals
        terminal_switch(test_count);
        terminal_switch(0);
        test_count++;
    }

    // scheduler();

    // printf("pit: %d \n", pit_count);
}
