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
int MAX_GPS_DATA_LENGTH = 120;

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
	while (str[i]!='\0'){
		serial_out(str[i]);
		i++;
	}
	//serial_out('\t');
	//_delay_ms(10);
	//serial_out('\t');
	//_delay_ms(10);

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



int main(void)
{
    serial_init(UBRR);
	i2c_init ( BDIV ); 	
	//accelerometer_init();

    char character = '0';
    int num = 0;
	int accel_x, accel_y, accel_z;
	char *GPS_data;
	char temp[32];
	char result[50] = {'\0'}; 
	int hex_val = 0; 
	uint16_t numuS_old = numuS; 


	_delay_ms(1000);
	//phonechip 

	//AT – This is the most basic AT command. It also initializes the Auto-bauder. If all is well, it sends the OK message.
	//serial_out_word("ATI ");
	serial_out_word("sending stuff to gsm: \r\n");

	// while(1){
	// //  serial_out('A');
	// //  serial_out('T');
	// //   serial_out('\r');
	// //   serial_out('\n');
	//   	//_delay_ms(1000);
	// char * str = "AT\r\n";
	// int i=0;
	// while (str[i]!='\0'){
	// 	serial_out(str[i]);
	// 	i++;
	// }
	// }


	//_delay_ms(500);
	// serial_out(serial_in());
	// serial_out(serial_in());
	// serial_out(serial_in());
	// serial_out_word(result);
	// serial_out_word("\r\n");
	// //It checks ‘Signal Strength’. The first number in the output response is the signal strength in dB. It should be more than about 5.
	// serial_out_word("AT+COPS?");
	// _delay_ms(500);
	// serial_out_word(serial_in());
	// serial_out_word("\r\n");
	//AT+CCID – It checks whether the SIM card is valid or not and sends the SIM card number.
	serial_out_word("AT+CCID\r\n");
	_delay_ms(500);
	//AT+CREG? – It checks whether you are registered to the network or not. 
	serial_out_word("AT+CREG?\r\n");
	_delay_ms(500);
	serial_out_word("AT+CMGF=1\r\n"); // Configuring TEXT mode
	 _delay_ms(500);
 	serial_out_word("AT+CMGS=\"+12139220237\"\r\n");//change ZZ with country code and xxxxxxxxxxx with phone number to sms
	//serial_out_word("AT+CMGS=\"+13234497255\"");//change ZZ with country code and xxxxxxxxxxx with phone number to sms #Josh's number
	_delay_ms(500);
  	serial_out_word("Help me PAAAAAAAAUL"); //text content 26 means control z for the ending of the sentence
	_delay_ms(500);
	serial_out('\x1A');
	
	serial_out_word("\r\n");

    // while(1) {

	// 	serial_out_word("\r\n--------------------------------------------");
	// 	_delay_ms(200);
    // }

    return 0;   /* never reached */
}