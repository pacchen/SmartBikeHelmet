// EE459 Team 10 - https://github.com/cristi85/RFM69
//EE459 radio testing
#include "RFM69.h"
#include "RFM69register.h"
#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <avr/interrupt.h>

#define NODEID        1   //must be unique for each node on same network (range up to 254, 255 is used for broadcast) - don't need?
#define NETWORKID     100  //the same on all nodes that talk to each other (range up to 255) - sync word - regsyncvalue
// start serial communication?
// initialize radio (freq, nodeid, networkid)
// set power high?

const unsigned short UBRR = 63;
int BDIV = 40; 

// what are the node and network id (also it seems like they don't use the node id for some reason)
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

int main(void){
	int init_worked; 
	serial_init(UBRR);
	serial_out_word("outside/////////////////"); 
	const void* ack_buf; 
	uint8_t ack_size; 
	char temp[7];
	uint8_t* data; 
	uint8_t data_size; 
	// uint8_t freqBand =  ; 
	//RFM69_getFrequency? do I need to get frequency? - is this one not just 915? why would I need to look in the register? also
	// I think using fr69_915 already chooses the right register in the c code 
	// doesn't the software decide what's in the register 
	SPI_init(); 
	//SPI_transfer8(3); 
	//_delay_ms(50000); 
	//MSPI_init(); 
	init_worked = RFM69_initialize(RF69_433MHZ, NODEID, NETWORKID); // does initialize set packet mode register or something? I think so
	// along with interrupt and etc. 
	
	
	char result[100] = {'\0'};
	// while(1){
		// uint8_t spidat; 
		// uint8_t dat_in;
		// dat_in = 2;
		// //input = 0b10100100; 
		// strcpy(result, "");  
		// sprintf(temp, "%06d", dat_in);
		// strcat(result, "data in while is:  ");
		// strcat(result, temp);
		// serial_out_word(result);
		// serial_out_word("while"); 
		// spidat = SPI_transfer8(dat_in); 
		// // strcpy(result, "");  
		// // sprintf(temp, "%06d", spidat);
		// // strcat(result, "data is:  ");
		// // strcat(result, temp);
		// // serial_out_word(result); 
		// _delay_ms(1000); 
	// }
	while(1){
		while(!RFM69_receiveDone()){ // put radio in listen mode and receivebegin if something is to be received... and handles interrupt for receiving  
			//RFM69_receiveDone(); 
			serial_out_word("rcv not done"); 
			RFM69_receiveDone();
			//_delay_ms(10000); 
		}
		_delay_ms(10000); 
		serial_out_word("receive done!"); 
		//serial_out_word("inside"); 
		// I think I do need to return the data somewhere
		data = RFM69_returnData(); 
		data_size = RFM69_returnDataLen(); 
		strcpy(result, "");  
		sprintf(temp, "%06d", data);
		strcat(result, "data is:  ");
		strcat(result, temp);
		strcat(result, " datasize is:  ");	
		sprintf(temp, "%06d", data_size);
		strcat(result, temp);
		serial_out_word(result); 
		
		//checking the state of the switch on the helmet
		// if(data[0] == 0b00){
		// // led lights are off
		// }
		// else if(data[0] == 0b01){
		// }
		// else if(data[0] == 0b10){
		// }
		
		// if(RFM69_ACKRequested()){
			// RFM69_sendACK(ack_buf, ack_size); // I think it should be sending ack information?
		// }
		
		// do something with the data received... I guess this is received in buffer and bufferSize 
	
	
		//RFM69_unselect(); // not sure if need this later?
		// - but if unselect would have to reinitialize - but should unselect at some point? or just when power off nothing happens?
	
	//don't need to set regopmode bc automatically changes between tx, rx, sleep etc. 
	
	// don't know if I need to set address or if that is just nodeid? RFM69_setAddress
	
	// if received signal strength not good, use this function RFM69_setPowerLevel(uint8_t powerLevel)
	}
	
	
	
	
	
	
	
	
}
