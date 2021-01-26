/*
 * Header containing definitions for the UART communication
 * Created at 26/01/2021
 * Written by m3m0ry
 */

void uart_init();
void uart_send(unsigned int c);
char uart_getc();
void uart_puts(char *s);
