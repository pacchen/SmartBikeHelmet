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
#include <stdbool.h>

#define NODEID        2   //must be unique for each node on same network (range up to 254, 255 is used for broadcast) - don't need?
#define NETWORKID     100  //the same on all nodes that talk to each other (range up to 255) - sync word - regsyncvalue

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

void switch_init(void){
	//ddrc pc2
	DDRC &= ~(1<<PC2); //one switch input 	
	DDRD &= ~(1<<PD3);						// 0other switch input 	
	
}

int main(void){
	int init_worked; 
	serial_init(UBRR);
	serial_out_word("in sub board"); 
	
	SPI_init(); 
	char result[100] = {'\0'};
	char temp[9];
	const uint8_t* data = {2}; 
	strcpy(result, "");  
			sprintf(temp, "%06d", data);
			strcat(result, "data is:  ");
			strcat(result, temp);
			serial_out_word(result); 
			//_delay_ms(50000); 
	uint8_t data_size = 1; 
	//switch_init(); // initiate the switch 
	init_worked = RFM69_initialize(RF69_433MHZ, NODEID, NETWORKID); // does initialize set packet mode register or something? I think so
	// along with interrupt and etc. 
	// RFM69_writeReg(REG_IRQFLAGS2, RF_IRQFLAGS2_PAYLOADREADY);
	// RFM69_writeReg(REG_IRQFLAGS1, RF_IRQFLAGS1_MODEREADY); 
	serial_init(UBRR); 
	serial_out_word("out of initialize"); 
	_delay_ms(1000); 
	
	
	uint8_t toAddress = 1; // address of main board node 
	bool requestACK = false; 
	while(1){
		RFM69_setMode(RF69_MODE_RX);
		serial_out_word("inside while"); 
		// checking the state of the switch 
		// if(!(DDRD & (1<<PD3)) && !(DDRC & (1<<PC2))){
			// data[0] = 0b00; 
			
		// }
		// else if(!(DDRD & (1<<PD3)) && (DDRC & (1<<PC2))){
				// data[0] = 0b01; 
		// }
		// else if((DDRD & (1<<PD3)) && !(DDRC & (1<<PC2)){
			// data[0] = 0b10; 
		// }
		
		if(RFM69_canSend()){
			_delay_ms(1000); 
			serial_out_word("inside cansend"); 
			RFM69_send(toAddress, data, data_size, requestACK);
			serial_out_word("finished send"); 
			//_delay_ms(10000); 
			// if(RFM69_ACKReceived(NODEID)){
				// serial_out_word("ack received"); 
			// }
			// else{
				// serial_out_word("ack not received"); 
			// }
		}
		 
			// strcpy(result, "");  
			// sprintf(temp, "%06d", data);
			// strcat(result, "data is:  ");
			// strcat(result, temp);
			// strcat(result, " datasize is:  ");	
			// sprintf(temp, "%06d", data_size);
			// strcat(result, temp);
			// serial_out_word(result); 
		
	}
		

}
	
	
	
	
	
	
	
	


	
	
	
	
	
	
	
	
	
