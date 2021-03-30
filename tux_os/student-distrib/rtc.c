#include "rtc.h"
#include "i8259.h"
#include "lib.h"
#include "system_call.h"

/* RTC ports */
#define RTC_INDEX_PORT 0x70
#define RTC_R_W_PORT 0x71

/* RTC register offsets/numbers */
#define REG_A_OFFSET 0x0A
#define REG_B_OFFSET 0x0B
#define REG_C_OFFSET 0x0C

/* disable and enable NMI bit masks */
#define DISABLE_NMI 0x80
#define ENABLE_NMI 0X7F

/* RTC irq number */
#define RTC_IRQ 8

/* size of ints in number of bytes */
#define INT_SIZE 4

/* max allowed frequency of the rtc */
#define MAX_RTC_FREQ 1024

/* actual rtc freq */
#define RTC_FREQ 8192

/* rtc constant specified by the hardware, used in calculating 
   frequency based on 4 least significant bits of reg A */
#define PRESET_RTC_CONST 32768

/* max number of process */
#define MAX_PROCESSES 6

/* array to store virtual frequencies of each process */
static int virtual_freq[MAX_PROCESSES];

/* flag for rtc_read */
static int rtc_read_flag = 1;

/* flag for rtc testing */
static int test_flag = 0;

/*
 * rtc_irq_handler
 * DESCRIPTION: handles interrupts from the rtc
 * INPUTS: none
 * OUTPUTS: for cp1, a visible change on console(test), else none
 * RETURN VALUE: none
 * SIDE EFFECTS: causes changes on console, updates the rtc_read flag
 */
void rtc_irq_handler()
{
    /* update the read flag to show that next interrupt has been recieved */
    rtc_read_flag = 1;
    cli();                              /* clear all interrupts */
    outb(REG_C_OFFSET, RTC_INDEX_PORT); /* select cmos reg C */
    inb(RTC_R_W_PORT);                  /* get the value from reg C to keep getting interrupts */

    /* cp 2 test for rtc */
    //if (test_flag == 1) {
    //    printf("! ");
    //}

    send_eoi(RTC_IRQ); /* send eoi signal to the processor via the pic */
    sti();             /* set all interrupts again */
}

/*
 * set_rate
 * DESCRIPTION: initializes the rtc rate to the given argument
 * INPUTS: none
 * OUTPUTS: rtc rate is now changed
 * RETURN VALUE: none
 * SIDE EFFECTS: none
 */
void set_rate(int rate)
{
    uint8_t prev; /* temp variable to hold previous value of reg A to 
                       preserve top four bits */
    int r;        /* number to send to the cmos reg A which will set the 
                       correct frequency for the rtc */

    /* loop through values 3 to 15 to find the correct number to send to reg A */
    for (r = 3; r < 16; r++)
    {

        /* the correct number is the one which gives true to the following operation */
        if ((PRESET_RTC_CONST >> (r - 1)) == rate)
        {
            break;
        }
    }

    /* not needed but just a safeguard to ensure only 4 least significant 
       bits of reg A are changed */
    r &= 0x0F;
    // while(1){
    //     printf("r: %d \n", r);
    // }

    outb(DISABLE_NMI | REG_A_OFFSET, RTC_INDEX_PORT); /* select cmos reg A */
    prev = inb(RTC_R_W_PORT);                         /* get previous value of reg A */
    outb(DISABLE_NMI | REG_A_OFFSET, RTC_INDEX_PORT); /* select reg A again */
    outb((prev & 0xF0) | r, RTC_R_W_PORT);            /* write the new rate to the register */
}

/*
 * rtc_init
 * DESCRIPTION: initializes the rtc
 * INPUTS: none
 * OUTPUTS: none
 * RETURN VALUE: none
 * SIDE EFFECTS: enables the irq line 0 on the slave pic
 */
void rtc_init()
{
    char prev;                                        /* variable to hold the previous value of cmos reg B */
    cli();                                            /* clear all interrupts */
    outb(DISABLE_NMI | REG_B_OFFSET, RTC_INDEX_PORT); /* select reg B and disable NMI */
    prev = inb(RTC_R_W_PORT);                         /* get the previous value of reg B */
    outb(DISABLE_NMI | REG_B_OFFSET, RTC_INDEX_PORT); /* reset the index, a read will reset the index value to D */
    outb(prev | 0x40, RTC_R_W_PORT);                  /* write the prev value ORed with 0x40 to turn on the 6th bit of reg B */
    enable_NMI();                                     /* enable NMI again */
    enable_irq(RTC_IRQ);                              /* enable the rtc irq line on the pic */
    sti();                                            /* set all interrupts again */
    outb(REG_C_OFFSET, RTC_INDEX_PORT);               /* select cmos reg C */
    inb(RTC_R_W_PORT);                                /* get the value from reg C to keep getting interrupts */

    /* set the rtc rate to max */
    set_rate(RTC_FREQ);
}

/*
 * enable_NMI
 * DESCRIPTION: re-enables NMI
 * INPUTS: none
 * OUTPUTS: enables NMI via cmos port
 * RETURN VALUE: none
 * SIDE EFFECTS: none
 */
void enable_NMI()
{
    /* turn off the MSB and send to port 0x70 */
    outb(inb(RTC_INDEX_PORT) & ENABLE_NMI, RTC_INDEX_PORT);
}

/*
 * disable_NMI
 * DESCRIPTION: disenables NMI
 * INPUTS: none
 * OUTPUTS: disables NMI via cmos port
 * RETURN VALUE: none
 * SIDE EFFECTS: none
 */
void disable_NMI()
{
    /* turn on the MSB and send to port 0x70 */
    outb(inb(RTC_INDEX_PORT) | DISABLE_NMI, RTC_INDEX_PORT);
}

/*
 * rtc_open
 * DESCRIPTION: initializes the rtc frequency to 2 Hz
 * INPUTS: ignored
 * OUPTUTS: rtc frequency is changed to 2 Hz
 * RETURN VALUE: 0 (for success, is default)
 * SIDE EFFECTS: none
 */
int32_t rtc_open(const uint8_t *filename)
{
    /* set virtual frequency to 2Hz */
    virtual_freq[helper_pass_pid()] = 2;

    /* start testing */
    test_flag = 0;

    /* default return value */
    return 0;
}

/*
 * rtc_read
 * DESCRIPTION: keeps the user program suspended until next rtc
 *              interrupt is received
 * INPUTS: ignored
 * OUPTUTS: none
 * RETURN VALUE: 0 (for success, is default)
 * SIDE EFFECTS: user program is suspended for a short time
 */
int32_t rtc_read(int32_t fd, void *buf, int32_t nbytes)
{
    /* initialize the rtc_read flag to indicate that a rtc 
       interrupt has not been recieved yet */
    rtc_read_flag = 0;

    /* loop to wait until the next interrupt has been recieved */
    while (!rtc_read_flag)
        ;

    /* return 0 as succes/default */
    return 0;
}

/*
 * rtc_write
 * DESCRIPTION: sets the frequency rate of the rtc
 * INPUTS: fd, nbytes are ignored
 *         buf is a pointer to an int which holds the integer 
 *         rate to which the rtc frequency has to be set
 * OUPTUTS: rtc frequency rate is set, if all criterias are met
 * RETURN VALUE: 0 for success, -1 for failure
 * SIDE EFFECTS: none
 */
int32_t rtc_write(int32_t fd, const void *buf, int32_t nbytes)
{
    /* checking if the arguments given are valid */
    /* return failure if args are invalid */
    if ((buf == NULL) || (nbytes < INT_SIZE))
    {
        return -1;
    }

    /* variable to hold the rate to be written to the rtc */
    int rate;

    /* get a temp buffer to avoid void ptr warnings and errors */
    uint32_t *temp;
    temp = (uint32_t *)buf;

    /* getting the rate */
    rate = *(uint32_t *)buf;

    /* checking if the given rate is a power of 2 */
    if ((rate & (rate - 1)) == 0)
    {

        /* checking to ensure the rate written to the rtc is never larger than 1024 Hz */
        if (rate > MAX_RTC_FREQ)
        {
            /* write failed if so; return failure */
            //return -1;
            /* for cp2 test only */
            rate = MAX_RTC_FREQ;
        }

        /* if so, then set the rate */
        set_rate(rate);

        /* for the cp2 test: */
        //printf("actual frequency: %d Hz\n", rate);

        /* return success */
        return 0;
    }

    /* for cp2 testing, if given frequency is not a power of 2 */
    else
    {
        printf("given frequency is not a power of two :(\n");
        printf("frequency unchanged\n");
    }

    /* if given rate was not a power of two, return failure */
    return -1;
}

/*
 * rtc_virtual_write
 * DESCRIPTION: sets the frequency rate of the rtc for that specific process
 * INPUTS: fd, nbytes are ignored
 *         buf is a pointer to an int which holds the integer 
 *         rate to which the rtc frequency has to be set
 * OUPTUTS: rtc virtual_frequency rate is saved, if all criterias are met
 * RETURN VALUE: 0 for success, -1 for failure
 * SIDE EFFECTS: none
 */
int32_t rtc_virtual_write(int32_t fd, const void *buf, int32_t nbytes)
{
    /* checking if the arguments given are valid */
    /* return failure if args are invalid */
    if ((buf == NULL) || (nbytes < INT_SIZE))
    {
        return -1;
    }

    /* variable to hold the rate to be written to the rtc */
    int rate;

    /* getting the rate */
    rate = *(uint32_t *)buf;

    /* checking if the given rate is a power of 2 */
    if ((rate & (rate - 1)) == 0)
    {

        /* checking to ensure the rate written to the rtc is never larger than 1024 Hz */
        if (rate > MAX_RTC_FREQ)
        {
            /* write failed if so; return failure */
            //return -1;
            /* for cp2 test only */
            rate = MAX_RTC_FREQ;
        }

        /* if so, then set the rate */
        int process_id = helper_pass_pid();
        virtual_freq[process_id] = rate;

        /* for the cp2 test: */
        //printf("actual frequency: %d Hz\n", rate);

        /* return success */
        return 0;
    }

    /* for cp2 testing, if given frequency is not a power of 2 */
    else
    {
        printf("given frequency is not a power of two :(\n");
        printf("frequency unchanged\n");
    }

    /* if given rate was not a power of two, return failure */
    return -1;
}

/*
 * rtc__virtual-read
 * DESCRIPTION: keeps the user program suspended until next rtc
 *              interrupt is received
 * INPUTS: ignored
 * OUPTUTS: none
 * RETURN VALUE: 0 (for success, is default)
 * SIDE EFFECTS: user program is suspended for a short time
 */
int32_t rtc_virtual_read(int32_t fd, void *buf, int32_t nbytes)
{
    /* intitialize the number of times rtc read has to be called for
       the virtual frequency of the current program */
    int32_t counter = MAX_RTC_FREQ / virtual_freq[helper_pass_pid()];

    /* call rtc_read the above number of times */
    while (counter)
    {
        rtc_read(fd, buf, nbytes);
        counter--;
    }

    /* return 0 as succes/default */
    return 0;
}

/*
 * rtc_close
 * DESCRIPTION: closes rtc file, does nothing
 * INPUTS: ignored
 * OUPTUTS: none
 * RETURN VALUE: 0
 * SIDE EFFECTS: none
 */
int32_t rtc_close(int32_t fd)
{
    /* set virtual frequecy to 2 Hz again */
    virtual_freq[helper_pass_pid()] = 2;

    /* default return */
    return 0;
}

/*
 * test_rtc
 * DESCRIPTION: tests rtc functionality
 * INPUTS: none
 * OUPTUTS: changes rate of the rtc multiple times
 * RETURN VALUE: 0
 * SIDE EFFECTS: none
 */
void test_rtc()
{
    /* variable to hold the rtc rate to be set to */
    int rate = 2;

    /* open rtc file */
    rtc_open((uint8_t *)"rtc");

    /* index variable */
    int i;

    /* carry out test in infinite loop */
    while (1)
    {

        /* print the current frequency of the rtc to the console */
        printf("frequency: %d Hz\n", rate);

        /* change the frequency to aformentioned rate */
        rtc_virtual_write(1, &rate, 4);

        /* carry out the rtc test 20 times for the current frequency */
        for (i = 0; i < 20; i++)
        {
            /* to ensure 20 interrupts on each frequency */
            /* use rtc_read */
            rtc_virtual_read(1, &rate, 4);
            printf("! ");
        }

        /* updating the rate, it loops back to lowest possible after 
           the highest possible frequency is reached */
        if (rate == 8192)
        {

            /* after the highest possible frequency is reached start invalid args test */
            rate = 8;
            printf("\ntesting 20 invalid frequencies now\n");

            /* test 20 invalid args in a loop */
            for (i = 0; i < 20; i++)
            {
                printf("given frequency: %d Hz\n", rate);

                /* try to change frequency */
                rtc_virtual_write(1, &rate, 4);

                /* index counter */
                int j;

                /* wait for 30 interrupts */
                for (j = 0; j < 30; j++)
                {
                    rtc_virtual_read(1, &rate, 4);
                    printf("! ");
                }

                /* increase given rate in a way for it to never be a power of 2 */
                rate = rate + 159;

                /* newline for clarity */
                printf("\n");
            }

            printf("resuming the infinite power of 2 frequency tests\n");
            rate = 2;
        }
        else
        {
            rate = rate << 1;
        }

        /* go to newline for clarity of printing */
        printf("\n");
    }
}
