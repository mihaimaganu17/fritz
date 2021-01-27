/* Baud rate: how fast data is sent over a serial line(bits/sec)
 * https://learn.sparkfun.com/tutorials/serial-communication/all
 * Baud rate determines how long the transmitter holds a serial line high/low 
 * or at what perios the receiving device samples its line
 */
#include "gpio.h"

/* Auxiliary mini UART registers */
#define AUX_ENABLES     ((volatile unsigned int*)(MMIO_BASE+0x00215004))
#define AUX_MU_IO       ((volatile unsigned int*)(MMIO_BASE+0x00215040))
#define AUX_MU_IER      ((volatile unsigned int*)(MMIO_BASE+0x00215044))
#define AUX_MU_IIR      ((volatile unsigned int*)(MMIO_BASE+0x00215048))
#define AUX_MU_LCR      ((volatile unsigned int*)(MMIO_BASE+0x0021504C))
#define AUX_MU_MCR      ((volatile unsigned int*)(MMIO_BASE+0x00215050))
#define AUX_MU_LSR      ((volatile unsigned int*)(MMIO_BASE+0x00215054))
#define AUX_MU_MSR      ((volatile unsigned int*)(MMIO_BASE+0x00215058))
#define AUX_MU_SCRATCH  ((volatile unsigned int*)(MMIO_BASE+0x0021505C))
#define AUX_MU_CNTL     ((volatile unsigned int*)(MMIO_BASE+0x00215060))
#define AUX_MU_STAT     ((volatile unsigned int*)(MMIO_BASE+0x00215064))
#define AUX_MU_BAUD     ((volatile unsigned int*)(MMIO_BASE+0x00215068))

void uart_init() {

    register unsigned int r;
    /* initialize UART 
     * TODO: Refactor this values as globals */
    *AUX_ENABLES |=1;        // enable UART1, AUX mini uart
    *AUX_MU_IER = 0;        // disable receiving interrupts
    *AUX_MU_IIR = 0xc6;     // disable interrupts ??? TODO: undestand this value
    *AUX_MU_LCR = 3;        // UART works in 8-bit mode
    *AUX_MU_MCR = 0;        // all bits reserved apparently
    *AUX_MU_CNTL = 0;       // Lots of stuff
    *AUX_MU_BAUD = 270;     // 115200 baud (baud rate counter)

    /* map UART1 to GPIO pins 
     * Each of the 54 GPIO pins has at leadt 2 alternative functions
     * The FSEL n field determines the functionality of the nth GPIO pin
     * All unused alternative function lines are tied to ground and output 0 if selected */
    r = *GPFSEL1;
    r &= ~((7 << 12) | (7 << 15)); // pin14(bits 14-12) and pin15(bits 17-15)
    r |= (2 << 12) | (2 << 15);

    *GPFSEL1 = r;
    *GPPUD = 0;             // off - disable pull-up/down state
    /* Wait for 150 clock cycles */
    r = 150;
    while (r--) {
        asm volatile ("nop");
    }
    /* Specify which GPIO pads we want to modify */
    *GPPUDCLK0 = (1 << 14) | (1 << 15);
    /* Wait another 150 clock cycles */
    r = 150;
    while (r--) {
        asm volatile ("nop");
    }
    /* Write to GPPUDCLK to remove the clock */
    *GPPUDCLK0 = 0;
    /* Enables miniUART transmitter and receiver */
    *AUX_MU_CNTL = 3;
}

/*
 * Send a character
 */
void uart_send(unsigned int c) {
    /* Wait until transmit FIFO can accept at least one byte */
    do {
        asm volatile("nop");
    } while (!(*AUX_MU_LSR & (1 << 5)));
    /* Write byte */
    *AUX_MU_IO = c;
}

/*
 * Receive a character
 */
char uart_getc() {
    char r;
    /* Wait until something is in the buffer */
    do {
        asm volatile("nop");
    } while (!(*AUX_MU_LSR & 3));
    /* Read char and return */
    r = (char)(*AUX_MU_IO);
    /* Convert carriage return to newline */
    return r=='\r'?'\n':r;
}

/*
 * Display a string
 */
void uart_puts(char *s) {
    while(*s) {
        if(*s=='\n')
            uart_send('\r');
        uart_send(*s++);
    }
}

/*
 * Disaply a binary value in hex
 */
void uart_hex(unsigned int d) {
    unsigned int n;
    int c;
    for (c=28; c >=0; c-=4) {
        // get highest tetrad
        n = (d >> c) & 0xF;
        // 0-9 => '0'-'9', 10-15 => 'A' - 'F'
        n += n > 9 ? 0x37: 0x30;
        uart_send(n);
    }
}
