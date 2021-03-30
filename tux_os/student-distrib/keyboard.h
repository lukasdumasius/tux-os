#include "types.h"
#include "i8259.h"

/* initializes the keyboard */
void keyboard_init();

/* handles keyboard interrupts */
void keyboard_handler();

/* handles the backspace key output on console */
void backspace_case();

/* handles the tab key output */
void tab_case();

