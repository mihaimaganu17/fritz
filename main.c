#include "uart.h"
#include "mbox.h"

void demo_uart() {
    // set up serial console
    uart_init();

    // say hello
    uart_puts("Hello World!");//ello World!\n");

    // echo everything back
    while(1) {
        uart_send(uart_getc());
    }
}

void main() {
    // Set up a serial console
    uart_init();

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
        uart_puts("My serial number is: ");
        uart_hex(mbox[6]);
        uart_hex(mbox[5]);
        uart_puts("\n");
    } else {
        uart_puts("Unable to query serial!\n");
    }

    /* Echo everything back like a terminal */
    while(1) {
        uart_send(uart_getc());
    }
}
