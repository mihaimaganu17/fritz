#include "uart.h"
#include "mbox.h"

void demo_uart() {
    // set up serial console
    pl011_uart_init();

    // say hello
    pl011_uart_puts("Hello World!");//ello World!\n");

    // echo everything back
    while(1) {
        pl011_uart_send(pl011_uart_getc());
    }
}

void main() {
    // Set up a serial console
    pl011_uart_init();

    // Get the board's unique serial number with a mailbox call
    mbox[0] = 8*4;                      // Length of the message
    mbox[1] = MBOX_REQUEST;             // Specify we are sending the message
    mbox[2] = MBOX_TAG_GETSERIAL;      // List of tags
    mbox[3] = 8;                        // Buffer size
    mbox[4] = 8;                        // TODO: ???
    mbox[5] = 0;                        // Clear ouput buffer
    mbox[6] = 0;                        // TODO: ???

    mbox[7] = MBOX_TAG_LAST;            // Specify end of tag list

    /* Send the message to the GPU and receive answer */
    if (mbox_call(MBOX_CH_PROP)) {
        pl011_uart_puts("My serial number is: ");
        pl011_uart_hex(mbox[6]);
        pl011_uart_hex(mbox[5]);
        pl011_uart_puts("\n");
    } else {
        pl011_uart_puts("Unable to query serial!\n");
    }

    /* Echo everything back like a terminal */
    while(1) {
        pl011_uart_send(pl011_uart_getc());
    }
}
