#include "Clock.h"
#include <stdio.h>
extern union rtc_data idata my_data;
unsigned long epoch; 
void Init_Device(void);
void test();
extern unsigned char xdata screen[];
void ptr1(char c,unsigned char k);
extern unsigned  char sec,min,hour,day,month,year;

bit correct;
extern bit gpsReady;

void main()
{
	unsigned int i;
//	unsigned char c;
	Init_Device();
	WATCHDOG;
  mbiInit();
	TR2=1; //start timer 2 to 800Hz
	TR1=1;
	WATCHDOG;

	for (i=0;i<(NUM_OF_MBI*NUM_OF_COLUMNS*16);i++)
	 screen[i]=0x0;
  mbiSend();
	rtc_init();
	if (!rtc_readram())
	{		
    for (i=0;i<sizeof( my_data.rtc_ram);i++)
       my_data.rtc_ram[i]=0;
		

    my_data.dat.INTENSITY=5;
    my_data.dat.GMT=0;
    rtc_writeram();            
		rtc_writeTime( 0L);
	}
	my_data.dat.INTENSITY=50;
	my_data.dat.GMT=3;
	rtc_writeram(); 
	WATCHDOG;
	//rtc_writeTime( 1565723036L);
	i=0;
	while (1) {
		  WATCHDOG;
		  while (!TF2H); //800HZ refresh rate
		  TF2H=0;
		  epoch=rtc_readTime();
		  date_time(epoch);

	   	printf("\f%02bu%02bu%02bu",hour,min,sec);//mine 
			screen [42]=(sec&1)?0:90;//mine
				/*
		  if ((sec%57)>3)
			{
			 printf("\f%02bu%02bu%02bu",hour,min,sec);
			 screen [42]=(sec&1)?0:80;
			 screen [43]=1;//mine
			 screen [44]=0;//mine
			}
		  else
			
			{
			 printf("\f%2bu%02bu%02bu",day,month,year);
		   screen [42]=0;
			 screen [43]=0;//mine
			 screen [44]=1;//mine
			}
			*/
			my_data.dat.INTENSITY=((hour>18) & (hour<5))?100:80;
		  mbiSend();
      if ( (hour==0) && (min==0) && (sec==0) && correct)  //evry one hour correct the time
			{
				correct=0;
				gpsReady=0;
				LED=1;
			}
			if (( correct==0) && (gpsReady))
			{
				LED=0;
				epoch=gps_read();
				rtc_writeTime( epoch);
				correct=1;
			}
		
	
	}
}
