#include "Clock.h"


union rtc_data idata my_data;
unsigned  char sec,min,hour,day,month,year;

void rtc_write(unsigned char reg,unsigned char dat)
{
  while ((RTC0ADR&0x80)==0x80); //wait bussy flag
  RTC0ADR=reg;
  RTC0DAT=dat;
}

unsigned char rtc_read(unsigned char reg)
{
   while ((RTC0ADR&0x80)==0x80); //wait bussy flag
	 RTC0ADR=reg;
   RTC0ADR=reg|0x80;     //write busy 1
   while ((RTC0ADR&0x80)==0x80); //wait bussy flag
   return RTC0DAT;
} 

bit rtc_readram()
{
    unsigned char addr;
// Read from the entire smaRTClock RAM
    CRC0CN=0x18;    //init crc generator
	RTC0ADR = 0x0E;// address the RAMADDR register
	RTC0DAT = 0x00;// write the address of 0x00 to RAMADDR
	while ((RTC0ADR & 0x80) == 0x80); // poll on the BUSY bit
	RTC0ADR = 0x0F; // address the RAMDATA register
	for (addr = 0; addr < sizeof(my_data.rtc_ram)-2; addr++)
	{
	 RTC0ADR |= 0x80; // initiate a read of the RAMDATA register
	 while ((RTC0ADR & 0x80) == 0x80); // poll on the BUSY bit
	 my_data.rtc_ram[addr] = RTC0DAT; // copy the data from the entire RAM
	 CRC0IN=my_data.rtc_ram[addr];
	}

	RTC0ADR |= 0x80; // initiate a read of the RAMDATA register
	while ((RTC0ADR & 0x80) == 0x80); // poll on the BUSY bit
	CRC0CN=0x10;   //select crc byte 0
	if (RTC0DAT!=CRC0DAT) 
	 return 0;

	RTC0ADR |= 0x80; // initiate a read of the RAMDATA register
	while ((RTC0ADR & 0x80) == 0x80); // poll on the BUSY bit
	CRC0CN=0x11;   //select crc byte 0
	if (RTC0DAT!= CRC0DAT) 
	 return 0;
  return 1;

}

void rtc_writeram()
{
   unsigned char addr;
   CRC0CN=0x18;    //init crc generator
// Write to the entire smaRTClock RAM
	RTC0ADR = 0x0E;// address the RAMADDR register
	RTC0DAT = 0x00;// write the address of 0x00 to RAMADDR
	while ((RTC0ADR & 0x80) == 0x80); // poll on the BUSY bit
	RTC0ADR = 0x0F;// address the RAMDATA register
	for (addr = 0; addr <  sizeof(my_data.rtc_ram)-2; addr++)
	{
	 CRC0IN=my_data.rtc_ram[addr];
	 RTC0DAT = my_data.rtc_ram[addr]; // write 0xA5 to every RAM address
	 while ((RTC0ADR & 0x80) == 0x80);// poll on the BUSY bit
	}
	CRC0CN=0x10;   //select crc byte 0
	RTC0DAT = CRC0DAT; // write 0xA5 to every RAM address
	while ((RTC0ADR & 0x80) == 0x80);// poll on the BUSY bit

 	CRC0CN=0x11;   //select crc byte 1
	RTC0DAT = CRC0DAT; // write 0xA5 to every RAM address
	while ((RTC0ADR & 0x80) == 0x80);// poll on the BUSY bit   

}

//setup rtc in self oscilating mode !!!!!
bit rtc_init()
{
  bit res;
  unsigned char c;

  res=1;
  if (RTC0KEY==3)
   // problem RTC locked need reset
    RSTSRC|=0x20;    //software reset
    //unlock RTC
  if (RTC0KEY==0)
   {
     //unlock the RTC is no locked
     RTC0KEY = 0xA5;
     RTC0KEY = 0xF1;
   };

  c=rtc_read(6); //READ RTC0CN
  if (((c&0x20)==0x20) || ((c&0x80)==0))  //check OSCFAIL
   {
    res=0;   //save status for return value
	  rtc_write(6,0); //disable RTC  
	  rtc_write(7,0x40); //enale crystal mode
		rtc_write(7,0xc0); //enale atogain
	  rtc_write(6,0x80); //enable auto gain
	  while ((rtc_read(7)&0x10)!=0x10); //clock be stable not nessery for self oscilating mode
	 // rtc_write(7,0x40); //disable autogain
   //	rtc_write(6,0xd0); //start counting enable missingo clock

   }
   return res;
}

unsigned long rtc_readTime()
{
  unsigned long time;
  unsigned char c,d;

   rtc_write(6,0xD1);            //transwer timer value to capture register
   while ((rtc_read(6)&0x1)==1); // wait time transwer to capture register

   for (c=2,d=4;d!=0;c++)  
      ((unsigned char *) &time)[--d]=rtc_read(c);

   return time;

}

void rtc_writeTime( unsigned long time)
{

  unsigned char c,d;

   rtc_write(0,0);
	 rtc_write(1,0);
   for (c=2,d=4;d!=0;c++)  
     rtc_write(c,((unsigned char *) &time)[--d]);

   rtc_write(6,0xD2);    //transwer timer value to capture register
   while ((rtc_read(6)&0x2)==2); // wait time transwer to capture register

}

//-----------------------------------------------------------------------------
//epoch operation

const unsigned code days[4][12] =
{
    {   0,  31,  60,  91, 121, 152, 182, 213, 244, 274, 305, 335},
    { 366, 397, 425, 456, 486, 517, 547, 578, 609, 639, 670, 700},
    { 731, 762, 790, 821, 851, 882, 912, 943, 974,1004,1035,1065},
    {1096,1127,1155,1186,1216,1247,1277,1308,1339,1369,1400,1430},
};
void date_time(unsigned long epoch)
{
	  unsigned int yearsa,yeara;
	  unsigned char montha;
	  epoch-=946684800L-(3L*3600L);   //corect the GMT lazy to calculate later 
    sec  = epoch%60; epoch /= 60;
    min  = epoch%60; epoch /= 60;
    hour = epoch%24; epoch /= 24;

    yearsa = epoch/(365*4+1)*4; epoch %= 365*4+1;

    for (yeara=3; yeara>0; yeara--)
    {
        if (epoch >= days[yeara][0])
            break;
    }


    for (montha=11; montha>0; montha--)
    {
        if (epoch >= days[yeara][montha])
            break;
    }
    month = montha+1;
    day   = epoch-days[yeara][montha]+1;
		year=(yearsa+yeara)%100;
}

