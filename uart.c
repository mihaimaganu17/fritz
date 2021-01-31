/* Baud rate: how fast data is sent over a serial line(bits/sec)
 * https://learn.sparkfun.com/tutorials/serial-communication/all
 * Baud rate determines how long the transmitter holds a serial line high/low 
 * or at what perios the receiving device samples its line
 */
#include "gpio.h"
#include "mbox.h"
#include "uart.h"

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

/* PL011 UART register */
#define UART0_DR        ((volatile unsigned int*)(MMIO_BASE+0x00201000))
#define UART0_FR        ((volatile unsigned int*)(MMIO_BASE+0x00201018))
/* Baud rate registers */
#define UART0_IBRD      ((volatile unsigned int*)(MMIO_BASE+0x00201024))
#define UART0_FBRD      ((volatile unsigned int*)(MMIO_BASE+0x00201028))

#define UART0_LCRH      ((volatile unsigned int*)(MMIO_BASE+0x0020102C))
#define UART0_CR        ((volatile unsigned int*)(MMIO_BASE+0x00201030))
#define UART0_IMSC      ((volatile unsigned int*)(MMIO_BASE+0x00201038))
#define UART0_ICR       ((volatile unsigned int*)(MMIO_BASE+0x00201044))

void mini_uart_init() {
    /* initialize UART 
     * TODO: Refactor this values as globals */
    *AUX_ENABLES |=1;        // enable UART1, AUX mini uart
    *AUX_MU_IER = 0;        // disable receiving interrupts
    *AUX_MU_IIR = 0xc6;     // disable interrupts ??? TODO: undestand this value
    *AUX_MU_LCR = 3;        // UART works in 8-bit mode
    *AUX_MU_MCR = 0;        // all bits reserved apparently
    *AUX_MU_CNTL = 0;       // Lots of stuff
    *AUX_MU_BAUD = 270;     // 115200 baud (baud rate counter)

    map_uart_to_gpio();

    /* Enables miniUART transmitter and receiver */
    *AUX_MU_CNTL = 3;
}

/*
 * Send a character
 */
void mini_uart_send(unsigned int c) {
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
char mini_uart_getc() {
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
void mini_uart_puts(char *s) {
    while(*s) {
        if(*s=='\n')
            mini_uart_send('\r');
        mini_uart_send(*s++);
    }
}

/*
 * Disaply a binary value in hex
 */
void mini_uart_hex(unsigned int d) {
    unsigned int n;
    int c;
    for (c=28; c >=0; c-=4) {
        // get highest tetrad
        n = (d >> c) & 0xF;
        // 0-9 => '0'-'9', 10-15 => 'A' - 'F'
        n += n > 9 ? 0x37: 0x30;
        mini_uart_send(n);
    }
}

void pl011_uart_init() {
    /* Disable UART first */
    *UART0_CR = 0;

    /* set up clock for consisten divisor values */
    mbox[0] = 9*4;
    mbox[1] = MBOX_REQUEST;
    mbox[2] = MBOX_TAG_CLKRATE;     // set clock rate
    mbox[3] = 12;                   // buffer size
    mbox[4] = 8;                    // value buffer
    mbox[5] = 2;                    // UART clock
    mbox[6] = 4000000;              // 4Mz
    mbox[7] = 0;                    // clear turbo
    mbox[8] = MBOX_TAG_LAST;
    mbox_call(MBOX_CH_PROP);

    map_uart_to_gpio();

    *UART0_ICR = 0x7FF;             // Basically set it up, clear interrupts
    /* baud_div = (FUART_CLK/(16 Baut rate))*/
    *UART0_IBRD = 2;                // Integer part of the Baud Rate Divisor
    *UART0_FBRD = 0xB;              // Fractional part of he baud rate divisor
    *UART0_LCRH = 0b11<<5;          // 8-bits wotd length 8n1
    *UART0_CR = 0x301;              // enable TX, RX, FIFO
}

void pl011_uart_send(unsigned int c) {
    /* Wait until we can send */
    do {
        asm volatile("nop");
    } while(*UART0_FR & 0x20);
    /* Write the character to the buffer */
    *UART0_DR = c;
}

char pl011_uart_getc() {
    char r;
    /* Wait until something is in the buffer */
    do {
        asm volatile("nop");
    } while(*UART0_FR & 0x10);
    /* Read it and return */
    r = (char)(*UART0_DR);
    return r == '\r' ? '\n':r;
}

void pl011_uart_puts(char *s) {
    while(*s) {
        if (*s == '\n')
            pl011_uart_send('\r');
        pl011_uart_send(*s++);
    }
}

void pl011_uart_hex(unsigned int d) {
    unsigned int n;
    int c;
    for (c=28; c >=0; c-=4) {
        // get highest tetrad
        n = (d >> c) & 0xF;
        // 0-9 => '0'-'9', 10-15 => 'A' - 'F'
        n += n > 9 ? 0x37: 0x30;
        pl011_uart_send(n);
    }
}

void map_uart_to_gpio() {
    /* map UART1 to GPIO pins 
     * Each of the 54 GPIO pins has at least 2 alternative functions
     * The FSEL n field determines the functionality of the nth GPIO pin
     * All unused alternative function lines are tied to ground and output 0 if selected */
    register unsigned int r;
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
}
