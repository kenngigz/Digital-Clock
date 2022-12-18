#include "Clock.h"
#include <intrins.h>
#include <string.h>

extern union rtc_data idata my_data;


#define NO_LACH            -1

#define DATA_LACH          1
#define GLOBAL_LACH        3
#define READ_CONFIGURATION 5
#define EABLE_ERRORDETECT  7
#define READ_ERRORSTATUS   9
#define WRITE_CONFIG       11
#define RESET_PWM          13



union ctrl {
              struct control {
							 unsigned int bt0:1; // 0    -> timeout of gck 0 - diable 1 enable	
					     unsigned int bt1:1; // 0    -> thermal protection 0- disable  1-enble
					     unsigned int bt9:8; // 0xab -> current gain adjustment
					     unsigned int bta:1; // 1    -> pwm data sinhronization 0- auto  1- manual
					     unsigned int btb:1; // 0    -> pwm counter reset 0-disable 1- enable
					     unsigned int btc:1; // 1    -> 0 - 6 bit pwm 1 - 12 bit pwm
					     unsigned int btd:1; // 1    -> reserved bit
					     unsigned int bte:1; // 0    -> thermal eror flag 0 - ok readonly							 
					     unsigned int btf:1; // 0    -> Data loading 0- 15 time + 1 datalach  1- 16 time +1 datalach
              } control;
             unsigned int i;
					};
					
union  ctrl ctrl;
// default ctrl 0b0011011010101100 - >	0x36ac	
					
static void sendWord(unsigned int w,char le)
{
	
      w=w;
	    le=le;
#pragma asm
;	     CLR  mbiLE
;	     CLR  Dclk
	
	     MOV A,#10H
	     CLR C
	     SUBB A,R5
	     XCH  A,R5
	
		   MOV R4,#8	
		   MOV A,R6		   ;high byte of w
LOOP1:
		   RLC A
		   MOV sdi,C
		   SETB Dclk
		   DJNZ R5,SETLE1  ;le?
		   SETB mbiLE
SETLE1:	
       CLR  Dclk
		   DJNZ R4,LOOP1
		   
		   MOV R4,#8
		   MOV A,R7	   ;LOW BYTE OF W
LOOP2:		  
		   RLC A
		   MOV  sdi,C
		   SETB Dclk
		   DJNZ R5,SETLE2  ;le?
		   SETB mbiLE
SETLE2:	
       CLR  Dclk
		   DJNZ R4,LOOP2		   
       CLR  mbiLE
#pragma endasm

}

//THIS MBI NOT RECIEVE CONFIG OR I MISTAKE BUT 
// IS SET TO 16 + 1 datalach
void mbiInit()
 {
	 unsigned char c;
	 ctrl.i=0x36ac; // default control
   ctrl.control.btf=1;    
	 ctrl.control.bt9=0xff;
	 for (c=0;c<NUM_OF_MBI-1;c++)
	  {
			sendWord(ctrl.i,NO_LACH);
		}
	 sendWord(ctrl.i,WRITE_CONFIG);
 }
 
 

unsigned char xdata screen[NUM_OF_MBI*NUM_OF_COLUMNS*16];

void mbiSend()
 {
	 unsigned char c,d;

   unsigned char *(xdata tmp);

	 tmp=&screen[0];
	 for (d=16;d!=0;d--)
	  {
	   for (c=NUM_OF_MBI-1;c!=0;c--)
			  sendWord(((int) *tmp++)*0x100,NO_LACH);
		 sendWord(((int) *tmp++)*0x100,DATA_LACH);
		}


	 sendWord(((int) (*tmp))*0x100,GLOBAL_LACH);

		
 }
 

																				// 0    1    2   3    4    5   6    7    8    9     C
 const unsigned char code numbers[16]= {0xfc,0x60,0xda,0xf2,0x66,0xb6,0xbe,0xe0,0xfe,0xf6,0x9c,0,0,0,0};
 const unsigned char code segments[6][7] = {{3 , 0,39,36,33, 9,12},
                                           {21,18,15,27,30, 6,45},
																					 { 4, 1,40,37,34,10,13},
																					 {22,19,16,28,31, 7,46},
																					 { 5, 2,41,38,35,11,14},
																					 {23,20,17,29,32, 8,47}};
 
char putchar(char c)
{
	static unsigned char xpos;
	unsigned char i;
	unsigned char mask;
	if (xpos>5) 
		xpos=5;
	switch (c)
	{
		case 'C':
			mask=numbers[10];
		break;
		case '\r':
			xpos=0;
		 return c;
		case '\f':
			xpos=0;
		  memset(screen,0,sizeof(screen));
			return c;
		case ' ':
			mask=0;
		break;
		
		default:
			mask=numbers[c&0x0f];
	}
	
	for (i=0;i<7;i++)
	 {
		 if (mask&0x80)
			 screen[(segments[xpos][i])]=my_data.dat.INTENSITY;
		 else
			 screen[(segments[xpos][i])]=0;
		 mask<<=1;
	 }
	 xpos++;
	 return c;
}


 


	


