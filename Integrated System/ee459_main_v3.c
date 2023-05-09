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
volatile uint16_t numuS_m = 0;
#define MAX_GPS_DATA_LENGTH 100
//int MAX_GPS_DATA_LENGTH = 1000;
int flag = 0;
int crash_warn = 0;
char gps_sentence [MAX_GPS_DATA_LENGTH] = {'\0'};

char SOS_message[100] = {'\0'};

/*
serial_init - Initialize the USART port
EE 459Lx, Rev. 6/3/2022 5
*/
void serial_init(unsigned short ubrr)
{
	UBRR0 = ubrr;			// Set baud rate
	UCSR0B |= (1 << TXEN0); // Turn on transmitter
	UCSR0B |= (1 << RXEN0); // Turn on receiver
	UCSR0C = (3 << UCSZ00); // Set for async . operation , no parity ,
							// one stop bit , 8 data bits
}

/*
serial_out - Output a byte to the USART0 port
*/
void serial_out(unsigned char ch)
{
	while ((UCSR0A & (1 << UDRE0)) == 0);
	UDR0 = ch;
}

void serial_out_word(char *str)
{
	int i = 0;
	while (str[i] != '\0')
	{
		serial_out(str[i]);
		i++;
	}
}
/*
serial_in - Read a byte from the USART0 and return it
*/
char serial_in()
{
	while (!(UCSR0A & (1 << RXC0)));
	return UDR0;
}

void accelerometer_init()
{
	// accelerometer - 1 byte I2C address
	//  write address with SD0 tied high 00110010b = 0x32
	//  read address: 00110011b = 0x33 -> ? I think the i2c code increments the write address for read itself so
	//  sub-byte address in datasheet is probably the internal address?
	unsigned char abuf[5] = {0x20, 0x67, 0x23, 0x80, 0x07}; //
	// unsigned char statusbuf ;
	_delay_ms(1000);

	unsigned char status;
	status = i2c_io(0x32, abuf, 2, NULL, 0, NULL, 0);
	status = i2c_io(0x32, abuf + 2, 2, NULL, 0, NULL, 0);
}

void read_accelerometer(int *accel_x, int *accel_y, int *accel_z)
{
	enum
	{
		OUT_XL = 0xA8,
		OUT_XH = 0x29,

	};
	unsigned char abuf_r[2] = {OUT_XL, OUT_XH};
	unsigned char rbuf[6];
	unsigned char status;

	status = i2c_io(0x32, abuf_r, 2, NULL, 0, rbuf, 6); // Read data from accelerometer through I2C

	int hex_val;

	hex_val = (int)((signed char)(((int)rbuf[1] << 4) | ((int)rbuf[0])));
	*accel_x = hex_val;

	hex_val = (int)((signed char)(((int)rbuf[3] << 4) | ((int)rbuf[2])));
	*accel_y = hex_val;

	hex_val = (int)((signed char)(((int)rbuf[5] << 4) | ((int)rbuf[4])));
	*accel_z = hex_val;
}

void read_GPS() {
	unsigned char GPSbuffer[MAX_GPS_DATA_LENGTH]; 
	unsigned char status;
	status = i2c_io (0x20 , NULL , 0 , NULL , 0 , GPSbuffer , MAX_GPS_DATA_LENGTH); // Read data from accelerometer through I2C
	char GPS_sentence_converted[MAX_GPS_DATA_LENGTH]; // String to hold converted bytes
	int i;
	int start = 0;
	int str_index = 0;
	int start_index = 0;
	for ( i = 0; i < MAX_GPS_DATA_LENGTH-10; i++) {
		if((char)GPSbuffer[i] == '\r' && (char)GPSbuffer[i+1] == '\n'){
			if((char)GPSbuffer[i+5] == 'G' && (char)GPSbuffer[i+6] == 'G' && (char)GPSbuffer[i+7] == 'A'){
			start_index = str_index; 
			start = 1;
			//GPS_sentence_converted = {''};
			}
		}
		if(start && !((char)GPSbuffer[i] == '\r' || (char)GPSbuffer[i+1] == '\n')){
			GPS_sentence_converted[str_index] = (char)GPSbuffer[i]; // Convert each byte to its ASCII character equivalent
			if((char)GPSbuffer[i-2] == '*'){
				start = 0;
				start_index = str_index+1;
				GPS_sentence_converted[start_index+1] = '\0';

				strcpy(gps_sentence, GPS_sentence_converted); // update global variable

			}		
			str_index ++; 
		}
	}
	if (start ==1){
		GPS_sentence_converted[start_index+1] = '\0';
	}
	//strcpy(gps_sentence, "No GPS signal collected"); // update global variable


}

void SOS() {
	serial_out_word("sending SOS\r\n");
	_delay_ms(500);
	serial_out_word("AT+CCID\r\n");
	_delay_ms(500);
	//AT+CREG? – It checks whether you are registered to the network or not. 
	serial_out_word("AT+CREG?\r\n");
	_delay_ms(500);
	serial_out_word("AT+CMGF=1\r\n"); // Configuring TEXT mode
	 _delay_ms(500);
 	//serial_out_word("AT+CMGS=\"+12139220237\"\r\n");//change ZZ with country code and xxxxxxxxxxx with phone number to sms
	//serial_out_word("AT+CMGS=\"+16267314315\"\r\n");//change ZZ with country code and xxxxxxxxxxx with phone number to sms
	serial_out_word("AT+CMGS=\"+13234497255\"\r\n");//change ZZ with country code and xxxxxxxxxxx with phone number to sms
	_delay_ms(500);
	//strcpy(SOS_message, "");
	//strcat(SOS_message, "I need help! Location info: ");
	//strcat(SOS_message, GPS);
  	//serial_out_word(SOS_message); //text content 26 means control z for the ending of the sentence
	char msg[100] = {'\0'};
	
	strcpy(msg, "I need help! Location info: ");
	strcat(msg, gps_sentence);
	serial_out_word(msg);
	_delay_ms(500);
	serial_out('\x1A');
	
	serial_out_word("\r\n");
	_delay_ms(10000);
}

void init()
{
	DDRD = 0xFF; // Port D all output. Display: DB0 - DB7 as PD0 - PD7
	DDRC = 0xFF; // Port C all output. PC0: RW		PC1: RS		PC2: E
	//--- right ultrasonic sensor
	DDRD &= ~(1 << DDD2);	// Set Pin D2 as input to read Echo
	PORTD |= (1 << PORTD2); // Enable pull up on D2
	PORTD &= ~(1 << PD3);	// Init D3 as low (trigger)
	//--- middle ultrasonic sensor
	DDRB &= ~(1 << DDB7);	// Port B set pin B7 as input to read echo
	PORTB |= (1 << PORTB7); // Enable pull up on B7
	PORTD &= ~(1 << PD5);	// Init D5 as low (trigger)
	//--- left ultrasonic sensor
	DDRD &= ~(1 << DDD6);	// Port D set pin D6 as input to read echo
	PORTD |= (1 << PORTD6); // Enable pull up on D6
	PORTD &= ~(1 << PD7);	// Init D7 as low (trigger)

	PRR &= ~(1 << PRTIM1); // To activate timer1 module
	TCNT1 = 0;			   // Initial timer value
	TCNT2 = 0;			   // Initial timer value

	// TCCR1B |= (1<<CS11);					// Timer prescaller 8.

	// LEDs
	PORTC &= ~(1<<PC1);    // Left Turn Signal
	PORTC &= ~(1<<PC2);    // Right Turn Signal

	TCCR1B |= (1 << CS10); // Timer prescaller 64.
	TCCR1B |= (1 << CS11); // ...

	TCCR1B |= (1 << ICES1); // First capture on rising edge

	PCICR |= (1 << PCIE2);	  // Enable PCINT[16:23] - port D group interrupts
	PCICR |= (1 << PCIE0);	  // Enable PCINT[0:7] - port B group interrupts
	PCMSK2 |= (1 << PCINT18); // Enable D2 interrupt - for right ultrasonic sensor - which pins of the activated port will trigger interrupt
	PCMSK0 |= (1 << PCINT7);  // Enable B7 interrupt - for middle ultrasonic sensor
	PCMSK2 |= (1 << PCINT22); // Enable D6 interrupt - for left ultrasonic sensor

	sei();			// Enable Global Interrupts
	DDRB |= (0x03); // Port B all output.
}

int main(void)
{
	init();
	serial_init(UBRR);
	i2c_init(BDIV);
	accelerometer_init();
	// ultrasonic
	int ultra_right = 100;
	int ultra_left = 0;
	int ultra_middle = 0;
	int c = 1;
	int hex_val = 0;
	int warn_l = 0;
	int warn_r = 0;
	int warn_m = 0;
	int vibrating = 0;
	int vibrating_count = 0;
	int left_turn = 0;
	int right_turn = 0;

	// accelerometer
	int accel_x = 0;
	int accel_y = 0;
	int accel_z = 0;
	// output
	char character = '0';
	int num = 0;
	//char temp[32];
	//char result[50] = {'\0'}; 
	char GPS_temp[100] = {'\0'};
	_delay_ms(1000);

	while (1)
	{
		//serial_out_word(gps_sentence);
		read_GPS();
		// _delay_ms(1000);
		// SOS();
		if (crash_warn == 0) {
			
			// Checking acceleration
			read_accelerometer(&accel_x, &accel_y, &accel_z);
			if((accel_x*accel_x + accel_y*accel_y + accel_z*accel_z) > 4000 && accel_y > 0)
				PORTD |= (1<< PD7);
			else 
				PORTD &= ~(1<< PD7);
			if ((accel_x*accel_x + accel_y*accel_y + accel_z*accel_z) > 14400) {
				crash_warn = 1;
				c = 0;
			}

			//-----------------------------------------------------


			//-----------------------------------------------------
			// ultrasonic
			// B, A = (PD3, PD5) -> 00 01(right) 10(middle) 11(left) 
			if (flag == 0)
			{

				flag = 1;
				if (c == 0)
				{
					PORTD |= (1 << PD3); // Set trigger high
					_delay_us(100);		 // for 10uS
					PORTD &= ~(1 << PD3);
					hex_val = (numuS)*64 / 58;
					// ultra_left = ultrasonic_val('m');
					// if (hex_val < 800)
					ultra_right = hex_val;
				}
				else if (c == 1)
				{
					// PORTD |= (1<<PD3);						// Set trigger high
					//_delay_us(10);							// for 10uS
					// PORTD &= ~(1<<PD3);
					PORTD |= (1 << PD5); // Set trigger high
					PORTD |= (1 << PD3); // Set trigger high

					_delay_us(100); // for 10uS
					PORTD &= ~(1 << PD5);
					PORTD &= ~(1 << PD3);

					hex_val = (numuS)*64 / 58;
					// ultra_right = ultrasonic_val('m');
					// if (hex_val < 800)
					ultra_left = hex_val;
				}
				else
				{
					PORTD |= (1 << PD5); // Set trigger high
					_delay_us(100);		 // for 10uS
					PORTD &= ~(1 << PD5);
					hex_val = (numuS_m)*64 / 58;
					// ultra_middle = ultrasonic_val('m');
					// if (hex_val < 800)
					ultra_middle = hex_val;
				}
				if (c++ == 2)
					c = 1;
			}
			/*
			strcpy(result, "\r\n");

			//--- right sensor
			// ultrasonic
			sprintf(temp, "%06d", ultra_right);
			strcat(result, "R: ");
			strcat(result, temp);
			strcat(result, " cm ");
			//--- middle sensor
			// ultrasonic
			sprintf(temp, "%06d", ultra_middle);
			strcat(result, "; M: ");
			strcat(result, temp);
			strcat(result, " cm ");
			//--- left sensor
			sprintf(temp, "%06d", ultra_left);
			strcat(result, "; L: ");
			strcat(result, temp);
			strcat(result, " cm ");
			//----------------------------------------------------
			// printing the accel x, y, z
			strcat(result, "; \t (X: ");
			sprintf(temp, "%06d", accel_x);
			strcat(result, temp);
			strcat(result, "; y: ");
			sprintf(temp, "%06d", accel_y);
			strcat(result, temp);
			strcat(result, "; z: ");
			sprintf(temp, "%06d", accel_z);
			strcat(result, temp);
			strcat(result, ")   ");

			sprintf(temp, "%06d", warn_m);
			strcat(result, temp);
			sprintf(temp, "%06d", warn_l);
			strcat(result, temp);

			// serial_out_word(result);

			serial_out_word(result);
			*/
			//serial_out_word(read_GPS());
			//serial_out_word("\r\n--------------------------------------------");
			_delay_ms(100);
			//////////////////////// output controls
			if ((ultra_middle < 10) && (warn_m == 0)){
				warn_m = 1;
				vibrating = 1;
			}
			else if ((ultra_right < 10) && (warn_r == 0)){
				warn_r = 1;
				vibrating = 4;
			}
			else if ((ultra_left < 10) && (warn_l == 0)){
				warn_l = 1;
				vibrating = 7;
			}
			else {
				PORTB &= ~(1<<PORTB0);						// right vibrator
				PORTB &= ~(1<<PORTB1);						// left viberator
			}

			switch(vibrating) {
				case 1:
					PORTB |= (1<<PORTB0);						// left vibrator
					PORTB |= (1<<PORTB1);						// right vibrator
					break;
				case 2: 
					PORTB &= ~(1<<PORTB0);
					PORTB &= ~(1<<PORTB1);
					break;
				case 3:
					PORTB |= (1<<PORTB0);
					PORTB |= (1<<PORTB1);
					break;
				case 4:
					PORTB &= ~(1<<PORTB0);						// left vibrator
					PORTB |= (1<<PORTB1);						// right viberator
					break;
				case 5:
					PORTB &= ~(1<<PORTB1);
					break;
				case 6:
					PORTB |= (1<<PORTB1);
					break;
				case 7:
					PORTB |= (1<<PORTB0);						// left vibrator
					PORTB &= ~(1<<PORTB1);						// right viberator
					break;
				case 8:
					PORTB &= ~(1<<PORTB0);
					break;
				case 9:
					PORTB |= (1<<PORTB0);
					break;
				default:
					PORTB &= ~(1<<PORTB0);						// right vibrator
					PORTB &= ~(1<<PORTB1);						// left viberator
			}

			if (vibrating != 0) {
				if (((vibrating == 3) || (vibrating == 6) || (vibrating == 9)) && vibrating_count == 2) {
					vibrating = 0;
					vibrating_count = 0;
				}else if (vibrating_count == 2) {
					vibrating++;
					vibrating_count = 0;
				} else {
					vibrating_count++;
				}
			}

			if ((ultra_middle > 20) && (warn_m == 1)) {
				warn_m = 0;
			}
			if ((ultra_left > 20) && (warn_l == 1)) {
				warn_l = 0;
			}
			if ((ultra_right > 20) && (warn_r == 1)) {
				warn_r = 0;
			}
			if ((ultra_left < 5) && (left_turn == 0)) {
				left_turn = 1;
			}

			if ((ultra_middle < 5) && (right_turn == 0)) {
				right_turn = 1;
			}

			if (left_turn > 0) {
				left_turn++;
				if (left_turn >= 9) {
					PORTC &= ~(1<<PC1);
					left_turn = 0;
				} else if ((left_turn <= 4) || (left_turn >= 6)) {
					PORTC |= (1<<PC1);
				} else if ((left_turn > 4) && (left_turn < 6)) {
					PORTC &= ~(1<<PC1);
				}
			}

			if (right_turn > 0) {
				right_turn++;
				if (right_turn >= 9) {
					PORTC &= ~(1<<PC2);
					right_turn = 0;
				} else if ((right_turn <= 4) || (right_turn >= 6)) {
					PORTC |= (1<<PC2);
				} else if ((right_turn > 4) && (right_turn < 6)) {
					PORTC &= ~(1<<PC2);
				}
			}
		} else {
			// Biker probably crashed
			// Gives 10 second warning before an emergency message is sent
			PORTB |= (1<<PORTB0);						// left vibrator
			PORTB |= (1<<PORTB1);						// right viberator
			if (c%2 == 0) {
				PORTD |= (1<< PD7);
			} else {
				PORTD &= ~(1<< PD7);
			}
			if (c == 10) {
				_delay_ms(500);
				SOS();
				_delay_ms(500);
				c = 1;
			}
			c++;
			_delay_ms(1000);
		}		
	}

	return 0; /* never reached */
}

// interrupts on port D
ISR(PCINT2_vect)
{
	if (bit_is_set(PIND, PD2) || bit_is_set(PIND, PD6))
	{			   // Checks if echo is high for right or left sensor
		TCNT1 = 0; // Reset Timer
	}
	else
	{
		numuS = TCNT1 / 9.830; // Save Timer value
		uint8_t oldSREG = SREG;
		cli(); // Disable Global interrupts
		SREG = oldSREG;
		flag = 0; // Enable interrupts
	}
}

// interrupts on port B
ISR(PCINT0_vect)
{
	if (bit_is_set(PINB, PB7))
	{ // Checks if echo is high for middle sensor
		TCNT1 = 0;
	}
	else
	{
		numuS_m = (TCNT1) / 9.830; // Save Timer value
		uint8_t oldSREG = SREG;
		cli();			// Disable Global
		SREG = oldSREG; // Enable interrupts
		flag = 0;
	}
}