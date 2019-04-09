/* uart.c - UART initialization & communication */
/* Reference material:
 * http://www.raspberrypi.org/wp-content/uploads/2012/02/BCM2835-ARM-Peripherals.pdf
 * Chapter 13: UART
 */
 
#include <stdint.h>
#include <mmio.h>
#include <uart.h>
 
enum 
{
    // The GPIO registers base address.
    GPIO_BASE = 0x20200000,
 
    // The offsets for reach register.
 
    // Controls actuation of pull up/down to ALL GPIO pins.
    GPPUD = (GPIO_BASE + 0x94),
 
    // Controls actuation of pull up/down for specific GPIO pin.
    GPPUDCLK0 = (GPIO_BASE + 0x98),
 
    // The base address for UART.
    UART0_BASE = 0x20201000,
 
    // The offsets for reach register for the UART.
    UART0_DR     = (UART0_BASE + 0x00),
    UART0_RSRECR = (UART0_BASE + 0x04),
    UART0_FR     = (UART0_BASE + 0x18),
    UART0_ILPR   = (UART0_BASE + 0x20),
    UART0_IBRD   = (UART0_BASE + 0x24),
    UART0_FBRD   = (UART0_BASE + 0x28),
    UART0_LCRH   = (UART0_BASE + 0x2C),
    UART0_CR     = (UART0_BASE + 0x30),
    UART0_IFLS   = (UART0_BASE + 0x34),
    UART0_IMSC   = (UART0_BASE + 0x38),
    UART0_RIS    = (UART0_BASE + 0x3C),
    UART0_MIS    = (UART0_BASE + 0x40),
    UART0_ICR    = (UART0_BASE + 0x44),
    UART0_DMACR  = (UART0_BASE + 0x48),
    UART0_ITCR   = (UART0_BASE + 0x80),
    UART0_ITIP   = (UART0_BASE + 0x84),
    UART0_ITOP   = (UART0_BASE + 0x88),
    UART0_TDR    = (UART0_BASE + 0x8C),
};
 
/*
 * delay function
 * int32_t delay: number of cycles to delay
 *
 * This just loops <delay> times in a way that the compiler
 * wont optimize away.
 */
static void delay(int32_t count) 
{
   __asm__ volatile("__delay%=: subs %[count], %[count], #1; bne __delay%=\n"
	     : : [count]"r"(count) : "cc");
}
 
/*
 * Initialize UART0.
 */
void uart_init() 
{
    // Disable UART0.
    mmio_write(UART0_CR, 0x00000000);
    // Setup the GPIO pin 14 && 15.
 
    // Disable pull up/down for all GPIO pins & delay for 150 cycles.
    mmio_write(GPPUD, 0x00000000);
    delay(150);
 
    // Disable pull up/down for pin 14,15 & delay for 150 cycles.
    mmio_write(GPPUDCLK0, (1 << 14) | (1 << 15));
    delay(150);
 
    // Write 0 to GPPUDCLK0 to make it take effect.
    mmio_write(GPPUDCLK0, 0x00000000);
 
    // Clear pending interrupts.
    mmio_write(UART0_ICR, 0x7FF);
 
    // Set integer & fractional part of baud rate.
    // Divider = UART_CLOCK/(16 * Baud)
    // Fraction part register = (Fractional part * 64) + 0.5
    // UART_CLOCK = 3000000; Baud = 115200.
 
    // Divider = 3000000/(16 * 115200) = 1.627 = ~1.
    // Fractional part register = (.627 * 64) + 0.5 = 40.6 = ~40.
    mmio_write(UART0_IBRD, 1);
    mmio_write(UART0_FBRD, 40);
 
    // Disable FIFO. Make 8 bit data transmission (1 stop bit, no parity). Set bit 4 to 1 to enable FIFO
    mmio_write(UART0_LCRH, (1 << 5) | (1 << 6));
 
    // Set FIFO interrupt levels, Page 187
    //mmio_write(UART0_IFLS, (0<<0) | (0<<1) | (0<<2) | (0<<3) | (0<<4) | (0<<5));
 
    // Engineer the Interrupt for UART0 Receive				//Enable interrupt for UART0 TX
    mmio_write(UART0_IMSC, (1 << 4));
 
    // Enable UART0, receive & transfer part of UART.
    mmio_write(UART0_CR, (1 << 0) | (1 << 8) | (1 << 9));
}
 
/*
 * Transmit a byte via UART0.
 * uint8_t Byte: byte to send.
 */
void uart_putc(uint8_t byte) 
{
    // test for UART to become ready to transmit
    while (1) {
        if (!(mmio_read(UART0_FR) & (1 << 5))) {			//Wait until hardware TX buffer isn't full to continue
	//		if (!(mmio_read(UART0_FR) & (1 << 3))) {
				break;
			}
	//	}
    }
	
    int interrupt = 0; 
    if ((mmio_read(UART0_IMSC) & (1<<5)) == (1<<5)) {			//Check if tx interrupt is on in interrupt mask 
		interrupt = 1;
		uart_tx_off(); 						//Turn off tx interrupt while writing to buffer
	}
    mmio_write(UART0_DR, byte);
    if (interrupt == 1) { 
		uart_tx_on();						//Reenable tx interrupt if disabled
	}
}

uint8_t uart_readc(void)
{
	// test for UART to become ready to read, can also test the interrupt receive flag.
	while (1) 
	{
        if (!(mmio_read(UART0_FR) & (1 << 4))) 
		{
			break;
		}
    }
	return mmio_read(UART0_DR);
}
 
/*
 * print a string to the UART one character at a time
 * const char *str: 0-terminated string
 */
void uart_puts(const char *str) 
{
    while (*str) 
	{
        uart_putc(*str++);
    }
}

/*
 * print a string to the UART one character at a time
 * char *str: 0-terminated string, of potential length n
 */
void uart_putString(char *s, int n) {
	for (int i=0;i<n;i++) {
		if (s[i]=='\0') {break;}
		uart_putc(s[i]);
	}
}

uint8_t uart_itrpt_status(void) {
	
	if (mmio_read(UART0_MIS) & ((1<<5) | (1<<4)) == 0)
	{
		return 0; 							//NO status bits set
	}
	else if (mmio_read(UART0_MIS) & ((1<<5) | (1<<4)) == (1<<4))
	{
		return 1;							//Rx is set, Tx is not
	}
	else if (mmio_read(UART0_MIS) & ((1<<5) | (1<<4)) == (1<<5))
	{
		return 2;							//Tx is set, Rx is not
	}
	else if (mmio_read(UART0_MIS) & ((1<<5) | (1<<4)) == ((1<<5) | (1<<4)))
	{
		return 3;							//Both Tx and Rx are set
	}

	return 4;								//Something went wrong.

}

/*
 * Enable tx and rx interrupt by writing to interrupt mask, disabling all other uart interrupts
 */
void uart_tx_on(void) {
	mmio_write(UART0_IMSC, (1<<4) | (1<<5));	
}

/*
 * Enable rx interrupt by writing to interrupt mask, disabling all other uart interrupts
 */
void uart_tx_off(void) {
	mmio_write(UART0_IMSC, (1<<4));
}

/*
 * Checks uart buffers, returns 1 if full, 0 if empty, and 3 if neither
 * accepts 'r' for rx buffer and 't' for tx buffer
*/
uint8_t uart_buffchk(char c) {
	if (c == 'r') {
		if (mmio_read(UART0_FR) & (1<<4)) {
			return 0;
		} else if (mmio_read(UART0_FR) & (1<<6)) {
			return 1;
		} else {
			return 3;
		}
	} else if (c == 't') {
		if (mmio_read(UART0_FR) & (1<<7)) {
			return 0;
		} else if (mmio_read(UART0_FR) & (1<<5)) {
			return 1;
		} else {
			return 3;
		}
	}
	return 0;
}
