#include "types.h"

extern int pit_count;

extern int test_count;

/* initializes the pit */
void pit_init();

/* handles pit interrupts and calls scheduler */
void pit_irq_handler();
