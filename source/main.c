// main.c - main for the CECS 525 Raspberry PI kernel
// by Eugene Rockey Copyright 2015 All Rights Reserved
// debug everything that needs debugging
// Add, remove, modify, preserve in order to fulfill project requirements.

#include <stdint.h>
#include "uart.h"
#include "mmio.h"
#include "bcm2835.h"
#include "can.h"
#include "softfloat.h"
#include "math.h"
#include "f2d.h"

#define SECS 0x00
#define MINS 0x01
#define HRS	 0x02
#define DOM	 0x04
#define MONTH 0x05
#define YEAR 0x06
#define ASECS 0x07
#define TEMPMSB 0x11
#define TEMPLSB 0x12
#define CR 0x0E
#define GPUREAD	0x2000B880
#define GPUPOLL	0x2000B890
#define GPUSENDER	0x2000B894
#define GPUSTATUS	0x2000B898
#define GPUCONFIG	0x2000B89C
#define GPUWRITE	0x2000B8A0



const char MS1[] = "\r\n\nCECS-525 RPI Tiny OS";
const char MS2[] = "\r\nby Eugene Rockey Copyright 2013 All Rights Reserved";
const char MS3[] = "\r\nReady: ";
const char MS4[] = "\r\nInvalid Command Try Again...";
const char MS5[] = "\r\n(A)DC,(C)ancom,(H)elp,a(L)arm,(R)eset,(S)FT,(T)ime,(V)FP11,(B)Print Long String,(G)Calculator";
const char GPUDATAERROR[] = "\r\nSystem Error: Invalid GPU Data";
const char LOGONNAME[] = "eugene    ";
const char PASSWORD[] = "cecs525   ";
//const char longstring[] = "This is a long string blaasuiguibjnkfgiue fiuwnkwdfuhjnu8iwref jifhgu8v hwefh9 8rjgioujfiogj ei ioe rgiorh gven fgklwrh fgi wnfiohfgkwnklfhj walefj iog awljenfwlfgja lwef'oiwue fjkasndfklwu fkjlaw n;fhkoaw eilf\r\n\0";
const char longstring[] = "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa\n\0";

extern int addition(int add1, int add2);
extern int subtraction(int sub1, int sub2);
extern int multiplication(int mul1, int mul2);
extern int division(int div1, int div2);
extern int remaind(int rem1, int rem2);

extern float32 vfp11_add(float32 op1, float32 op2);
extern float32 vfp11_sub(float32 op1, float32 op2);
extern float32 vfp11_mul(float32 op1, float32 op2);
extern float32 vfp11_div(float32 op1, float32 op2);
extern float32 vfp11_sqrt(float32 operand1);

//PWM Data for Alarm Tone
uint32_t N[200] = {0,1,2,3,4,5,6,7,8,9,10,11,12,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,
				36,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,60,61,62,63,64,65,66,67,68,69,
				70,71,72,73,74,75,76,77,78,79,80,81,82,83,84,84,85,86,87,88,89,90,91,92,93,94,95,96,95,94,93,92,91,90,
				89,88,87,86,85,84,84,83,82,81,80,79,78,77,76,75,74,73,72,71,70,69,68,67,66,65,64,63,62,61,60,60,59,58,57,
				56,55,54,53,52,51,50,49,48,47,46,45,44,43,42,41,40,39,38,37,36,36,35,34,33,32,31,30,29,28,27,26,25,24,23,
				   22,21,20,19,18,17,16,15,14,13,12,12,11,10,9,8,7,6,5,4,3,2,1};
char logname[10];
char pass[10];
char* buffer[1];
char alarm[1];
uint8_t ones;
uint8_t tens;
char* tbuf;
char* rbuf;
void kernel_main();             //prototypes
void enable_arm_irq();
void disable_arm_irq();
void enable_arm_fiq();
void disable_arm_fiq();
void reboot();
void enable_irq_57();
void disable_irq_57();
void testdelay();

/*************************** Modified Section **********************************/
void buff_print(void);							//Transmits entire rx buffer one character at a time
char buff_readc(void);							//Returns and removes oldest character from rx buffer
void buff_readline(char *s, int n);					//Reads n characters from rx buffer into *s or until '\n' 
void tx_string(void);							//Tranmits long string from memory							
int toString(int num, char* numArray);					//Converts signed int num into ascii characters in *numArray
int log_10(int num);							//Returns the number of digits in an integer
int stringToint(char *string);						//Returns signed int as converted from ascii string
int calc(void);								//Enters calculator submenu
int gettime_s(char *s);
int getdate_s(char *s);
void time_reset(void);
int temp_update(char *temp_string);

//Serial buffers
#define rxbuffsize 256
char rxbuff[rxbuffsize];
volatile unsigned int rxbuff_b;
volatile unsigned int rxbuff_e;

#define txbuffsize 256
char txbuff[txbuffsize];
volatile unsigned int txbuff_b;
volatile unsigned int txbuff_e;

//Date Variables
uint8_t date_century=0;						//0-256 
uint8_t date_year;  						//0-99
uint8_t date_month;							//1-12
uint8_t date_day;							//1-31

//Time Variables
uint8_t time_ampm;							//0-1
uint8_t time_hour;							//0-12
uint8_t time_minute;							//0-59
uint8_t time_second;							//0-59
	
//Temperature Variables
uint8_t temp_units;						//0 for Celsius, 1 for Fahrenheit
/***************************************************************************/

extern int invar;               //assembly variables
extern int outvar;

//Pointers to some of the BCM2835 peripheral register bases
volatile uint32_t* bcm2835_gpio = (uint32_t*)BCM2835_GPIO_BASE;
volatile uint32_t* bcm2835_clk = (uint32_t*)BCM2835_CLOCK_BASE;
volatile uint32_t* bcm2835_pads = (uint32_t*)BCM2835_GPIO_PADS;		//for later updates to program
volatile uint32_t* bcm2835_spi0 = (uint32_t*)BCM2835_SPI0_BASE;
volatile uint32_t* bcm2835_bsc0 = (uint32_t*)BCM2835_BSC0_BASE;		//for later updates to program
volatile uint32_t* bcm2835_bsc1 = (uint32_t*)BCM2835_BSC1_BASE;
volatile uint32_t* bcm2835_st = (uint32_t*)BCM2835_ST_BASE;

void testdelay(void)
{
	int count = 0xFFFFF;
	while (count > 0) {count--;}
}

void enable_irq_57(void) 
{
	mmio_write(0x2000B214, 0x02000000);				//BCM2835-Page 175, 112
}
void disable_irq_57(void)
{
	mmio_write(0x2000B220, 0x02000000);
}
void enable_irq(uint8_t x) {
	uint32_t write = 1;
	if (x < 32) {
		write = write << x;
		mmio_write(0x2000B210, write);
	} else {
		x = x - 32;
		write = write << x;
		mmio_write(0x2000B214, write);
	}
}
void disable_irq(uint8_t x) {
	uint32_t write = 1;
	if (x < 32) {
		write = write << x;
		mmio_write(0x2000B21C, write);
	} else {
		x = x - 32;
		write = write << x;
		mmio_write(0x2000B220, write);
	}
}

uint8_t ValidateGPUData(int data)
{
	if (data && 0b1111 != 0)
	{
		uart_puts (GPUDATAERROR);
		return 0;
	}
	return 1;	
}

void GPUInit(void)
{
	int data;
	if (ValidateGPUData(data) == 1)
	{
		// Under Construction
	}
	
}

void banner(void)
{
	uart_puts(MS1);
	uart_puts(MS2);
}

uint8_t BCDtoUint8(uint8_t BCD)
{
	return (BCD & 0x0F) + ((BCD >> 4) * 10);
}

void DATE(void)
{
	char ones;
	char tens;
	char c2 = '\0';
	uart_puts("\r\nEnter DATE (S)et or (D)isplay\r\n");
	uint8_t c = '\0';
	while (c == '\0') 
	{
		c = buff_readc();
	}
	switch (c) {
		case 'S' | 's':						//needs to be edited to fit DATE, this was copied from time
			bcm2835_i2c_begin();
			bcm2835_i2c_setClockDivider(BCM2835_I2C_CLOCK_DIVIDER_2500);
			bcm2835_i2c_setSlaveAddress(0x68);
			uart_puts("\r\nSet Date:");
			uart_puts("\r\nType Year (Four Digits 0000-9999): ");
			
			c2 = buff_readc();
			uart_putc(c2);
			date_century = (c2-48);			//input date_millenium
			date_century = date_century*10;
			c2 = buff_readc();
			uart_putc(c2);	
			date_century += (c2-48);		//input date_century
			c2 = buff_readc();
			uart_putc(c2);
			tens = (c2-48);			//holding the tens spot for date_year
			c2 = buff_readc();
			uart_putc(c2);
			ones = (c2-48);			//input ones spot for date_year
			date_year = ones + (tens*10); 	//combining date_hold and date_year to make date_year 0-99
			
			if (ones > 9 || tens > 9) 
			{
				*buffer[0] = 0x00;
				uart_puts("\r\nInvalid Year Value!");
				break;
			}
			else
			{
				*buffer[0] = (tens << 4) | ones;
			}			
			bcm2835_i2c_write(YEAR,*buffer);
			
			uart_puts("\r\nType Month (two digits 00-12): ");
			c2 = buff_readc();
			uart_putc(c2);
			tens = ((c2-48)<<4) | 0x0F;			//holding the tens spot for date_month
			c2 = buff_readc();
			uart_putc(c2);
			ones = (c2-48) | 0xF0;			//input ones spot for date_month
			
			if (BCDtoUint8(tens & ones) < 0 || BCDtoUint8(tens & ones) > 12) 
			{
				*buffer[0] = 0x00;
				uart_puts("\r\nInvalid Month Value!");
				break;
			}
			else
			{
				*buffer[0] = tens & ones;
			}			
			bcm2835_i2c_write(MONTH,*buffer);
						
			uart_puts("\r\nType Day (two digits 00-31): ");
			c2 = buff_readc();
			uart_putc(c2);
			tens = ((c2-48) << 4) | 0x0F;
			c2 = buff_readc();
			uart_putc(c2);
			ones = (c2-48) | 0xF0;
			if (BCDtoUint8(tens & ones) < 0 || BCDtoUint8(tens & ones) > 59) 
			{
				*buffer[0] = 0x00;
				uart_puts("\r\nInvalid Day Value!");
				break;
			}
			else
			{
				*buffer[0] = tens & ones;
			}			
			bcm2835_i2c_write(DOM,*buffer);			
			uart_puts("\r\nDate is now set.");
			bcm2835_i2c_end();
			break;

			
		case 'D' | 'd':
			bcm2835_i2c_begin();
			bcm2835_i2c_setClockDivider(BCM2835_I2C_CLOCK_DIVIDER_2500);
			bcm2835_i2c_setSlaveAddress(0x68);
			bcm2835_i2c_read(MONTH,*buffer);
			if (*buffer[0] & 0x80) {
				date_century++;
				*buffer[0] = *buffer[0] & 0x7F;
				bcm2835_i2c_write(MONTH,*buffer);
			}
			ones = *buffer[0] & 0x0F;
			tens = (*buffer[0] >> 4);
			uart_putc(tens+48);
			uart_putc(ones+48);
			uart_putc('/');
			bcm2835_i2c_read(DOM,*buffer);
			ones = *buffer[0] & 0x0F;
			tens = (*buffer[0] >> 4);
			uart_putc(tens+48);
			uart_putc(ones+48);
			uart_putc('/');
			tens = date_century / 10;
			ones = date_century - (tens * 10);
			uart_putc(tens+48);
			uart_putc(ones+48);
			bcm2835_i2c_read(YEAR,*buffer);
			ones = *buffer[0] & 0x0F;
			tens = (*buffer[0] >> 4);
			uart_putc(tens+48);
			uart_putc(ones+48);	
			bcm2835_i2c_end();
			break;
		default:
			uart_puts(MS4);
			DATE();
			break;
	}
}

int temp_update(char *temp_string) {
	
	int temp_int;					//For use within tostring, which takes an int as an input
	uint8_t temp_frac;				//Used for fractional input
	
	bcm2835_i2c_begin();
	bcm2835_i2c_setClockDivider(BCM2835_I2C_CLOCK_DIVIDER_2500);
	bcm2835_i2c_setSlaveAddress(0x68);
	bcm2835_i2c_read(TEMPMSB, *buffer);
	temp_int = *buffer[0];
	
	if (temp_int & 0x80) {				//converts a signed 8-bit integer into a signed 32-bit integer
		temp_int = (temp_int | 0x80000000);
		temp_int = (temp_int & 0xFFFFFF7F);
	}
	
	int n = toString(temp_int, temp_string);
	
	bcm2835_i2c_read(TEMPLSB, *buffer);
	temp_frac = ((*buffer[0] & 0xC0) >> 6);		//00 for 0, 01 for .25, 10 for .5, 11 for .75
	
	if (temp_frac != 0) {
		if (temp_frac == 1) {
			temp_string[n++] = '.';
			temp_string[n++] = '2';
			temp_string[n++] = '5';
		}
		else if (temp_frac == 2) {
			temp_string[n++] = '.';
			temp_string[n++] = '5';
		}
		else if (temp_frac == 3) {
			temp_string[n++] = '.';
			temp_string[n++] = '7';
			temp_string[n++] = '5';
		}
		temp_string[n] = '\0';
	}
	
	return n;
}
	
int getdate_s(char *s) {
	char ones,tens;
	int i = 0;
	
	bcm2835_i2c_begin();
	bcm2835_i2c_setClockDivider(BCM2835_I2C_CLOCK_DIVIDER_2500);
	bcm2835_i2c_setSlaveAddress(0x68);
	bcm2835_i2c_read(MONTH,*buffer);
	if (*buffer[0] & 0x80) {
		date_century++;
		*buffer[0] = *buffer[0] & 0x7F;
		bcm2835_i2c_write(MONTH,*buffer);
	}
	ones = *buffer[0] & 0x0F;
	tens = (*buffer[0] >> 4);
	s[i++] = tens + 48;	//uart_putc(tens+48);
	s[i++] = ones + 48;	//uart_putc(ones+48);
		
	bcm2835_i2c_read(DOM,*buffer);
	ones = *buffer[0] & 0x0F;
	tens = (*buffer[0] >> 4);
	s[i++] = '/';	//uart_putc('/');
	s[i++] = tens + 48;	//uart_putc(tens+48);
	s[i++] = ones + 48;	//uart_putc(ones+48);
	
	tens = date_century / 10;
	ones = date_century - (tens * 10);
	s[i++] = '/';	//uart_putc('/');
	s[i++] = tens + 48;	//uart_putc(tens+48);
	s[i++] = ones + 48;	//uart_putc(ones+48);
	
	bcm2835_i2c_read(YEAR,*buffer);
	ones = *buffer[0] & 0x0F;
	tens = (*buffer[0] >> 4);
	bcm2835_i2c_end();
	
	s[i++] = tens + 48;	//uart_putc(tens+48);
	s[i++] = ones + 48;	//uart_putc(ones+48);
	s[i] = '\0';
	
	return i;
}

int gettime_s(char *s) {
	char ones,tens;
	int8_t ampm = -1;
	int i = 0;
	
	bcm2835_i2c_begin();
	bcm2835_i2c_setClockDivider(BCM2835_I2C_CLOCK_DIVIDER_2500);
	bcm2835_i2c_setSlaveAddress(0x68);
	bcm2835_i2c_read(HRS,*buffer);
	ones = *buffer[0] & 0x0F;
	if (*buffer[0] & 0b01000000) {
		tens = (*buffer[0] >> 4) & 0b1;
		ampm = (*buffer[0] >> 5) & 0b1;
	} else {
		tens = (*buffer[0] >> 4) & 0b11;
	}
	s[i++] = tens + 48;	//uart_putc(tens+48);
	s[i++] = ones + 48;	//uart_putc(ones+48);
	
	bcm2835_i2c_read(MINS,*buffer);
	ones = *buffer[0] & 0x0F;
	tens = (*buffer[0] >> 4);
	s[i++] = ':';	//uart_putc(':');
	s[i++] = tens + 48;	//uart_putc(tens+48);
	s[i++] = ones + 48;	//uart_putc(ones+48);
	
	bcm2835_i2c_read(SECS,*buffer);
	bcm2835_i2c_end(); 
	ones = *buffer[0] & 0x0F;
	tens = (*buffer[0] >> 4);
	s[i++] = ':';	//uart_putc(':');
	s[i++] = tens + 48;	//uart_putc(tens+48);
	s[i++] = ones + 48;	//uart_putc(ones+48);	  
	
	if (ampm != -1) {
		s[i++] = ' ';	//uart_putc(' ');
		if (ampm == 0) {
			s[i++] = 'A';	//uart_puts("AM");
		} else {
			s[i++] = 'P';	//uart_puts("PM");	
		}
		s[i++] = 'M';
	}
	s[i] = '\0';
	
	return i;
}

void TIME(void)
{
	char ones = -1;
	char tens = -1;
	int8_t ampm = -1;
	char c2 = '\0';
	char hrs,mins,secs,yrs,mth,dom;
	
	disable_irq(49);
	bcm2835_i2c_begin();
	bcm2835_i2c_setClockDivider(BCM2835_I2C_CLOCK_DIVIDER_2500);
	bcm2835_i2c_setSlaveAddress(0x68);
	
	bcm2835_delayMicroseconds(500);
	bcm2835_i2c_read(CR,*buffer);
	bcm2835_delayMicroseconds(500);
	*buffer[0] = (*buffer[0] & 0b11111011);	
	bcm2835_i2c_write(CR,*buffer);
	bcm2835_delayMicroseconds(500);
	bcm2835_i2c_end();
	
	//Activate Interrupts
	bcm2835_gpio_fsel(RPI_GPIO_P1_16, BCM2835_GPIO_FSEL_INPT);
	bcm2835_gpio_fsel(RPI_GPIO_P1_18, BCM2835_GPIO_FSEL_INPT);
	bcm2835_gpio_set_pud(RPI_GPIO_P1_16, BCM2835_GPIO_PUD_UP);
	bcm2835_gpio_set_pud(RPI_GPIO_P1_18, BCM2835_GPIO_PUD_OFF);
	bcm2835_gpio_fen(RPI_GPIO_P1_16);
	bcm2835_gpio_ren(RPI_GPIO_P1_18);
	enable_irq(49);
	
	uart_puts("\x1B" "[2J" "\x1B" "[2;1H");	//Clear Screen, Go to row 2, col 1
	uart_puts("Set (T)IME, (D)ate, or (e)xit: ");
	uint8_t c = '\0';
	while (c == '\0') 
	{
		c = buff_readc();
	}
	switch (c) {
		case 'T': case 't':
			uart_puts("\r\nSet AM(0), PM(1), or (2)4Hour ");
			ampm = buff_readc() - 48;
			uart_puts("\r\nType Hours (two digits 00-");
			if (ampm == 2) {
				uart_puts("23): ");
			} else {
				uart_puts("12): ");
			}
			c2 = buff_readc();
			uart_putc(c2);
			tens = ((c2-48) << 4) | 0x0F;
			c2 = buff_readc();
			uart_putc(c2);
			ones = (c2-48) | 0xF0;
			if (BCDtoUint8(tens & ones) < 0 || BCDtoUint8(tens & ones) > 23 || (BCDtoUint8(tens & ones) > 12 && ampm != 2)) 
			{
				hrs = 0x00;
				uart_puts("\r\nInvalid Hours Value!");
				break;
			}
			else
			{
				if (ampm != 2) {
					ampm = 0b1000000 | (ampm << 5);
				} else {
					ampm = 0;
				}
				
				hrs = ampm | (tens & ones);
			}			
			
			uart_puts("\r\nType Minutes (two digits 00-59): ");
			c2 = buff_readc();
			uart_putc(c2);
			tens = ((c2-48) << 4) | 0x0F;
			c2 = buff_readc();
			uart_putc(c2);
			ones = (c2-48) | 0xF0;
			if (BCDtoUint8(tens & ones) < 0 || BCDtoUint8(tens & ones) > 59) 
			{
				mins = 0x00;
				uart_puts("\r\nInvalid Minutes Value!");
				break;
			}
			else
			{
				mins = tens & ones;
			}			
						
			uart_puts("\r\nType Seconds (two digits 00-59): ");
			c2 = buff_readc();
			uart_putc(c2);
			tens = ((c2-48) << 4) | 0x0F;
			c2 = buff_readc();
			uart_putc(c2);
			ones = (c2-48) | 0xF0;
			if (BCDtoUint8(tens & ones) < 0 || BCDtoUint8(tens & ones) > 59) 
			{
				secs = 0x00;
				uart_puts("\r\nInvalid Seconds Value!");
				break;
			}
			else
			{
				secs = tens & ones;
			}
			disable_irq(49);
			bcm2835_i2c_begin();
			bcm2835_i2c_setClockDivider(BCM2835_I2C_CLOCK_DIVIDER_2500);
			bcm2835_i2c_setSlaveAddress(0x68);
			*buffer[0] = hrs;
			bcm2835_i2c_write(HRS,*buffer);
			bcm2835_delayMicroseconds(500);
			*buffer[0] = mins;
			bcm2835_i2c_write(MINS,*buffer);
			bcm2835_delayMicroseconds(500);
			*buffer[0] = secs;
			bcm2835_i2c_write(SECS,*buffer);
			bcm2835_delayMicroseconds(500);			
			uart_puts("\r\nTime is now set.");
			bcm2835_i2c_end();
			enable_irq(49);
			break;
			
		case 'D': case 'd':
			uart_puts("\r\nSet Date:");
			uart_puts("\r\nType Year (Four Digits 0000-9999): ");
			
			c2 = buff_readc();
			uart_putc(c2);
			date_century = (c2-48);			//input date_millenium
			date_century = date_century*10;
			c2 = buff_readc();
			uart_putc(c2);	
			date_century += (c2-48);		//input date_century
			c2 = buff_readc();
			uart_putc(c2);
			tens = (c2-48);			//holding the tens spot for date_year
			c2 = buff_readc();
			uart_putc(c2);
			ones = (c2-48);			//input ones spot for date_year
			date_year = ones + (tens*10); 	//combining date_hold and date_year to make date_year 0-99
			
			if (ones > 9 || tens > 9) 
			{
				yrs = 0x00;
				uart_puts("\r\nInvalid Year Value!");
				break;
			}
			else
			{
				yrs = (tens << 4) | ones;
			}
			
			uart_puts("\r\nType Month (two digits 00-12): ");
			c2 = buff_readc();
			uart_putc(c2);
			tens = ((c2-48)<<4) | 0x0F;			//holding the tens spot for date_month
			c2 = buff_readc();
			uart_putc(c2);
			ones = (c2-48) | 0xF0;			//input ones spot for date_month
			
			if (BCDtoUint8(tens & ones) < 0 || BCDtoUint8(tens & ones) > 12) 
			{
				mth = 0x00;
				uart_puts("\r\nInvalid Month Value!");
				break;
			}
			else
			{
				mth = tens & ones;
			}
						
			uart_puts("\r\nType Day (two digits 00-31): ");
			c2 = buff_readc();
			uart_putc(c2);
			tens = ((c2-48) << 4) | 0x0F;
			c2 = buff_readc();
			uart_putc(c2);
			ones = (c2-48) | 0xF0;
			if (BCDtoUint8(tens & ones) < 0 || BCDtoUint8(tens & ones) > 59) 
			{
				dom = 0x00;
				uart_puts("\r\nInvalid Day Value!");
				break;
			}
			else
			{
				dom = tens & ones;
			}
			disable_irq(49);
			bcm2835_i2c_begin();
			bcm2835_i2c_setClockDivider(BCM2835_I2C_CLOCK_DIVIDER_2500);
			bcm2835_i2c_setSlaveAddress(0x68);
			*buffer[0] = yrs;
			bcm2835_i2c_write(YEAR,*buffer);
			bcm2835_delayMicroseconds(500);
			*buffer[0] = mth;
			bcm2835_i2c_write(MONTH,*buffer);
			bcm2835_delayMicroseconds(500);
			*buffer[0] = dom;
			bcm2835_i2c_write(DOM,*buffer);
			bcm2835_delayMicroseconds(500);	
			uart_puts("\r\nDate is now set.");
			bcm2835_i2c_end();
			enable_irq(49);
			break;	
		case 'E': case 'e':
			disable_irq(49);
			break;
		default:
			uart_puts(MS4);
			TIME();
			break;	
	}
}

void time_reset(void) {
	bcm2835_i2c_begin();
	bcm2835_i2c_setClockDivider(BCM2835_I2C_CLOCK_DIVIDER_2500);
	bcm2835_i2c_setSlaveAddress(0x68);

	*buffer[0] = 0x00;
	bcm2835_i2c_write(SECS,*buffer);
	bcm2835_delayMicroseconds(500);
	*buffer[0] = 0x00;
	bcm2835_i2c_write(MINS,*buffer);
	bcm2835_delayMicroseconds(500);
	*buffer[0] = 0b00010010;
	bcm2835_i2c_write(HRS,*buffer);
	bcm2835_delayMicroseconds(500);
	*buffer[0] = 0x01;			
	bcm2835_i2c_write(DOM,*buffer);
	bcm2835_delayMicroseconds(500);		
	*buffer[0] = 0x01;			
	bcm2835_i2c_write(MONTH,*buffer);
	bcm2835_delayMicroseconds(500);
	date_century = 20;
	*buffer[0] = 0x18;
	bcm2835_i2c_write(YEAR,*buffer);
	bcm2835_delayMicroseconds(500);

	bcm2835_i2c_end();
}

void ALARM(void)
{
	uart_puts("\r\nType ALARM (S)et or (D)isplay or (T)est\r\n");
	uint8_t c = '\0';
	while (c == '\0') 
	{
		c = buff_readc();
	}
	switch (c) {
		case 'S' | 's':
			uart_puts("\r\nSet Seconds Alarm");
			uart_puts("\r\nType Starting Alarm Seconds (two digits 05-59): ");
			tens = ((buff_readc()-48) << 4) | 0x0F;
			uart_putc((tens >> 4)+48);
			ones = (buff_readc()-48) | 0xF0;
			uart_putc((ones & 0x0F)+48);
			if (BCDtoUint8(tens & ones) < 5 || BCDtoUint8(tens & ones) > 59) 
			{
				alarm[0] = 0x05;
				uart_puts("\r\nInvalid Alarm Value, Value Reset to 5!");
				break;
			}
			else
			{
				alarm[0] = tens & ones;
				uart_puts("\r\nAlarm is now set.");
			}
			break;
		case 'D' | 'd':
			ones = alarm[0] & 0x0F;
			tens = alarm[0] >> 4;
			uart_putc(tens+48);
			uart_putc(ones+48);
			break;
		case 'T' | 't':
			if (BCDtoUint8(alarm[0]) < 5 || BCDtoUint8(alarm[0]) > 59)
			{
				uart_puts("\r\nAlarm Value Out of Range, Set to a Proper Value First!");
				break;
			}
			uart_puts("\r\nPlease wait, now testing Alarm...");

			//Engineer the code here to Drive the Pulse Width Modulated audio to the speaker jack.
          
			break;
		default:
			uart_puts(MS4);
			ALARM();
			break;	
	}
}

void CANCOM(void)
{
	//Initialize SPI Peripheral I/O
    bcm2835_spi_begin();
    bcm2835_spi_setBitOrder(BCM2835_SPI_BIT_ORDER_MSBFIRST);      // The default
    bcm2835_spi_setDataMode(BCM2835_SPI_MODE0);                   // The default
    bcm2835_spi_setClockDivider(BCM2835_SPI_CLOCK_DIVIDER_65536); // The default
    bcm2835_spi_chipSelect(BCM2835_SPI_CS0);                      // The default
    bcm2835_spi_setChipSelectPolarity(BCM2835_SPI_CS0, LOW);      // the default
    //Reset the CAN Controller
	bcm2835_gpio_fsel(RPI_GPIO_P1_15, BCM2835_GPIO_FSEL_OUTP);//GPIO22 drives MCP2515 *RESET Pin low.
	bcm2835_gpio_fsel(RPI_GPIO_P1_18, BCM2835_GPIO_FSEL_OUTP);//GPIO24 drives MCP2515 *CS Pin low
	bcm2835_gpio_write(RPI_GPIO_P1_15, LOW);
	bcm2835_delayMicroseconds(150);
	bcm2835_gpio_write(RPI_GPIO_P1_15, HIGH);
	
	
	uart_puts("\r\n(T)ransmit or (R)eceive\r\n"); //Choose to transmit or receive
	uint8_t c = '\0';
	while (c == '\0') 
	{
		c = buff_readc();
	}
	switch (c) {
		case 'T' | 't':
			bcm2835_gpio_write(RPI_GPIO_P1_18, LOW);
			CANtransmit(0x30);
			bcm2835_gpio_write(RPI_GPIO_P1_18, HIGH);
			break;
		case 'R' | 'r':
			bcm2835_gpio_write(RPI_GPIO_P1_18, LOW);
			uint8_t data = CANreceive();
			bcm2835_gpio_write(RPI_GPIO_P1_18, HIGH);
			break;
		default:
			uart_puts(MS4);
			CANCOM;
			break;
	}
	//Restore
    bcm2835_gpio_fsel(RPI_GPIO_P1_15, BCM2835_GPIO_FSEL_INPT);//GPIO22 restored to input.
	bcm2835_gpio_fsel(RPI_GPIO_P1_18, BCM2835_GPIO_FSEL_INPT);//GPIO24 restored to input.
	bcm2835_spi_end();
}

void ADC(void) //Demo Analog to Digital Converter circuit
{
    
    
    
    //Engineer the bare-metal code here to communicate with the MCP3008 ADC via the RPI SPI port
    //This function should display on Hyperterminal the changing analog input on MCP3008 Channel 0 as a value from ~ 0 to 1023.
    //The analog input could be from a potentiometer connected to 3.3V
    //Use only 3.3V with the MCP3008 ADC chip circuit.
    //See the ADC web link in helpful links doc.
    
    
    
    
}

void RES(void)
{
	reboot();
}

void HELP(void) //Command List
{
	uart_puts(MS5);
}

void SFT(void) //Soft Floating Point Demo, optionally complete this command and related code for experience with floating point operations performed in software.
{
	char a[20];
	char b[20];
	unsigned int c, d, result;
	uart_puts("\r\nInput First Decimal Number eg. -0.04, 23.45, 1.89, -5.0");
	//Get first input string here
	uart_puts("\r\nInput Second Decimal Number eg. -0.04, 23.45, 1.89, -5.0");
	//Get second input string here
	c = ASCII_to_float32(a);
	d = ASCII_to_float32(b);
	uart_puts("\r\nResults...");
	result = float32_add(c,d);
	//print add result here
	result = float32_sub(c,d);
	//print sub result here
}

void VFP11(void) //ARM Vector Floating Point Unit Demo, see softfloat.c for some useful and helpful example code.
{
    //Engineer the VFP11 math coprocessor application here.
    //Send a menu to hyperterminal as was done with the minimal computer
    //FADD, FSUB, FMUL, and FDIV, and any other functions you wish to implement in ARM assembly routines.
    char operator[1];
	const char calcmsg1[] = "\n\rSelect an Operator (+,-,*,/,(s)qrt,(v)olumeSphere,(e)xit): \0";
	const char calcmsg2[] = "\n\rEnter first Operand: \0";
	const char calcmsg3[] = "\n\rEnter second Operand: \0";
	const char calcmsg4[] = "\n\rYour answer is: \0";
	const char calcmsg5[] = " with a remainder of \0";
	const char calcmsg6[] = "\n\rOperator error. Please try again.\0";
	char opstring[30];
	float32 operand1;
	float32 operand2;
	float32 output = 0;
	char outputstring[30];
	
	while (1) {		
		//printf("Select an Operator (+,-,*,/,(s)phere,(e)xit): ");
		uart_puts(calcmsg1);
	
		//scanf("%c", operator[1]);
		operator[0] = buff_readc();
		//buff_readline(operator, 1);
	
		if (operator[0] != 'e') {
			//printf("\nEnter first Operand: ");
			uart_puts(calcmsg2);
			//scanf("%d", operand1);
			buff_readline(opstring, 30);
			if (opstring[0] == 'p') {operand1=output;}
			else {operand1 = ASCII_to_float32(opstring);}
			
			if (operator[0] != 'v' && operator[0] != 's') {
				//printf("\nEnter second Operand: ");
				uart_puts(calcmsg3);
				//scanf("%d", operand2);
				buff_readline(opstring, 30);
				if (opstring[0] == 'p') {operand2 = output;}
				else {operand2 = ASCII_to_float32(opstring);}
			}
		} else {
			return;
		}	
		
		output = 0;

		switch(operator[0]) {
			case '+':
				output = vfp11_add(operand1, operand2);
				break;
			case '-':
				output = vfp11_sub(operand1, operand2);
				break;
			case '*':
				output = vfp11_mul(operand1, operand2);				
				break;
			case '/':
				output = vfp11_div(operand1, operand2);
				break;
			case 's':
				output = vfp11_sqrt(operand1);
				break;
			case 'v':
				output = vfp11_mul(operand1, operand1);
				output = vfp11_mul(operand1, output);
				output = vfp11_mul(output, 0x40860a92);
				break;
			default:
				//printf("\nOperator error. Please try again.");
				uart_puts(calcmsg6);
				break;
		}
	
        //printf("Your answer is %d", output);
		uart_puts(calcmsg4);
//
        float_d fd = f2d(output);
        if (fd.s) {uart_putc('-');}
		
		if (fd.m == 0 && fd.e == 0) {uart_putc('0');}
		else if (fd.e == 0x7FFFFFFF) {uart_puts("Infinity\0");}
		else if (fd.e == 0x80000000) {uart_puts("NaN\0");}
		else {
			toString(fd.m,outputstring);
			uart_putString(outputstring, 30);
			uart_putc('E');
			toString(fd.e,outputstring);
			uart_putString(outputstring, 30);
		}
//
		uart_puts(" => hex:\0");
        toString_hex(output, outputstring);
		uart_putString(outputstring, 30);

	}
}

void command(void)
{
	uart_puts(MS5);
	uart_puts(MS3);
	uint8_t c = '\0';
	while (c == '\0') {
		c = buff_readc();
	}
	switch (c) {
		case 'C': case 'c':
			CANCOM();
			break;
		case 'D': case 'd':
			DATE();
			break;
		case 'T': case 't':
			TIME();
			break;
		case 'L': case 'l':
			ALARM();
			break;
		case 'A': case 'a':
			ADC();
			break;
		case 'R': case 'r':
			RES();
			break;
		case 'S': case 's':
			SFT();
			break;
		case 'H': case 'h':
			HELP();
			break;
		case 'V': case 'v':
			VFP11();
			break;
		case 'B': case 'b':
			tx_string();
			break;
		case 'G': case 'g':
			calc();
			break;
		case '7':
			break;
		default:
			uart_puts(MS4);
			HELP();
			break;
	}
}

int logon(void)
{
	int success = 0;
    
    
	//Engineer the code here to LOGON to the system as was done with the MC68000 minimal computer.
    
    
    return success;
}

void kernel_main() 
{
	rxbuff_b = 0;
	rxbuff_e = 0;
	txbuff_b = 0;
	txbuff_e = 0;
	char inpt[16];
	
	uart_init();
	enable_irq(57);	//enable_irq_57();
	enable_arm_irq();
//	if (logon() == 0) while (1) {}
	time_reset();
	banner();
//	HELP();
	while (1) {command();}
	
	while (1) 
	{
		uart_putc(' ');
		uart_putc('B');
		uart_putc(' ');
		testdelay();
					
		/*
		if (rxbuff_e != rxbuff_b) {					//If buffer isn't empty
			if (rxbuff_e < rxbuff_b) {				//If buffer has wrapped around (circular buffer)
				if ((255 - rxbuff_b) + rxbuff_e > 5) {		//If buffer has more than 5 elements
					buff_read();									
				}
			} else {
				if (rxbuff_e - rxbuff_b > 5) {			//If buffer has more than 5 elements
					buff_read();
				}
			}		
		}
		*/


	}
}

void irq_handler(void)
{
	
	if (bcm2835_gpio_eds(RPI_GPIO_P1_16)) {		//Reset Clock
		bcm2835_gpio_set_eds(RPI_GPIO_P1_16);
		disable_irq(49);
		time_reset();
		enable_irq(49);
	}
	if (bcm2835_gpio_eds(RPI_GPIO_P1_18)) {		//Time Update
		bcm2835_gpio_set_eds(RPI_GPIO_P1_18);
		disable_irq(49);
		char datetime[20];
		uart_puts("\x1B" "[s" "\x1B" "[H" "\x1B" "[2K");
		gettime_s(datetime);
		uart_putString(datetime,20);
		uart_putc(' ');
		getdate_s(datetime);
		uart_putString(datetime,20);
		uart_putc(' ');
		temp_update(datetime);
		uart_putString(datetime,20);
		uart_putc(0xC2);
		uart_putc(0xB0);
		uart_putc('C');
		uart_puts("\x1B" "[u");	//Return Cursor to saved location
		enable_irq(49);
	}
	if (txbuff_b != txbuff_e) { 
		if (uart_buffchk('t') == 0) {
			for (int i=0;i<8;i++) {
				uart_putc(txbuff[txbuff_b]);
				txbuff_b++;
				if (txbuff_b >= txbuffsize) {txbuff_b = 0;}
				if (txbuff_b == txbuff_e) {i=8;}		//Turn off tx interrupt, exit loop
			}
		}
	} else {
		uart_tx_off();
	}
	while (uart_buffchk('r') != 0) {
		rxbuff[rxbuff_e]  = uart_readc();
		//uart_putc(rxbuff[rxbuff_e]);					//Echo test
		rxbuff_e++;
		if (rxbuff_e >= rxbuffsize) {rxbuff_e = 0;}
	}
}

void tx_string(void) {
	unsigned int i = 0;
	while (longstring[i] != '\0') {
		txbuff[txbuff_e] = longstring[i];
		txbuff_e++;
		i++;
		if (txbuff_e >= txbuffsize) {txbuff_e = 0;}		
	}
	uart_tx_on();
/*	
	while (rxbuff_b == rxbuff_e) {						//neverending a's transmit mode
		txbuff[txbuff_e] = 'a';						
		txbuff_e++;
		if (txbuff_e >= txbuffsize) {txbuff_e = 0;}	
		uart_tx_on();
	}
*/
}

void buff_print(void) {
	while (rxbuff_b != rxbuff_e) {						//Echo entire rx buffer to tx, one character at a time
		uart_putc(rxbuff[rxbuff_b]);
		rxbuff_b++;
		if (rxbuff_b >= rxbuffsize) {rxbuff_b = 0;}
	}
	return;
}

void buff_readline(char *s, int n) {
	int i = 0;
	char c = buff_readc();						
	while (c != '\r' && c != '\n' && i < n) {				//Read from buffer until newline or out of space
		s[i] = c;
		uart_putc(s[i]);						//Echo recieved character
		i++;
		c=buff_readc();
	}
	while (i < n) {									//Fill remaining string with termination character
		s[i] = '\0';
		i++;
	}
}

char buff_readc(void) {
	while (rxbuff_b == rxbuff_e) {							//Wait until buffer is nonempty
				
	}
	char c = rxbuff[rxbuff_b];
	rxbuff_b++;
	if (rxbuff_b >= rxbuffsize) {rxbuff_b = 0;}

	return c;	
}

int toString(int num, char* numArray) {
	int neg = 0;
	if (num < 0) {
		num = num * (-1);
		neg = 1;
	}
	
	int n = log_10(num) - 1;
	if (neg == 1) {n++;}
	
	for (int i = n; i >= 0; --i, num /= 10) {
		numArray[i] = num % 10 + 48;
	}
	
	numArray[n+1]='\0';
	if (neg == 1) {numArray[0] = '-';}
	
	return n+1;
}

int log_10(int num) {
	int result;
	
	for (result = 0; num >= 1; result++) {
		num = num / 10;
	}
	return result;
}

int stringToInt(char* string) {
	int num = 0;
	
	for(int i = 0; i < 30; i++) { //15 can be changed based on max string length
		if (!((string[i] >= 48 && string[i] <= 57) || string[i] == 45)) {	//Break if not '-' or number
			break;
		}
		
		if (string[i] != 45) {
			num *= 10;
			num += (string[i] - 48);
		}
	}
	
	if (string[0] == 45) {
		num = 0 - num;
	}
	
	return num;
}

int calc() {
	
	char operator[1];
	const char calcmsg1[] = "\n\rSelect an Operator (+,-,*,/,(e)xit): \0";
	const char calcmsg2[] = "\n\rEnter first Operand: \0";
	const char calcmsg3[] = "\n\rEnter second Operand: \0";
	const char calcmsg4[] = "\n\rYour answer is: \0";
	const char calcmsg5[] = " with a remainder of \0";
	const char calcmsg6[] = "\n\rOperator error. Please try again.\0";
	int operand1;
	char opstring1[30];
	int operand2;
	char opstring2[30];
	int output = 0;
	char outputstring[30];
	int rem = 0;
	
	while (1) {		
		//printf("Select an Operator (+,-,*,/,(e)xit): ");
		uart_puts(calcmsg1);
	
		//scanf("%c", operator[1]);
		operator[0] = buff_readc();
		//buff_readline(operator, 1);
	
		if (operator[0] != 'e') {
			//printf("\nEnter first Operand: ");
			uart_puts(calcmsg2);
	
			//scanf("%d", operand1);
			buff_readline(opstring1, 30);
			operand1 = stringToInt(opstring1);
	
			//printf("\nEnter second Operand: ");
			uart_puts(calcmsg3);
	
			//scanf("%d", operand2);
			buff_readline(opstring2, 30);
			operand2 = stringToInt(opstring2);
		} else {
			return 0;
		}	
		
		output = 0;
		rem = 0;

		switch(operator[0]) {
			case '+':
				output = addition(operand1, operand2);
				break;
			case '-':
				output = subtraction(operand1, operand2);
				break;
			case '*':
				output = multiplication(operand1, operand2);				
				break;
			case '/':
				output = division(operand1, operand2);
				rem = remaind(operand1, operand2);
				break;	
			default:
				//printf("\nOperator error. Please try again.");
				uart_puts(calcmsg6);
				break;
		}
	

		toString(output, outputstring);
		//printf("Your answer is %d", output);
		uart_puts(calcmsg4);
		uart_putString(outputstring, 30);

		if (rem != 0) {	
			toString(rem, outputstring);
			//printf(" with remainder of %d", rem);
			uart_puts(calcmsg5);
			uart_putString(outputstring, 30);
		}
	}

	
	return 0;
}
