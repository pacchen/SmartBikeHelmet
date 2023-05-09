#include <avr/io.h>
#include <util/delay.h>
#include "i2c.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <avr/interrupt.h>

const unsigned short UBRR = 63;
int BDIV = 40; 
volatile uint16_t numuS = 0; 

/*
serial_init - Initialize the USART port
EE 459Lx, Rev. 6/3/2022 5
*/
void serial_init ( unsigned short ubrr ) {
	UBRR0 = ubrr ; // Set baud rate
	UCSR0B |= (1 << TXEN0 ); // Turn on transmitter
	UCSR0B |= (1 << RXEN0 ); // Turn on receiver
	UCSR0C = (3 << UCSZ00 ); // Set for async . operation , no parity ,
	// one stop bit , 8 data bits
}

/*
serial_out - Output a byte to the USART0 port
*/
void serial_out ( unsigned char ch )
{
	while (( UCSR0A & (1 << UDRE0 )) == 0);
	UDR0 = ch ;
}

void serial_out_word ( char *str )
{
	int i=0;
	//int size = strlen(str);
	//int size = 20;
	//for (i = 0; i< strlen(str); i++){
	//	serial_out(str[i]);
	//	_delay_ms(10); 
//
	//}
	while (str[i]!='\0'){
		serial_out(str[i]);
		i++;
	}
	serial_out('\t');
	_delay_ms(10);
	serial_out('\t');
	_delay_ms(10);

}
/*
serial_in - Read a byte from the USART0 and return it
*/
char serial_in ()
{
	while ( !( UCSR0A & (1 << RXC0 )) );
	return UDR0 ;
}

void accelerometer_init() {
	//accelerometer - 1 byte I2C address 
	// write address with SD0 tied high 00110010b = 0x32
	// read address: 00110011b = 0x33 -> ? I think the i2c code increments the write address for read itself so 
	// sub-byte address in datasheet is probably the internal address?
	unsigned char abuf[5] = {0x20,0x67,0x23,0x80,0x07}; //
	//unsigned char statusbuf ;
	_delay_ms(1000); 

	unsigned char status;
	status = i2c_io (0x32 , abuf , 2 , NULL , 0 , NULL , 0);
	status = i2c_io (0x32 , abuf+2 , 2 , NULL , 0 , NULL , 0);
	//do{
	//	status = i2c_io (0x32 , abuf+4 , 1 , NULL , 0 , &statusbuf , 1);
	//}while(statusbuf!=0x08);
}

void read_accelerometer(int* accel_x, int* accel_y, int* accel_z) {
	enum
	{
		OUT_XL = 0xA8,
		OUT_XH = 0x29, 
		//OUT_YL = 0x2A, 
		//OUT_YH = 0x2B,
		//OUT_ZL = 0x2C, 
		//OUT_ZH = 0x2D
	}; 
	unsigned char abuf_r[2] = {OUT_XL,OUT_XH};//,OUT_YL,OUT_YH,OUT_ZL,OUT_ZH};
	unsigned char rbuf [6] ; 
	unsigned char status;

	status = i2c_io (0x32 , abuf_r , 2 , NULL , 0 , rbuf , 6); // Read data from accelerometer through I2C

	int hex_val; 

	hex_val = (int)((signed char) (((int)rbuf[1] << 4 )| ((int)rbuf[0]))) ;
	*accel_x = hex_val;

	hex_val = (int)((signed char) (((int)rbuf[3] << 4 )| ((int)rbuf[2]))) ;
	*accel_y = hex_val;

	hex_val = (int)((signed char) (((int)rbuf[5] << 4 )| ((int)rbuf[4]))) ;
	*accel_z = hex_val;
}

void init() {
	DDRD = 0xFF;							// Port D all output. Display: DB0 - DB7 as PD0 - PD7
	DDRC = 0xFF;							// Port C all output. PC0: RW		PC1: RS		PC2: E
	//--- right ultrasonic sensor 
	DDRD &= ~(1<<DDD2);						// Set Pin D2 as input to read Echo
	PORTD |= (1<<PORTD2);					// Enable pull up on D2
	PORTD &= ~(1<<PD3);						// Init D3 as low (trigger)

	

	PRR &= ~(1<<PRTIM1);					// To activate timer1 module
	TCNT1 = 0;								// Initial timer value
	TCNT2 = 0;								// Initial timer value

	//TCCR1B |= (1<<CS11);					// Timer prescaller 8.


	TCCR1B |= (1<<CS11);					// Timer prescaller 64.
	TCCR1B |= (1<<CS10);					// ...

	//TCCR1B |= (1<<CS10);					// Timer without prescaller. Since default clock for atmega328p is 1Mhz period is 1uS

	TCCR1B |= (1<<ICES1);					// First capture on rising edge

	PCICR |= (1<<PCIE2);						// Enable PCINT[16:23] - port D group interrupts 
	PCICR |= (1<<PCIE0);						// Enable PCINT[0:7] - port B group interrupts 	
	PCMSK2 |= (1<<PCINT18);					// Enable D2 interrupt - for right ultrasonic sensor - which pins of the activated port will trigger interrupt
	PCMSK0 |= (1<<PCINT7);					// Enable B7 interrupt - for middle ultrasonic sensor 	
	PCMSK2 |= (1<<PCINT22);					// Enable D6 interrupt - for left ultrasonic sensor 

	sei();									// Enable Global Interrupts								// Enable Global Interrupts
}




int main(void)
{
	init();
    serial_init(UBRR);
	i2c_init ( BDIV ); 	
	accelerometer_init();

    char character = '0';
    int num = 0;
	int accel_x, accel_y, accel_z;
	char temp[7];
	char result[50] = {'\0'}; 
	int hex_val = 0; 
	///uint16_t numuS_old = numuS; 

    while(1) {

		strcpy(result, " ");		


		//-----------------------------------------------------
		//ultrasonic right sensor 
		PORTD |= (1<<PD3);						// Set trigger high
		_delay_us(10);							// for 10uS
		PORTD &= ~(1<<PD3);	
		// to trigger the ultrasonic module
		//if(numuS != numuS_old){
			
		hex_val = (numuS)/58; 
		sprintf(temp, "%06d", hex_val);
		strcat(result, "; \t Distance is ");
		strcat(result, temp);
		strcat(result, " cm ");	
		serial_out_word(result); 
		//}
		//numuS_old = numuS; 


		//
		_delay_ms(50);
    }

    return 0;   /* never reached */
}

ISR(PCINT2_vect) {
	if (bit_is_set(PIND,PB2)) {					// Checks if echo is high for middle sensor 
		TCNT1 = 0;								// Reset Timer
		//PORTC |= (1<<PC3);
		//serial_out_word("INTERRUPT IF...........");

	} else {
		numuS = TCNT1*64/9.830;					// Save Timer value
		//numuS = TCNT1;
		uint8_t oldSREG = SREG;
		cli();									// Disable Global interrupts
		SREG = oldSREG;							// Enable interrupts
	}
}

// interrupts on port B
ISR(PCINT0_vect) {
	if (bit_is_set(PINB,PB7)) {					// Checks if echo is high for middle sensor 
		TCNT1 = 0;								// Reset Timer
		//numuS_m = TCNT1;
	}
	else {
		numuS_m = (TCNT1)*8/9.830;
		//numuS_m = (TCNT1 - numuS_m)*1024/9.830;					// Save Timer value
		uint8_t oldSREG = SREG;
		cli();									// Disable Global interrupts
		SREG = oldSREG;							// Enable interrupts
	}
}
