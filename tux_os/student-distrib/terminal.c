#include "keyboard.h"
//#include "types.h"	// These two are included
//#include "i8259.h"	// in terminal.h
#include "lib.h"
#include "terminal.h"
#include "system_call.h"
#include "x86_desc.h" // *** Checkpoint 5 addition:
					  // added for:
					  // 1. access to screen_coord_storage[3]
					  // 2. also for access to paging structures
					  //	  for multiple terminal storage

uint32_t line_buffer_location[3]; //next location to write to in buffer

uint8_t terminal_line_buffer[3][LINE_BUFFER_LEN]; // line buffer that stores user input
												  // updated by keyboard.c with user input,
												  // but jointly maintained
uint8_t terminal_active_id = 0;					  // extern-ed in terminal.h
												  // #include that to access this

int terminal_flags[3] = {0, 1, 1};
int terminal_process[3] = {-1, -1, -1};
/*
 * int32_t terminal_open();
 * Inputs: const uint8_t filename
 * Return Value: 0 on success, -1 on failure
 * Function: prepares the terminal by initializing
 *			line buffer to empty and line buffer location [index]
 *			to 0.
 */
int32_t
terminal_open(const uint8_t filename)
{
	//arg ignored

	int i; //iterates through buffer
	for (i = 0; i < LINE_BUFFER_LEN; i++)
	{									//0 because array starts at index 0
		terminal_line_buffer[0][i] = 0; //empty line buffer=0
		terminal_line_buffer[1][i] = 0;
		terminal_line_buffer[2][i] = 0;
	}
	line_buffer_location[0] = 0; //0 because array starts at index 0
	line_buffer_location[1] = 0;
	line_buffer_location[2] = 0;
	return 0; //return sucess, so 0
}

/*
 * int32_t terminal_close();
 * Inputs: int32_t fd
 * Return Value: 0 on success, -1 on failure
 * Function: closes the terminal by setting
 *			line buffer to empty and line buffer location [index]
 *			to 0.
 */
int32_t terminal_close(int32_t fd)
{
	//arg ignored

	int i; //iterates through buffer
	for (i = 0; i < LINE_BUFFER_LEN; i++)
	{									//0 because array starts at index 0
		terminal_line_buffer[0][i] = 0; //empty line buffer=0
		terminal_line_buffer[1][i] = 0;
		terminal_line_buffer[2][i] = 0;
	}
	line_buffer_location[0] = 0; //0 because array starts at index 0
	line_buffer_location[1] = 0;
	line_buffer_location[2] = 0;
	return 0; //return sucess, so 0
}

/*
 * int32_t terminal_read();
 * Inputs: int32_t fd, void* buf, int32_t nbytes
 * Return Value: 0 on success, -1 on failure
 * Function: reads terminal line buffer into the user passed
 *			 buffer. By design reads up to end of line buffer or
 *			 however many bytes you asked for if its less.
 */
int32_t terminal_read(int32_t fd, void *buf, int32_t nbytes)
{ //maybe add copying a separate copy buffer for write
	/* args under the hood:	uint8_t * passed_buffer, int bytes_to_copy	 */
	if (buf == NULL)
	{
		return -1; //return failure no read bad pointer
	}

	uint8_t *passed_buffer = (uint8_t *)(buf);
	int bytes_to_copy = (int)(nbytes);
	// ignored fd

	int i; //used to iterate across buffer
	uint32_t temp_line_buffer_length;

	////////////// NEED TO MODIFIED LATER
	// pit_count = 0;

	uint8_t last_entry = terminal_line_buffer[pit_count][line_buffer_location[pit_count] - 1]; // -1 to check last element 				//*** patikrink 0 ilguma
	while (last_entry != NEWLINE_CHAR)
	{
		last_entry = terminal_line_buffer[pit_count][line_buffer_location[pit_count] - 1]; // -1 to check last element
	};																					   //block until enter pressed by user

	for (i = 0; (i < line_buffer_location[pit_count]) && (i < bytes_to_copy); i++)
	{ // 0 because array starts at index 0
		passed_buffer[i] = terminal_line_buffer[pit_count][i];
	}
	//printf("%d", i);
	temp_line_buffer_length = i; // grab how many bytes we printed

	//terminal_write(terminal_line_buffer, temp_line_buffer_length);

	line_buffer_location[pit_count] = 0; //0 because array starts at index 0
	for (i = 0; i < LINE_BUFFER_LEN; i++)
	{											// also clear buffer as in discussion, 0 because array starts at index 0
		terminal_line_buffer[pit_count][i] = 0; //  empty line buffer=0
	}

	return temp_line_buffer_length;
}

/*
 * int32_t terminal_write();
 * Inputs: int32_t fd, const void* buf, int32_t nbytes
 * Return Value: 0 on success, -1 on failure
 * Function: prints buffer to the terminal. modificiations to 
 *			 putc means we dont need to calculate new lines here.
 *			 can just send the entire string. Has some "smart" logic
 *			 to catch small user mistakes like null buffer pointer,
 *			 and if the previous user stopped printing without a newline.
 */
/*
* you dont need to do anything fancy like ending your string with a \0 or \n newline.
* Actually, it will overwrite this if you do, I think. and might print an extra
* newline. 
* Just pass a buffer full of chars and the bytes_to_write
* is how many characters will appear on screen.
*
* Example: buffer: ['h', 'e', 'l', 'l', 'o'], bytes_to_write: 5
*
*/
int32_t terminal_write(int32_t fd, const void *buf, int32_t nbytes)
{ //bytes_to_write is length
	/* args under the hood: const char * passed_buffer, int bytes_to_write	*/

	// printf("we have reached terminal write\n");
	const char *passed_buffer = (char *)(buf);
	int bytes_to_write = (int)(nbytes);
	// ignored fd

	if (buf == NULL)
	{
		return -1; //return failure no read bad pointer
	}

	int i; //tracks original passed buffer
	//int temp_passed_buffer_len;

	if (bytes_to_write < 1)
	{			  //1 because we need at least 1 byte to write to be valid
		return 0; // user passed us garbage input or is playing games with us
	}			  // anyways, return and tell user we copied nothing.
				  // mostly this is here to prevent trying to create
				  // an array with 0 elements in it
				  // return 0 because 0 bytes copied

	char temp_passed_buffer[bytes_to_write + 2]; //we make sure it can fit the bytes
												 //we're writing, and an extra newline ,and a null terminator
												 // 2 because need space for newline (potentially)
												 // and \0 terminator

	// removed to not start on fresh line every time
	/*
	if (screen_x != 0)
	{						// terminal_write() assumes that we are starting on a fresh line
		putc(NEWLINE_CHAR); // it is the responsibility of anyone writing to terminal through
	}						// terminal_write(), or any other function (putc) to end with a \n
							// but we do this failsafe anyway.
							// 0 because it is the first column (new line needed)


*/
	// end removed to not start on fresh line every time

	for (i = 0; i < bytes_to_write; i++)
	{ //0 because array starts at index 0
		temp_passed_buffer[i] = passed_buffer[i];
		putc(temp_passed_buffer[i]);
	}
	/*
	if(temp_passed_buffer[i-1]!=NEWLINE_CHAR){				// -1 because i points to index we write into NEXT
		temp_passed_buffer[i++]=NEWLINE_CHAR; 				// user sent us a string not terminating in newline.
															//  we'll fix this :)
	}
	*/
	// causes double space problems, removed for now
	// temp_passed_buffer[i++] = NULL_TERMINATOR; //terminate the string

	// printf("%s", temp_passed_buffer);
	return i;
}

/*
 * int terminal_update_location();
 * Inputs: int x, int y
 * Return Value: 0 on success, -1 on failure
 * Function: force resets cursor, 
 * 			does not force reset screen coords.
 */
int terminal_update_location(int x, int y)
{
	if ((x != screen_x) || (y != screen_y))
	{
		screen_x = x;
		screen_y = y;
	}
	terminal_update_cursor(screen_x, screen_y);

	return 0; //return success, so 0
}

/*
 * int terminal_update_cursor();
 * Inputs: int x, int y
 * Return Value: 0 on success, -1 on failure
 * Function: force update cursor location
 *			on terminal.
 */
int terminal_update_cursor(int x, int y)
{
	uint16_t pos = y * NUM_COLS + x;

	outb(CURSOR_DATA_1, CURSOR_PORT_1);
	outb((uint8_t)(pos & LOW_BYTE_HIGH), CURSOR_PORT_2);
	outb(CURSOR_DATA_2, CURSOR_PORT_1);
	outb((uint8_t)((pos >> SCOOT_8) & LOW_BYTE_HIGH), CURSOR_PORT_2);

	return 0; //return success, so 0
}

/*
 * int terminal_clear_and_reset();
 * Inputs: none
 * Return Value: 0 on success, -1 on failure
 * Function: force clear the terminal of
 *		     anything thats printed to screen 
 *			 and set cursor to the top left.
 */
int terminal_clear_and_reset()
{ //AKA, the "ctrl + L" function

	clear();
	screen_x = START;
	screen_y = START;
	terminal_update_cursor(screen_x, screen_y);
	return 0; //return success, so 0
}

/*
 * int terminal_scroll_up();
 * Inputs: none
 * Return Value: 0 on success, -1 on failure
 * Function: scroll the terminal up one line
 *			 when its called by putc. putc
 *			 determines when to scroll and calls this
 *			 to move everything up a line that appears
 *			 on screen.
 */
int terminal_scroll_up()
{

	int32_t i;
	for (i = 0; i < (NUM_ROWS - 1) * NUM_COLS; i++)
	{																									// 0 because array starts at index 0, -1 because we dont need to copy
																										// into last line
		*(uint8_t *)(video_mem + (i << 1)) = *(uint8_t *)(video_mem + ((i + NUM_COLS) << 1));			//1 because need to shift to calculate correct
																										// mem location. and add 1 to offset to attribute location.
		*(uint8_t *)(video_mem + (i << 1) + 1) = *(uint8_t *)(video_mem + (((i + NUM_COLS) << 1)) + 1); // probably = ATTRIB, but we
	}																									//  copy it over anyway from original
	for (i = (NUM_ROWS - 1) * NUM_COLS; i < NUM_ROWS * NUM_COLS; i++)
	{													 //   location
		*(uint8_t *)(video_mem + (i << 1)) = ' ';		 //1 because need to shift to calculate correct, and ' ' because need empty space
		*(uint8_t *)(video_mem + (i << 1) + 1) = ATTRIB; // mem location. and add 1 to offset to attribute location.
	}

	terminal_update_location(START, LAST_ROW_INDEX); //last row

	return 0; //return success, so 0
}

// Assumption: Cursor location same as
// screen coords
screen_coord_info_t terminal_get_location()
{
	screen_coord_info_t info;
	info.screen_x = screen_x;
	info.screen_y = screen_y;
	return info;
}

// Called from keyboard.c when ALT+F#
// is pressed requesting terminal switch
// to a different terminal

// args:
// terminal_id - number of terminal we
//				 are switching TO
// terminal ids go 0-2 (3 total)
// returns 0 on success
int terminal_switch(uint8_t terminal_id)
{
	uint32_t video_address_to_store;
	uint32_t video_address_to_load;

	void *video_address_to_store_ready;
	void *video_address_to_load_ready;
	void *video_address_main = (void *)(0xB8000);	//xb8000 - video mem

	if (terminal_id == terminal_active_id)
	{			  // if don't need to switch terminals
		return 0; // then just return success as we
				  // don't need to do anything
	}

	screen_coord_info_t old_screen_coords;
	screen_coord_info_t new_screen_coords;

	old_screen_coords = terminal_get_location();

	// store old_screen_coords
	screen_coord_storage[terminal_active_id] = old_screen_coords;

	// (pre- store video mem step )
	// calculate address where to store
	video_address_to_store = (0xB8000) + (terminal_active_id + 1) * (4 * (0x400)); // 0xB8000 = VIDEO MEM
																				   // 0x400   = 1 KB
																				   // 4*0x400 = 4 KB (size of one terminal mem)
	video_address_to_store_ready = (void *)video_address_to_store;
	// store video mem
	memcpy(video_address_to_store_ready, video_address_main, (4 * (0x400)));

	// load video mem
	video_address_to_load = (0xB8000) + (terminal_id + 1) * (4 * (0x400)); // 0xB8000 = VIDEO MEM
																		   // 0x400   = 1 KB
																		   // 4*0x400 = 4 KB (size of one terminal mem)
	video_address_to_load_ready = (void *)video_address_to_load;
	memcpy(video_address_main, video_address_to_load_ready, (4 * (0x400)));

	// load new_screen_coords
	new_screen_coords = screen_coord_storage[terminal_id]; // we load the coords here,
														   // and we can now safely set the cursor
														   // finalizing everything
	terminal_update_location(new_screen_coords.screen_x, new_screen_coords.screen_y);

	terminal_active_id = terminal_id; // now that we've switched over to this terminal,
									  // set it as the active one

	// if (terminal_id == 0)
	// {
	// 	printf("\n T0 has been reached \n");
	// }

	if (terminal_flags[terminal_active_id] == 1)
	{
		terminal_flags[terminal_active_id] = 0;

		sti();							// this ensure even though below execute() hangs, we can still interrupt with keyboard...
		pit_count = terminal_active_id; // this cuz in terminal_read(), pit_count depends on this number and would hang in while loop is NOT updated
		printf("Welcome to a new Terminal: T%d \n", terminal_active_id);
		execute((uint8_t *)"shell");
		// printf("reach a new terminal: %u \n", terminal_id);
	}
	pit_count = terminal_active_id;

	pcb_t *old_pcb = (pcb_t *)(MEM_8MB - (current_pid + 1) * MEM_8KB);
	asm volatile("movl %%esp, %0"
				 : "=r"(old_pcb->esp));
	asm volatile("movl %%ebp, %0"
				 : "=r"(old_pcb->ebp));

	// current_pid is the one next scheduled
	current_pid = terminal_process[pit_count];

	// update paging for current pid
	map_paging(current_pid);

	// tss.ss0 = KERNEL_DS;
	tss.esp0 = MEM_8MB - (current_pid)*MEM_8KB - 0x4; // we restore tss's kernel stack ptr to parent process

	// get current pid's pcb
	pcb_t *pcb = (pcb_t *)(MEM_8MB - (current_pid + 1) * MEM_8KB);

	// context switch
	asm volatile(
		"movl %0, %%esp   ;"
		"movl %1, %%ebp   ;"
		:
		: "r"(pcb->esp), "r"(pcb->ebp) // restore to the currently halting process's esp, ebp
		: "memory");				   // clobbered registers

	return 0; //return success, so 0

	return 0; //return success, so 0
}

// Called in kernel.c to setup 'backup' storage
// that is used for storing/writing to terminals
// currently not active (in view).
// Maps:
// Virtual xB9000 -> Physical xB9000
// Virtual xBA000 -> Physical xBA000
// Virtual xBB000 -> Physical xBB000
// returns 0 on success
int terminal_multiple_terminal_init_paging()
{
	// Added to iterate during setup
	// intitial coords at 0 (bottom):
	int i;

	// These definitions are all here just so we
	// can point to these data sections to memset so
	// we can make sure they're cleared out
	uint32_t term_0_address_temp = 0xB9000;		// xb9 4 kb after xb8
	uint32_t term_1_address_temp = 0xBA000;		// xba 4 kb after xb9
	uint32_t term_2_address_temp = 0xBB000;		// xbb 4 kb after xba

	void *term_0_address_temp_ready = (void *)term_0_address_temp;
	void *term_1_address_temp_ready = (void *)term_1_address_temp;
	void *term_2_address_temp_ready = (void *)term_2_address_temp;

	//     terminal 0 - xB9    //
	page_table[0xB9].present = 1;		  // this page is now present, so 1
	page_table[0xB9].read_write = 1;	  // because its present, we also want to read/write
	page_table[0xB9].user_supervisor = 0; // 0 for only supervisor/kernel access (us)
	page_table[0xB9].page_write_through = 0;
	page_table[0xB9].page_cashed_disabled = 0;
	page_table[0xB9].accessed = 0;
	page_table[0xB9].dirty = 0;
	page_table[0xB9].page_attribute_table = 0;
	page_table[0xB9].global_bit = 0;
	page_table[0xB9].available = 0;
	page_table[0xB9].page_base_addr = 0xB9; // Same mapping between virtual/physical
	//  fin - terminal 0  //

	//     terminal 1 - xBA    //
	page_table[0xBA].present = 1;		  // this page is now present, so 1
	page_table[0xBA].read_write = 1;	  // because its present, we also want to read/write
	page_table[0xBA].user_supervisor = 0; // 0 for only supervisor/kernel access (us)
	page_table[0xBA].page_write_through = 0;
	page_table[0xBA].page_cashed_disabled = 0;
	page_table[0xBA].accessed = 0;
	page_table[0xBA].dirty = 0;
	page_table[0xBA].page_attribute_table = 0;
	page_table[0xBA].global_bit = 0;
	page_table[0xBA].available = 0;
	page_table[0xBA].page_base_addr = 0xBA; // Same mapping between virtual/physical
	//  fin - terminal 1  //

	//     terminal 2  - xBB   //
	page_table[0xBB].present = 1;		  // this page is now present, so 1
	page_table[0xBB].read_write = 1;	  // because its present, we also want to read/write
	page_table[0xBB].user_supervisor = 0; // 0 for only supervisor/kernel access (us)
	page_table[0xBB].page_write_through = 0;
	page_table[0xBB].page_cashed_disabled = 0;
	page_table[0xBB].accessed = 0;
	page_table[0xBB].dirty = 0;
	page_table[0xBB].page_attribute_table = 0;
	page_table[0xBB].global_bit = 0;
	page_table[0xBB].available = 0;
	page_table[0xBB].page_base_addr = 0xBB; // Same mapping between virtual/physical
	//  fin - terminal 2  //

	// not 100% sure if we necessaily need to clear out these mem spaces
	// but I think we do and will anyway for safety...

	memset(term_0_address_temp_ready, 0, 4096);	//4096 = 4 kb
	memset(term_1_address_temp_ready, 0, 4096);
	memset(term_2_address_temp_ready, 0, 4096);

	//Added to setup intitial coords at 0:
	for (i = 0; i < 3; i++)
	{
		screen_coord_storage[i].screen_x = 0;	// top of screen
		screen_coord_storage[i].screen_y = 0;	// top of screen
	}

	return 0; //return success, so 0
}
