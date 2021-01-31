/*
 * Implementing function to wait a precise amount of time
 * Created at 31/01/2021
 * Written by m3mory
 */

#include "gpio.h"

#define SYSTMR_LO       ((volatile unsigned int*)(MMIO_BASE+0x00003004))
#define SYSTMR_HI       ((volatile unsigned int*)(MMIO_BASE+0x00003008))

/*
 * Wait N CPU cycles (ARM CPU only)
 */
void wait_cycles(unsigned int n) {
    if (n) {
        while (n--) {
            asm volatile("nop");
        }
    }
}

/*
 * Wait N microsec (ARM CPU only)
 */
void wait_msec(unsigned int n) {
    register unsigned long freq, cnt, diff;
    // get current counter frequency
    asm volatile("mrs %0, cntfrq_el0" : "=r"(freq));
    // read current counter
    asm volatile("mrs %0, cntpct_el0" : "=r"(cnt));
    // compute expire value for counter
    cnt += ((freq / 1000) * n) / 1000;
    do {
        asm volatile("mrs %0, cntpct_el0" : "=r"(diff));
    } while ( diff < cnt );
}

/*
 * Get system Timer's counter
 */
unsigned long get_system_timer() {
    unsigned int high = -1;
    unsigned int low;
    // Read MMIO area as 2 separat 32 bit reads
    high = *SYSTMR_HI;
    low = *SYSTMR_LO;

    // Repeat it if high word changed during read
    if (high != *SYSTMR_HI) {
        high = *SYSTMR_HI;
        low = *SYSTMR_LO;
    }

    return ((unsigned long) high << 32) | low;
}

/*
 * Wait N microsec (with BCM system timer)
 */
void wait_msec_st(unsigned int n) {
    unsigned long t = get_system_timer();
    /* We must chekc if it's non-zero, cuz qemu does not 
     * emulate system time, and returning constant zero would mean infinte loop
     */
    if (t) {
        while (get_system_timer() < t+n);
    }
}

