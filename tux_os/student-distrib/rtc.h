#include "types.h"

/* initializes rtc */
void rtc_init();

/* enables NMI */
void enable_NMI();

/* disables NMI */
void disable_NMI();

/* handles rtc interrupts */
void rtc_irq_handler();

/* sets the frequency of the rtc to slowest possible one */
void set_rate(int rate);

/* initializes the rtc frequency to 2 Hz */
int32_t rtc_open(const uint8_t *filename);

/* keeps the user program suspended until next rtc interrupt is received */
int32_t rtc_read(int32_t fd, void *buf, int32_t nbytes);

/* sets the frequency rate of the rtc */
int32_t rtc_write(int32_t fd, const void *buf, int32_t nbytes);

/* closes rtc file, does nothing */
int32_t rtc_close(int32_t fd);

/* rtc funcitonality tester function */
void test_rtc();

// Added below for Checkpoint 5 + final

/* See function interface for more details */
int32_t rtc_virtual_write(int32_t fd, const void *buf, int32_t nbytes);

/* See function interface for more details */
int32_t rtc_virtual_read(int32_t fd, void *buf, int32_t nbytes);
