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

#define SECS 0x00
#define MINS 0x01
#define HRS	 0x02
#define DOM	 0x04
#define MONTH 0x05
#define YEAR 0x06
#define ASECS 0x07
#define CR 0x0D
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
const char GPUDATAERROR[] = "\r\nSystem Error: Invalid GPU Data";
const char LOGONNAME[] = "eugene    ";
const char PASSWORD[] = "cecs525   ";
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
extern int invar;               //assembly variables
extern int outvar;



//Pointers to some of the BCM2835 peripheral register bases
volatile uint32_t* bcm2835_gpio = (uint32_t*)BCM2835_GPIO_BASE;
volatile uint32_t* bcm2835_clk = (uint32_t*)BCM2835_CLOCK_BASE;
volatile uint32_t* bcm2835_pads = (uint32_t*)BCM2835_GPIO_PADS;//for later updates to program
volatile uint32_t* bcm2835_spi0 = (uint32_t*)BCM2835_SPI0_BASE;
volatile uint32_t* bcm2835_bsc0 = (uint32_t*)BCM2835_BSC0_BASE;//for later updates to program
volatile uint32_t* bcm2835_bsc1 = (uint32_t*)BCM2835_BSC1_BASE;
volatile uint32_t* bcm2835_st = (uint32_t*)BCM2835_ST_BASE;

void testdelay(void)
{
	int count = 0xFFFFF;
	while (count > 0) {count = count - 1;}
}

void enable_irq_57(void)
{
	mmio_write(0x2000B214, 0x02000000);
}

void disable_irq_57(void)
{
	mmio_write(0x2000B220, 0x02000000);
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
	uart_puts("\r\nEnter DATE (S)et or (D)isplay\r\n");
	uint8_t c = '\0';
	while (c == '\0') 
	{
		c = uart_readc();
	}
	switch (c) {
		case 'S' | 's':
            
            
			
            //Engineer the code here to Set the current Date in the DS3231M
            
            
            
            break;
		case 'D' | 'd':
			bcm2835_i2c_begin();
			bcm2835_i2c_setClockDivider(BCM2835_I2C_CLOCK_DIVIDER_2500);
			bcm2835_i2c_setSlaveAddress(0x68);
			bcm2835_i2c_read(DOM,*buffer);
			char ones = *buffer[0] & 0x0F;
			char tens = (*buffer[0] >> 4);
			uart_putc(tens+48);
			uart_putc(ones+48);
			bcm2835_i2c_read(MONTH,*buffer);
			ones = *buffer[0] & 0x0F;
			tens = (*buffer[0] >> 4);
			uart_putc('/');
			uart_putc(tens+48);
			uart_putc(ones+48);
			bcm2835_i2c_read(YEAR,*buffer);
			ones = *buffer[0] & 0x0F;
			tens = (*buffer[0] >> 4);
			uart_putc('/');
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

void TIME(void)
{
	uart_puts("\r\nType TIME (S)et or (D)isplay\r\n");
	uint8_t c = '\0';
	while (c == '\0') 
	{
		c = uart_readc();
	}
	switch (c) {
		case 'S' | 's':
			bcm2835_i2c_begin();
			bcm2835_i2c_setClockDivider(BCM2835_I2C_CLOCK_DIVIDER_2500);
			bcm2835_i2c_setSlaveAddress(0x68);
			uart_puts("\r\nSet 24 Hour Time");
			uart_puts("\r\nType Hours (two digits 00-23): ");
			tens = ((uart_readc()-48) << 4) | 0x0F;
			uart_putc((tens >> 4)+48);
			ones = (uart_readc()-48) | 0xF0;
			uart_putc((ones & 0x0F)+48);
			if (BCDtoUint8(tens & ones) < 0 || BCDtoUint8(tens & ones) > 23) 
			{
				*buffer[0] = 0x00;
				uart_puts("\r\nInvalid Hours Value!");
				break;
			}
			else
			{
				*buffer[0] = tens & ones;
			}			
			bcm2835_i2c_write(HRS,*buffer);
			uart_puts("\r\nType Minutes (two digits 00-59): ");
			tens = ((uart_readc()-48) << 4) | 0x0F;
			uart_putc((tens >> 4)+48);
			ones = (uart_readc()-48) | 0xF0;
			uart_putc((ones & 0x0F)+48);
			if (BCDtoUint8(tens & ones) < 0 || BCDtoUint8(tens & ones) > 59) 
			{
				*buffer[0] = 0x00;
				uart_puts("\r\nInvalid Minutes Value!");
				break;
			}
			else
			{
				*buffer[0] = tens & ones;
			}			
			bcm2835_i2c_write(MINS,*buffer);			
			uart_puts("\r\nType Seconds (two digits 00-59): ");
			tens = ((uart_readc()-48) << 4) | 0x0F;
			uart_putc((tens >> 4)+48);
			ones = (uart_readc()-48) | 0xF0;
			uart_putc((ones & 0x0F)+48);
			if (BCDtoUint8(tens & ones) < 0 || BCDtoUint8(tens & ones) > 59) 
			{
				*buffer[0] = 0x00;
				uart_puts("\r\nInvalid Seconds Value!");
				break;
			}
			else
			{
				*buffer[0] = tens & ones;
			}			
			bcm2835_i2c_write(SECS,*buffer);			
			uart_puts("\r\nTime is now set.");
			bcm2835_i2c_end();
			break;
		case 'D' | 'd':
            
            
            
            
			//Engineer the code here to Display the current Time stored in the DS3231M to HyperTerminal
            
            
            
            
            
			break;	
		default:
			uart_puts(MS4);
			TIME();
			break;	
	}
}

void ALARM(void)
{
	uart_puts("\r\nType ALARM (S)et or (D)isplay or (T)est\r\n");
	uint8_t c = '\0';
	while (c == '\0') 
	{
		c = uart_readc();
	}
	switch (c) {
		case 'S' | 's':
			uart_puts("\r\nSet Seconds Alarm");
			uart_puts("\r\nType Starting Alarm Seconds (two digits 05-59): ");
			tens = ((uart_readc()-48) << 4) | 0x0F;
			uart_putc((tens >> 4)+48);
			ones = (uart_readc()-48) | 0xF0;
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
		c = uart_readc();
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
	uart_puts("\r\n(A)DC,(C)ancom,(D)ate,(H)elp,a(L)arm,(R)eset,(S)FT,(T)ime,(V)FP11");
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
}

void command(void)
{
	uart_puts(MS3);
	uint8_t c = '\0';
	while (c == '\0') {
		c = uart_readc();
	}
	switch (c) {
		case 'C' | 'c':
			CANCOM();
			break;
		case 'D' | 'd':
			DATE();
			break;
		case 'T' | 't':
			TIME();
			break;
		case 'L' | 'l':
			ALARM();
			break;
		case 'A' | 'a':
			ADC();
			break;
		case 'R' | 'r':
			RES();
			break;
		case 'S' | 's':
			SFT();
			break;
		case 'H' | 'h':
			HELP();
			break;
		case 'V' | 'v':
			VFP11();
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
	uart_init();
	enable_irq_57();
	enable_arm_irq();
//	if (logon() == 0) while (1) {}
//	banner();
//	HELP();
//	while (1) {command();}
	
	while (1) 
	{
	uart_putc(' ');
	uart_putc('A');
	uart_putc(' ');
	testdelay();
	}
}

void irq_handler(void)
{
    uint8_t c  = uart_readc();
	uart_putc(' ');
	uart_putc(c);
	uart_putc(' ');
}

