/*
 * Power management functions for RaspberryPi 3
 * Created at 31/01/2021
 * Written by m3m0ry
 */

#include "gpio.h"
#include "uart.h"
#include "mbox.h"
#include "delays.h"

#define PM_RSTC         ((volatile unsigned int*)(MMIO_BASE+0x0010001c))
#define PM_RSTS         ((volatile unsigned int*)(MMIO_BASE+0x00100020))
#define PM_WDOG         ((volatile unsigned int*)(MMIO_BASE+0x00100024))
#define PM_WDOG_MAGIC   0x5a000000
#define PM_RSTC_FULLRST 0x00000020


void power_off() {
    unsigned long r;
    /* First power off all devices */
    for (r = 0; r < 16; r++) {
        mbox[0] = 8*4;
        mbox[1] = MBOX_REQUEST;
        mbox[2] = MBOX_TAG_SETPOWER;    // set power state
        mbox[3] = 8;                    // Values buffer size in bytes
        mbox[4] = 8;                    // Response length
        mbox[5] = (unsigned int)r;      // device id
        mbox[6] = 0;                    // bit 0:off, bit 1: no wait
        mbox[7] = MBOX_TAG_LAST;        // End of tag buffer
        mbox_call(MBOX_CH_PROP);
    }
    /* Power off gpio pins (but not VCC pins) */
    *GPFSEL0 = 0;
    *GPFSEL1 = 0;
    *GPFSEL2 = 0;
    *GPFSEL3 = 0;
    *GPFSEL4 = 0;
    *GPFSEL5 = 0;
    *GPPUD = 0;
    wait_cycles(150);
    *GPPUDCLK0 = 0xffffffff;
    *GPPUDCLK1 = 0xffffffff;
    wait_cycles(150);
    // Flush GPIO setup
    *GPPUDCLK0 = 0;
    *GPPUDCLK1 = 0;

    // power off the SoC (GPU + CPU)
    r = *PM_RSTS;
    r &= ~0xfffffaaa;
    r |= 0x555;     // partition 63 used to indicate halt

    *PM_RSTS = PM_WDOG_MAGIC | r;
    *PM_WDOG = PM_WDOG_MAGIC | 10;
    *PM_RSTC = PM_WDOG_MAGIC | PM_RSTC_FULLRST;
}

/*
 * Reboot
 */
void reset() {
    unsigned int r;
    pl011_uart_send('m');
    // Trigger a restart by instruction the GPU to boot from partition 0
    r = *PM_RSTS;
    r &= ~0xfffffaaa;

    *PM_RSTS = PM_WDOG_MAGIC | r; // boot from partition 0
    *PM_WDOG = PM_WDOG_MAGIC | 10;
    *PM_RSTC = PM_WDOG_MAGIC | PM_RSTC_FULLRST;
}
