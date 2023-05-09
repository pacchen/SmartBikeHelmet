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
int MAX_GPS_DATA_LENGTH = 32;

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
	}; 
	unsigned char abuf_r[2] = {OUT_XL,OUT_XH};
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

void read_GPS(int* GPS_data) {
	unsigned char GPSbuffer[MAX_GPS_DATA_LENGTH]; 
	unsigned char status;
 
	status = i2c_io (0x20 , NULL , 0 , NULL , 0 , GPSbuffer , MAX_GPS_DATA_LENGTH); // Read data from accelerometer through I2C

	char GPS_data_converted[MAX_GPS_DATA_LENGTH * 2 + 1];
	int i;
	for (i = 0; i < MAX_GPS_DATA_LENGTH; i++) {
		sprintf(&GPS_data_converted[i * 2], "%02c", GPSbuffer[i]);
	}
	GPS_data_converted[MAX_GPS_DATA_LENGTH * 2] = '\0';

	*GPS_data = GPS_data_converted; 
}


int main(void)
{
    serial_init(UBRR);
	i2c_init ( BDIV ); 	
	//accelerometer_init();

    char character = '0';
    int num = 0;
	int accel_x, accel_y, accel_z;
	char GPS_data;
	char temp[32];
	char result[50] = {'\0'}; 
	int hex_val = 0; 
	uint16_t numuS_old = numuS; 

    while(1) {
		// read_accelerometer(&accel_x, &accel_y, &accel_z);

		// //----------------------------------------------------
		// //printing the accel x, y, z

		// strcpy(result, "(X: ");		
		// sprintf(temp, "%06d", accel_x);
		// strcat(result, temp);
		// strcat(result, "; y: ");
		// sprintf(temp, "%06d", accel_y);
		// strcat(result, temp);
		// strcat(result, "; z: ");
		// sprintf(temp, "%06d", accel_z);
		// strcat(result,  temp);
		// strcat(result,  ") ");
		//serial_out_word(result); 


		//-----------------------------------------------------
		// GPS
		strcpy(result, "");
		read_GPS(&GPS_data);
		sprintf(temp, "%c", GPS_data);
		strcat(result, "\t GPS: ");
		strcat(result, temp);
		serial_out_word(result);
		_delay_ms(100);
    }

    return 0;   /* never reached */
}