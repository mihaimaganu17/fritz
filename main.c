#include "uart.h"
#include "mbox.h"
#include "rand.h"
#include "delays.h"
#include "power.h"
#include "lfb.h"

void wait_demo();

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


    wait_demo();

    /* Send the message to the GPU and receive answer */
    if (mbox_call(MBOX_CH_PROP)) {
        pl011_uart_puts("My serial number is: ");
        pl011_uart_hex(mbox[6]);
        pl011_uart_hex(mbox[5]);
        pl011_uart_puts("\n");
    } else {
        pl011_uart_puts("Unable to query serial!\n");
    }

    lfb_init();
    lfb_showpicture();

    char c;
    /* Echo everything back like a terminal */
    while(1) {
        pl011_uart_puts("1 - power off\n 2 -reset\n");
        c = pl011_uart_getc();
        pl011_uart_send(c);
        if (c == '1') power_off();
        if (c == '2') reset();
    }
}

void wait_demo() {
    pl011_uart_puts("Waiting 1000000 CPU cycles (ARM CPU)");
    wait_cycles(1000000);
    pl011_uart_puts("OK\n");

    pl011_uart_puts("Waiting 1000000 microsecs (ARM CPU)");
    wait_cycles(1000000);
    pl011_uart_puts("OK\n");

    pl011_uart_puts("Waiting 1000000 microsecs (ARM CPU)");
    if (get_system_timer() == 0) {
        pl011_uart_puts("Not available\n");
    } else {
        wait_msec_st(1000000);
        pl011_uart_puts("OK\n");
    }
}
