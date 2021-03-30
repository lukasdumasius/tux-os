#include "keyboard.h"
#include "types.h"
#include "i8259.h"
#include "lib.h"
#include "terminal.h"
#include "system_call.h"

/* keyboard ports and irq number */
#define KB_DATA_PORT 0x60
#define KB_COM_PORT 0X64
#define KB_IRQ 1

/* scancodes for special keys which update keyboard state */
#define R_SHIFT_PRESSED 0x36
#define R_SHIFT_RELEASED 0xB6
#define L_SHIFT_PRESSED 0x2A
#define L_SHIFT_RELEASED 0xAA
#define CAPS_PRESSED 0x3A
#define CTRL_PRESSED 0x1D
#define CTRL_RELEASED 0x9D
#define ALT_PRESSED 0x38
#define ALT_RELEASED 0xB8

/* function keys */
#define F1 0x3B
#define F2 0x3C
#define F3 0x3D

/* further special keys that don't change the keyboard state */
#define BACKSPACE 0x0E
#define TAB 0x0F
#define SPACE 0x39
#define EXTENDED 0xE0

/* scancodes for bound checks and special clear key */
#define MIN_CODE 0x02
#define MAX_CODE 128
#define L_CODE 0x26
#define C_CODE 0x2E
#define ENTER_CODE 0x1C

/* array second index for current kbd state */
#define NORMAL 0
#define SHIFT 1
#define CAPS 2
#define SHIFT_CAPS 3

/* buffer size for keyboard and terminal */
#define BUF_SIZE 128

/* global variables to hold state of the keyboard in current session */
/* for all the flags, 1 is on, 0 is off */
static uint8_t kbd_buf[3][BUF_SIZE];
static int curr_i[3];
static int shift_flag;
static int ctrl_flag;
static int alt_flag;
static int caps_flag;
static int caps_number;
static int letters[3];

/* the array to hold all scancode mappings for the keys pressed */
static uint8_t kbd[4][128] = {
    {
        /* normal mode, no shift/caps pressed */
        0, 27, '1', '2', '3', '4', '5', '6', '7', '8',    /* 9 */
        '9', '0', '-', '=', '\b',                         /* Backspace */
        '\t',                                             /* Tab */
        'q', 'w', 'e', 'r',                               /* 19 */
        't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',     /* Enter key */
        0,                                                /* 29   - Control */
        'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', /* 39 */
        '\'', '`', 0,                                     /* Left shift */
        '\\', 'z', 'x', 'c', 'v', 'b', 'n',               /* 49 */
        'm', ',', '.', '/', 0,                            /* Right shift */
        '*',
        0,   /* Alt */
        ' ', /* Space bar */
        0,   /* Caps lock */
        0,   /* 59 - F1 key ... > */
        0, 0, 0, 0, 0, 0, 0, 0,
        0, /* < ... F10 */
        0, /* 69 - Num lock*/
        0, /* Scroll Lock */
        0, /* Home key */
        0, /* Up Arrow */
        0, /* Page Up */
        '-',
        0, /* Left Arrow */
        0,
        0, /* Right Arrow */
        '+',
        0, /* 79 - End key*/
        0, /* Down Arrow */
        0, /* Page Down */
        0, /* Insert Key */
        0, /* Delete Key */
        0, 0, 0,
        0, /* F11 Key */
        0, /* F12 Key */
        0, /* All other keys are undefined */
    },

    {
        /* shift key only pressed */
        0, 27, '!', '@', '#', '$', '%', '^', '&', '*',    /* 9 */
        '(', ')', '_', '+', '\b',                         /* Backspace */
        '\t',                                             /* Tab */
        'Q', 'W', 'E', 'R',                               /* 19 */
        'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n',     /* Enter key */
        0,                                                /* 29   - Control */
        'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', /* 39 */
        '"', '~', 0,                                      /* Left shift */
        '|', 'Z', 'X', 'C', 'V', 'B', 'N',                /* 49 */
        'M', '<', '>', '?', 0,                            /* Right shift */
        '*',
        0,   /* Alt */
        ' ', /* Space bar */
        0,   /* Caps lock */
        0,   /* 59 - F1 key ... > */
        0, 0, 0, 0, 0, 0, 0, 0,
        0, /* < ... F10 */
        0, /* 69 - Num lock*/
        0, /* Scroll Lock */
        0, /* Home key */
        0, /* Up Arrow */
        0, /* Page Up */
        '-',
        0, /* Left Arrow */
        0,
        0, /* Right Arrow */
        '+',
        0, /* 79 - End key*/
        0, /* Down Arrow */
        0, /* Page Down */
        0, /* Insert Key */
        0, /* Delete Key */
        0, 0, 0,
        0, /* F11 Key */
        0, /* F12 Key */
        0, /* All other keys are undefined */
    },

    {
        /* caps key only pressed */
        0, 27, '1', '2', '3', '4', '5', '6', '7', '8',    /* 9 */
        '9', '0', '-', '=', '\b',                         /* Backspace */
        '\t',                                             /* Tab */
        'Q', 'W', 'E', 'R',                               /* 19 */
        'T', 'Y', 'U', 'I', 'O', 'P', '[', ']', '\n',     /* Enter key */
        0,                                                /* 29   - Control */
        'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ';', /* 39 */
        '\'', '`', 0,                                     /* Left shift */
        '\\', 'Z', 'X', 'C', 'V', 'B', 'N',               /* 49 */
        'M', ',', '.', '/', 0,                            /* Right shift */
        '*',
        0,   /* Alt */
        ' ', /* Space bar */
        0,   /* Caps lock */
        0,   /* 59 - F1 key ... > */
        0, 0, 0, 0, 0, 0, 0, 0,
        0, /* < ... F10 */
        0, /* 69 - Num lock*/
        0, /* Scroll Lock */
        0, /* Home key */
        0, /* Up Arrow */
        0, /* Page Up */
        '-',
        0, /* Left Arrow */
        0,
        0, /* Right Arrow */
        '+',
        0, /* 79 - End key*/
        0, /* Down Arrow */
        0, /* Page Down */
        0, /* Insert Key */
        0, /* Delete Key */
        0, 0, 0,
        0, /* F11 Key */
        0, /* F12 Key */
        0, /* All other keys are undefined */
    },

    {
        /* both shift and caps keys are pressed */
        0, 27, '!', '@', '#', '$', '%', '^', '&', '*',    /* 9 */
        '(', ')', '_', '+', '\b',                         /* Backspace */
        '\t',                                             /* Tab */
        'q', 'w', 'e', 'r',                               /* 19 */
        't', 'y', 'u', 'i', 'o', 'p', '{', '}', '\n',     /* Enter key */
        0,                                                /* 29   - Control */
        'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ':', /* 39 */
        '"', '~', 0,                                      /* Left shift */
        '|', 'z', 'x', 'c', 'v', 'b', 'n',                /* 49 */
        'm', '<', '>', '?', 0,                            /* Right shift */
        '*',
        0,   /* Alt */
        ' ', /* Space bar */
        0,   /* Caps lock */
        0,   /* 59 - F1 key ... > */
        0, 0, 0, 0, 0, 0, 0, 0,
        0, /* < ... F10 */
        0, /* 69 - Num lock*/
        0, /* Scroll Lock */
        0, /* Home key */
        0, /* Up Arrow */
        0, /* Page Up */
        '-',
        0, /* Left Arrow */
        0,
        0, /* Right Arrow */
        '+',
        0, /* 79 - End key*/
        0, /* Down Arrow */
        0, /* Page Down */
        0, /* Insert Key */
        0, /* Delete Key */
        0, 0, 0,
        0, /* F11 Key */
        0, /* F12 Key */
        0, /* All other keys are undefined */
    }

};

/*
 * keyboard_init
 * DESCRIPTION: initializes the keyboard
 * INPUTS: none
 * OUTPUTS: none
 * RETURN VALUE: none
 * SIDE EFFECTS: enables the irq line 1 on the master pic
 */
void keyboard_init()
{
  /* enable the irq line on the pic for the keyboard */
  enable_irq(KB_IRQ);

  /* initializing state of the keyboard in memory */
  shift_flag = 0;
  caps_flag = 0;
  caps_number = 0;
  ctrl_flag = 0;
  alt_flag = 0;
  int i;
  for (i = 0; i < 3; i++)
  {
    curr_i[i] = 0;
    letters[i] = 0;
  }
}

/*
 * keyboard_handler
 * DESCRIPTION: handles interrupts from the keyboard
 * INPUTS: none
 * OUTPUTS: the key pressed
 * RETURN VALUE: none
 * SIDE EFFECTS: prints a character to the console
 */
void keyboard_handler()
{
  // cli();

  /* send eoi to the processor via pic */
  send_eoi(KB_IRQ);

  int code;  /* variable to hold the code sent by the keyboard */
  char ch;   /* variable to hold the ascii representation of the code */
  int index; /* variable to hold the second dimension index of the kbd array */
  int kf;    /* variable to hold flag for if any special key pressed */
  int j;     /* iterates thru enter case, kbd buf */
  /* initialize special keys pressed to none */
  kf = 0;

  /* initialize curr_i index to the value held in terminal buffer index */
  curr_i[terminal_active_id] = line_buffer_location[terminal_active_id];

  /* get the code from the keyboard */
  code = inb(KB_DATA_PORT);

  /* 
   * all cases for the special flags keys which update keyboard 
   * status or perfrom a specific operation 
   */
  switch (code)
  {

  /* cases when a shift key is pressed */
  case L_SHIFT_PRESSED:
    shift_flag = 1;
    kf = 1;
    break;

  case R_SHIFT_PRESSED:
    shift_flag = 1;
    kf = 1;
    break;

  /* cases when a shift key is released */
  case L_SHIFT_RELEASED:
    shift_flag = 0;
    break;

  case R_SHIFT_RELEASED:
    shift_flag = 0;
    break;

  /* case when the caps lock is pressed */
  case CAPS_PRESSED:
    caps_number++;
    caps_flag = caps_number % 2;
    break;

  /* case when the ctrl key is pressed */
  case CTRL_PRESSED:
    ctrl_flag = 1;
    kf = 1;
    break;

  /* case when the ctrl key is released */
  case CTRL_RELEASED:
    ctrl_flag = 0;
    break;

  /* case when the alt key is pressed */
  case ALT_PRESSED:
    alt_flag = 1;
    kf = 1;
    break;

  /* case when the alt key is released */
  case ALT_RELEASED:
    alt_flag = 0;
    break;

  /* case when the backspace key is pressed */
  case BACKSPACE:
    backspace_case();
    kf = 1;
    break;

  /* case when the tab key is pressed */
  case TAB:
    tab_case();
    kf = 1;
    break;

  /* case when a key not mapped is pressed */
  case EXTENDED:
    kf = 1;
    break;
  /* if none of the cases match then no special key is 
       pressed continue to the regular keyboard operation */
  default:
    break;
  };

  /* check which flags are on and determine the kbd array to use */
  if ((shift_flag == 1) && (caps_flag == 1))
  {
    index = SHIFT_CAPS;
  }
  else if ((shift_flag == 1) && (caps_flag == 0))
  {
    index = SHIFT;
  }
  else if ((shift_flag == 0) && (caps_flag == 1))
  {
    index = CAPS;
  }
  else
  {
    index = NORMAL;
  }

  /* 
   * check if the key pressed is a printable key 
   * if it is, print the char and add it into the buffer
   */
  if ((code >= MIN_CODE) && (code <= MAX_CODE))
  {

    /* 
     * another special case: 
     * if ctrl key is pressed with letter 'L' key
     * clear the screen 
     */
    if ((ctrl_flag == 1) && ((code == L_CODE) || (code == C_CODE)))
    {
      if (code == L_CODE)
      {
        /* clear the terminal screen */
        terminal_clear_and_reset();
      }
      if (code == C_CODE && helper_pass_pid() > 1)
      {
        // Ctrl+c to halt process
        /* halt the current process */
        halt(1);
      }
    }

    /*
     * another special case:
     * alt + F1 or F2 or F3 keys
     * switches the terminal
     */
    else if ((alt_flag == 1) && ((code == F1) || (code == F2) || (code == F3)))
    {
      int terminalID;
      switch (code)
      {
      case F1:
        terminalID = 0;
        break;
      case F2:
        terminalID = 1;
        break;
      case F3:
        terminalID = 2;
        break;
      default:
        break;
      };
      // printf("reache %d", terminalID);
      terminal_switch(terminalID);
    }

    /* otherwise regular printing of char */
    else if (kf == 0)
    {

      /* increment letter counter */
      letters[terminal_active_id]++;

      /* get the currently typed character */
      ch = kbd[index][code];

      /* only add to buffer if within the buffer limits */
      if (curr_i[terminal_active_id] < BUF_SIZE - 1)
      {
        kbd_buf[terminal_active_id][curr_i[terminal_active_id]] = ch;
        terminal_line_buffer[terminal_active_id][curr_i[terminal_active_id]] = kbd_buf[terminal_active_id][curr_i[terminal_active_id]];
        curr_i[terminal_active_id]++;
        line_buffer_location[terminal_active_id] = curr_i[terminal_active_id];
        printf("%c", ch);
      }
      if ((curr_i[terminal_active_id] == BUF_SIZE - 1) && code == ENTER_CODE)
      {
        kbd_buf[terminal_active_id][curr_i[terminal_active_id]] = ch;
        terminal_line_buffer[terminal_active_id][curr_i[terminal_active_id]] = kbd_buf[terminal_active_id][curr_i[terminal_active_id]];
        curr_i[terminal_active_id]++;
        line_buffer_location[terminal_active_id] = curr_i[terminal_active_id];
        printf("%c", ch);
      }

      if (code == ENTER_CODE)
      {
        for (j = 0; j < BUF_SIZE; j++)
        {
          kbd_buf[terminal_active_id][j] = 0;
        }
        curr_i[terminal_active_id] = 0;
      }

      /* print the typed char to the console regardless if it 
         was added to the terminal buffer or not */
    }
  }
  // sti();
}

/* 
 * backspace_case
 * DESCRIPTION: handles the backspace key output
 * INPUTS: none
 * OUTPUTS: previous key on console deleted, and last entry 
 *          in terminal buffer cleared
 * RETURN VALUE: none
 * SIDE EFFECTS: none
 */
void backspace_case()
{
  /* only output anything if the currently typed line has some 
     character(s) typed */
  if (curr_i[terminal_active_id] != 0)
  {

    /* update buffer index */
    /* then update buffer elements */
    curr_i[terminal_active_id]--;
    kbd_buf[terminal_active_id][curr_i[terminal_active_id]] = 0;
    terminal_line_buffer[terminal_active_id][curr_i[terminal_active_id]] = kbd_buf[terminal_active_id][curr_i[terminal_active_id]];
    line_buffer_location[terminal_active_id] = curr_i[terminal_active_id];

    /* delete previous char on the console */
    delete_c();

    /* decrement the count of characters typed */
    letters[terminal_active_id]--;
  }
}

/* 
 * tab_case
 * DESCRIPTION: handles the tab key output
 * INPUTS: none
 * OUTPUTS: tab output displayed on console, tab characters 
 *          added to terminal buffer
 * RETURN VALUE: none
 * SIDE EFFECTS: none
 */
void tab_case()
{
  /* tab is just 4 space characters one after the other */
  /* get the space character */
  uint8_t ch = kbd[NORMAL][SPACE];
  int i; /* index for writing 4 space characters */

  /* do four iterations to write 4 space characters to the 
     terminal buffer and console */
  for (i = 0; i < 4; i++)
  {

    /* check for buffer bounds, if outside of bounds then stop writing to teh terminal */
    if (curr_i[terminal_active_id] >= BUF_SIZE - 1)
    {
      break;
    }

    /* update the buffer elements */
    kbd_buf[terminal_active_id][curr_i[terminal_active_id]] = ch;
    terminal_line_buffer[terminal_active_id][curr_i[terminal_active_id]] = kbd_buf[terminal_active_id][curr_i[terminal_active_id]];

    /* update the buffer index */
    curr_i[terminal_active_id]++;
    line_buffer_location[terminal_active_id] = curr_i[terminal_active_id];

    /* update the typed character count */
    letters[terminal_active_id]++;
  }

  /* print the tab to the cconsole */
  for (i = 0; i < 4; i++)
  {

    /* print a space char to the console */
    putc(ch);
  }
}
