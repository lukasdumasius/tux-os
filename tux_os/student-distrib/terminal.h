#include "types.h"
#include "i8259.h"
#include "pit.h"
#include "x86_desc.h" //*** Added for Checkpoint 5: Multiple Terminals
                      //	  check back later if including this here
                      //	  causes issues

#define LINE_BUFFER_LEN 128
#define START 0
#define LAST_ROW_INDEX 24

#define CURSOR_PORT_1 0x3D4
#define CURSOR_PORT_2 0x3D5
#define CURSOR_DATA_1 0x0F
#define CURSOR_DATA_2 0x0E
#define LOW_BYTE_HIGH 0xFF
#define SCOOT_8 8

#define NEWLINE_CHAR '\n'
#define NULL_TERMINATOR '\0'

///////////////  These constants and global vars transferred from lib.c  ///////////////
#define ATTRIB 0x7
#define VIDEO 0xB8000
#define NUM_COLS 80
#define NUM_ROWS 25

/* defined in: lib.c */
extern int screen_x;
extern int screen_y;
extern char *video_mem;
/////////////  end: These constants and global vars transferred from lib.c  /////////////

/* defined in: terminal.c */
extern uint32_t line_buffer_location[3];                 // next location to write to in buffer
extern uint8_t terminal_line_buffer[3][LINE_BUFFER_LEN]; // *** idk if need LINE_BUFFER_LEN here
                                                         // me from future: yes, its needed

extern int terminal_flags[3];

//New structs/variables for Checkpoint 5, multiple terminals...
extern uint8_t terminal_active_id; /* defined in: terminal.c */
                                   // values should range from 0-2 only
                                   // probably a good idea to do this checking
                                   // if you take a terminal_id as arg

/* opens terminal, see function interface/header for more details */
int32_t terminal_open(const uint8_t filename);
/* closes terminal, see function interface/header for more details */
int32_t terminal_close(int32_t fd);
/* reads user input into terminal so far, see function interface/header for more details */
int32_t terminal_read(int32_t fd, void *buf, int32_t nbytes);
/* args under the hood:	 uint8_t * passed_buffer, int bytes_to_copy	 */
/* writes / prints to terminal screen, see function interface/header for more details */
int32_t terminal_write(int32_t fd, const void *buf, int32_t nbytes);
/* args under the hood: const char * passed_buffer, int bytes_to_write	*/

/* updates location on terminal, see function interface/header for more details */
int terminal_update_location(int x, int y);
/* updates just the cursor on terminal, see function interface/header for more details */
int terminal_update_cursor(int x, int y);
/* clears the terminal screen and resets cursor, see function interface/header for more details */
int terminal_clear_and_reset();
/* scrolls the terminal up, see function interface/header for more details */
int terminal_scroll_up();

// Added for Checkpoint 5, Multiple Terminals:
/* grabs current terminal location (x/y coords), see function interface/header for more details */
screen_coord_info_t terminal_get_location();
/* main functino used to switch to new terminal, see function interface/header for more details */
int terminal_switch(uint8_t terminal_id);
/* called in kernel.c to setup additional paging structures */
/* for backup/storage pages for multiple terminals, see function interface/header for more details */
int terminal_multiple_terminal_init_paging();
