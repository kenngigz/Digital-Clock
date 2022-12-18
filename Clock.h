#include "compiler_defs.h"
#include "C8051F410_defs.h"

#define WATCHDOG PCA0CPH5=0

#define NUM_OF_MBI 3
#define NUM_OF_COLUMNS 1

sbit LED     = P1^2; //test led
sbit mbiLE   = P2^4; //SPI CE for MBI



#define  RST sel1
#define  CNT sel0

sbit Dclk   =   P0^2;
sbit sdi    =   P0^6;
sbit sdo    =   P0^3;
#define MOSI sdi
#define MISO sdo
#define SCK  Dclk



union  rtc_data
 {
	  
	  struct dat {
									unsigned char INTENSITY;
			            char GMT;
									unsigned char DSTSET;
									unsigned char DST;

		} dat;
		unsigned char rtc_ram[sizeof(struct dat)+2]; //2 is the crc bytes
 };


void mbiInit();
void mbiSend();
 
bit rtc_readram();
void rtc_writeram();
bit rtc_init();
unsigned long rtc_readTime();
void rtc_writeTime( unsigned long time);
void date_time(unsigned long epoch);
unsigned long date_time_to_epoch();
 
 long gps_read();