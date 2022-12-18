#include "Clock.h"
#include <intrins.h>

bit gpsReady;
char pdata gps[64];

const char code *gpsData  ="$GPRMC,"; //not the right one no time repair
bit dat;
void uart()  interrupt 4 using 1  //uart INTERRUPT
{
	static unsigned char inx;
	unsigned char byte;
	if (_testbit_(RI0))
	{
    byte=SBUF0;
	  if (gpsReady==0)
        {
         if (dat==0)
           {
            if ((byte==gpsData[inx])||(inx==2)) //using GPS with glonas is GN insteed GP
            {
                inx++;
                if (inx==7)
                {
                    dat=1;
                    inx=0;
                }
            }
            else
            {
                inx=0;
            }
           }
         else
         {
           gps[inx] = byte;
           inx++;
           if ((byte=='*')||(inx>61))
           {
              dat=0;
              inx=0;
              if ((gps[11]=='A')&&(inx<60))
                  gpsReady=1;
           }
         } 
        }
			}
	TI0=0;
}


long gps_read()
{
          // Valid GPS data is here
             int hlp,ind, hh,mm,ss,dd,mn,yy;

          long t;

              //rtc[0]=gps[5]&0xf+((gps[4]&0x0f)<<4)+1; //start soscilatur + 1 sec correction
              
              hh=(gps[0]&0xf)*10 + (gps[1]&0xf);
              mm=(gps[2]&0xf)*10 + (gps[3]&0xf);
              ss=(gps[4]&0xf)*10 + (gps[5]&0xf);
              

              for (hlp=0,ind=5;hlp<8;)
                  if (gps[ind++]==',')
                      hlp++;

              
              dd=(gps[ind+1]&0xf)+((gps[ind]&0x0f))*10;
              ind+=2;
              mn=(gps[ind+1]&0xf)+((gps[ind]&0x0f))*10;
              ind+=2;
              yy=(gps[ind+1]&0xf)+((gps[ind]&0x0f))*10+2000;      



 
  //conerting to epoch
 
    //January and February are counted as months 13 and 14 of the previous year
               if(mn <= 2)
                {
                 mn += 12;
                 yy -= 1;
                }
 
              //Convert years to days
              t = (365L * (unsigned long)yy) + ((unsigned long)yy / 4L) - ((unsigned long)yy / 100L) + ((unsigned long)yy / 400L);
              //Convert months to days
              t += (30L * (unsigned long)mn) + (3L * ((unsigned long)mn + 1L) / 5L) + (unsigned long)dd;
              //Unix time starts on January 1st, 1970
              t -= 719561L;
              //Convert days to seconds
              t *= 86400L;
             //Add hours, minutes and seconds
             t += (3600L * (unsigned long)hh) + (60L * (unsigned long)mm) + (unsigned long)ss;
 
             //Return Unix time

             return t;								

			}
						