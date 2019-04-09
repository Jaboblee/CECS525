/* uart.h - UART initialization & communication */
// by ??
 
#ifndef UART_H
#define UART_H
 
#include <stdint.h>
 
/*
 * Initialize UART0.
 */
void uart_init();
 
/*
 * Transmit a byte via UART0.
 * uint8_t Byte: byte to send.
 */
void uart_putc(uint8_t byte);

// read one character
uint8_t uart_readc(void);
 
/*
 * print a string to the UART one character at a time
 * const char *str: 0-terminated string
 */
void uart_puts(const char *str);

/*
 * print a string to the UART one character at a time
 * char *str: 0-terminated string, of potential length n
 */
void uart_putString(char *s, int n);

/*
 * Turn on transmit interrupt
 */
void uart_tx_on(void);

/*
 * Turn off transmit interrupt
 */
void uart_tx_off(void);

/*
 * Output 0 if rx interrupt, 1 if tx interrupt, and 2 if both
 */
uint8_t uart_itrpt_status(void);

/*
 * returns 1 if full, 0 if empty, and 3 if neither
 * accepts 'r' for rx buffer and 't' for tx buffer
*/
uint8_t uart_buffchk(char c);

#endif // #ifndef UART_H
