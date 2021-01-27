/*
 * Created at 27/01/2021
 * Written by m3m0ry
 */
#include "gpio.h"

/* message buffer */
volatile unsigned int __attribute__((aligned(16))) mbox[36];

#define VIDEOCORE_MBOX  (MMIO_BASE + 0x0000B880)
#define MBOX_READ       ((volatile unsigned int*)(VIDEOCORE_MBOX+0x0))
#define MBOX_POLL       ((volatile unsigned int*)(VIDEOCORE_MBOX+0x10))
#define MBOX_SENDER     ((volatile unsigned int*)(VIDEOCORE_MBOX+0x14))
#define MBOX_STATUS     ((volatile unsigned int*)(VIDEOCORE_MBOX+0x18))
#define MBOX_CONFIG     ((volatile unsigned int*)(VIDEOCORE_MBOX+0x1C))
#define MBOX_WRITE      ((volatile unsigned int*)(VIDEOCORE_MBOX+0x20))
#define MBOX_RESPONSE   0x80000000
#define MBOX_FULL       0x80000000
#define MBOX_EMPTY      0x40000000

/*
 * Make a mailbox call. Returns 0 on failure, non-zero on success
 */
int mbox_call(unsigned char ch) {
    unsigned int message_addr = ((((unsigned int)(unsigned long)&mbox) & ~0xF) | (ch & 0xF));

    /* Wait until we can write to the mailbox */
    do {
        asm volatile("nop");
    } while (*MBOX_STATUS & MBOX_FULL);

    /* Write the address of our message to the mailbox */
    *MBOX_WRITE = message_addr;

    /* Wait for the response */
    while(1) {
        do {
            asm volatile("nop");
        } while(*MBOX_STATUS & MBOX_EMPTY);
        /* is it a response to our message? */
        if (message_addr == *MBOX_READ)
            return mbox[1]==MBOX_RESPONSE;
    }
    return 0;
}
