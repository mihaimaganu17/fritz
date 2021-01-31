#pragma once
/*
 * Header containing definitions for the UART communication
 * Created at 26/01/2021
 * Written by m3m0ry
 */

/* Utils */
void map_uart_to_gpio();

/* Mini UART functions */
void mini_uart_init();
void mini_uart_send(unsigned int c);
char mini_uart_getc();
void mini_uart_puts(char *s);
void mini_uart_hex(unsigned int d);

void pl011_uart_init();
void pl011_uart_send(unsigned int c);
char pl011_uart_getc();
void pl011_uart_puts(char *s);
void pl011_uart_hex(unsigned int d);
