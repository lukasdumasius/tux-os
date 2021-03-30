/* i8259.c - Functions to interact with the 8259 interrupt controller
 * vim:ts=4 noexpandtab
 */

#include "i8259.h"
#include "lib.h"

/* PIC ports (written for convenience of name) */
#define MASTER_COM_PORT     0x20
#define MASTER_DATA_PORT    0x21
#define SLAVE_COM_PORT      0xA0
#define SLAVE_DATA_PORT     0xA1

/* Interrupt masks to determine which interrupts are enabled and disabled */
uint8_t master_mask; /* IRQs 0-7  */
uint8_t slave_mask;  /* IRQs 8-15 */

/*
 * i8259_init
 * DESCRIPTION: initializes both master and slave pics
 * INPUTS: none
 * OUTPUTS: none
 * RETURN VALUE: none
 * SIDE EFFECTS: masks all irq lines except line 2 on master, 
 *               connection to slave
 */
void i8259_init(void) {
    master_mask = 0xFF; /* update mask to mask all interrupts on master */
    slave_mask = 0xFF;  /* update mask to mask all interrupts on slave */
	
    /* mask all interrupts on both pics */
    outb(0xFF, MASTER_DATA_PORT);
    outb(0xFF, SLAVE_DATA_PORT);

    /* INITIALIZING THE MASTER PIC */
    /* ICW1: send 0x11 to the com port of the master pic to start init, 
       edge triggered inputs and ccascade mode */
    outb(ICW1, MASTER_COM_PORT);   
    /* ICW2: send the irq vectors for this pic to the data port */
    outb(ICW2_MASTER, MASTER_DATA_PORT);   
    /* ICW3: send the irq line number corresponding to the slave pic, that 
       is 00000100b, to the master data port */
    outb(ICW3_MASTER, MASTER_DATA_PORT);
    /* ICW4: send the normal eoi to the master pic */
    outb(ICW4, MASTER_DATA_PORT);

    /* INITIALIZING THE SLAVE PIC */
    /* ICW1: send 0x11 to the com port of the slave pic to start init, 
       edge triggered inputs and ccascade mode */
    outb(ICW1, SLAVE_COM_PORT);
    /* ICW2: send the irq vectors for this pic to the data port */
    outb(ICW2_SLAVE, SLAVE_DATA_PORT);
    /* ICW3: send the irq number corresponding to the slave pic on the 
       master pic, that is 2, to the slave data port */
    outb(ICW3_SLAVE, SLAVE_DATA_PORT);
    /* ICW4: send the normal eoi to the slave pic */
    outb(ICW4, SLAVE_DATA_PORT);
	
    /* update master's mask to enable iterrupts from the slave */
	master_mask= 0xFB;
	outb(master_mask, MASTER_DATA_PORT);
}


/*
 * enable_irq
 * DESCRIPTION: enables a specific irq number on the pics
 * INPUTS: irq number to enable
 * OUTPUTS: irq line corresponding to the given irq number is unmasked
 * RETURN VALUE: none
 * SIDE EFFECTS: current state of the pic mask(s) is updated
 */
void enable_irq(uint32_t irq_num) {
    uint8_t val;    /* variable to hold the bit mask representing which irq line is on */
    uint16_t port;  /* port at which the irq number is sent */
    uint8_t mask;   /* to store the previous pic mask */

    /* if irq belongs to master pic, then: */
    if (irq_num < 8) {
        port = MASTER_DATA_PORT;
        /* setting the bit corresponding to the irq number given to 0 in pic mask */
		val = 1 << irq_num;
        master_mask = ( master_mask&(~val) );
        /*updating master mask */
		mask = master_mask;
    }
    /* otherwise irq belongs to slave pic and then: */
    else {
        port = SLAVE_DATA_PORT;
		irq_num -= 8;
        /* setting the bit corresponding to the irq number given to 0 in pic mask */
		val = 1 << irq_num;
        slave_mask = ( slave_mask&(~val) );
        /*updating slave mask */
		mask = slave_mask;
        
    }  

    /* writing new mask to the pic */
    outb(mask, port);
}


/*
 * disable_irq
 * DESCRIPTION: disables a specific irq number on the pics
 * INPUTS: irq number to disable
 * OUTPUTS: irq line corresponding to the given irq number is masked
 * RETURN VALUE: none
 * SIDE EFFECTS: current state of the pic mask(s) is updated
 */
void disable_irq(uint32_t irq_num) {
    uint8_t val;    /* variable to hold the bit mask representing which irq line is on */
    uint16_t port;  /* port at which the irq number is sent */
    uint8_t mask;   /* to store the previous pic mask */

    /* if irq belongs to master pic, then: */
    if (irq_num < 8) {
        port = MASTER_DATA_PORT;
        /* setting the bit corresponding to the irq number given to 1 in pic mask */
		val = 1 << irq_num;
		master_mask = master_mask | val;
        /*updating master mask */
        mask = master_mask;
    }
    /* otherwise irq belongs to slave pic and then: */
    else {
        port = SLAVE_DATA_PORT;
        irq_num -= 8;
        /* setting the bit corresponding to the irq number given to 1 in pic mask */
    	val = 1 << irq_num;
		slave_mask = slave_mask | val;
        /*updating slave mask */
		mask = slave_mask;
    }   

    /* writing new mask to the pic */
    outb(mask, port);
}


/*
 * send_eoi
 * DESCRIPTION: sends an eoi signal to the pic corresponding 
 *              to the given irq number
 * INPUTS: irq number to which the eoi correponds to
 * OUTPUTS: eoi signal is given to the pic
 * RETURN VALUE: none
 * SIDE EFFECTS: pic unmasks all interrupts of lower priority 
 *               and other interrupts are now allowed
 */
void send_eoi(uint32_t irq_num) {
    uint16_t port;  /* port at which the irq number is sent */
    uint8_t eoi;    /* variable to hold the correct eoi signal */

    /* if irq belongs to master pic, then: */
    if (irq_num < 8) {
        port = MASTER_COM_PORT;
    }
    /* otherwise irq belongs to slave pic and then: */
    else {
        irq_num -= 8;   /* not needed */
        port = SLAVE_COM_PORT;
        /* first send the eoi to the master pic on line 2, the irq 
           line slave pic is connected to on master */
        outb(EOI | 2, MASTER_COM_PORT);
    }

    /* update the eoi to send based on the irq number */
    eoi = EOI | irq_num;

    /* send an eoi for the current interrupt at the correct port */
    outb(eoi, port);
}

