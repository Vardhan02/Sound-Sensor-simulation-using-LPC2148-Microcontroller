#include<lpc214x.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#define SOUND (IO1PIN & (1<<24))
 
unsigned int i;

//	declaring functions
void lcd_int(void);		 
void dat(unsigned char);
void cmd(unsigned char);
void string(unsigned char *); 
 

void delay_ms(uint16_t j) /* Function for delay in milliseconds */
{
    uint16_t x,i;
	for(i=0;i<j;i++)
	{
    for(x=0; x<6000; x++);    /* loop to generate 1 millisecond delay with Cclk = 60MHz */
	}
}

 

void lcd_int(void)
{
	IO0DIR = 0x0000FFF0; /* P0.12 to P0.15 LCD Data. P0.4,5,6 as RS RW and EN */
	delay_ms(20);
	cmd(0x02);  /*Initializing cursor to home position (1st row, 1st col) (reserverd) */
	cmd(0x28);  /* 4 bit, 2 lines,5x8 pixels */	 
	cmd(0x0C);   /* Display on, cursor off, blink off*/
	cmd(0x06);  /*  Auto increment cursor, display shift off */
	cmd(0x01);   /* Display clear (reserverd) */
	cmd(0x80);  /* First line first position */
}
 

void cmd(unsigned char a)
{
	
	IO0SET = 0x00000040; /* EN = 1 */
	IO0CLR = 0x00000030; /* RS = 0 (command input), RW = 0 (write)*/
	IO0PIN = ( (IO0PIN & 0xFFFF00FF) | ((a & 0xF0)<<8) ); /* Upper nibble of command */
	delay_ms(5);
	IO0CLR = 0x00000040; /* EN = 0, RS and RW unchanged(i.e. RS = RW = 0)	 */

	delay_ms(5);

	
	IO0SET = 0x00000040; /* EN = 1 */
	IO0CLR = 0x00000030; /* RS = 0, RW = 0 */
	IO0PIN = ( (IO0PIN & 0xFFFF00FF) | ((a & 0x0F)<<12) ); /* Lower nibble of command */
	delay_ms(5);
	IO0CLR = 0x00000040; /* EN = 0, RS and RW unchanged(i.e. RS = RW = 0)	 */
	delay_ms(5);
	
}



void string (unsigned char *p)
{
	uint8_t i=0;
	while(p[i]!=0)
	{
		
		IO0SET = 0x00000050; /* RS = 1, EN = 1 */
		IO0CLR = 0x00000020; /* RW = 0 */
		IO0PIN = ( (IO0PIN & 0xFFFF00FF) | ((p[i] & 0xF0)<<8) );
		delay_ms(2);
		IO0CLR = 0x00000040; /* EN = 0, RS and RW unchanged(i.e. RS = 1, RW = 0) */

		delay_ms(5);

		
		IO0SET = 0x00000050; /* RS = 1, EN = 1 */
		IO0CLR = 0x00000020; /* RW = 0 */
		IO0PIN = ( (IO0PIN & 0xFFFF00FF) | ((p[i] & 0x0F)<<12) );
		delay_ms(2);
		IO0CLR = 0x00000040; /* EN = 0, RS and RW unchanged(i.e. RS = 1, RW = 0) */
		delay_ms(5);

		i++;
	}
}


void sound_sensor(void)
{
    cmd(0xC0); //Second row of lcd
    PINSEL1 |= 0x00000000;	// PINSEL1 is used for configuring port 0 (p0.16 to p0.31
   	IO0DIR |= 0x00030000; //Configuring P0.16, p0.17 as output pins (led)
    
            if(SOUND)		 //When the sound detection module detects a signal, Print in the LCD
			{  
                    string("Detected");
					IOSET0 = 0x00010000;	  // To on bulb	(connected to p0.16)
					IOCLR0 = 0x00020000;	  // To off bulb (connected to p0.17)
            }
			else 
			{		string("Not Detected ");
					IOCLR0 = 0x00010000;	  // To off bulb (connected to p0.16)
					IOSET0 = 0x00020000;	  // To on bulb	 (connected to p0.17)
					
			}
            delay_ms(1000);
			
            cmd(0x01); //Clearing LCD display
        
}


int data;
int main (void)
 { 	 /* execution starts here */
                
  PINSEL0|= 0x00000005;           /* Enable RxD0(3:2) and TxD0(1:0                     */
  U0LCR = 0x83;                   /* (1000 0011) DLAB = 1, No break control,  no Parity, 1 Stop bit, 8 bits */
  U0DLL = 97;                     /* 9600 Baud Rate @ 15MHz VPB Clock         */
  U0LCR = 0x03;                   /* DLAB = 0                                 */
  
  lcd_int();
 
  string ("ENTER A");
  
  while(1)                           /* An embedded program does not stop */
      {
 	   	while(!(U0LSR & 0x01));	  // Wait until UART0 ready with received data
  		data=U0RBR;				 // read received data
		while(!(U0LSR & 0x20)); // Wait until UART0 ready to send character     
		U0THR =data;  // Send character
		cmd(0X01);  //Clearing LCD display
		
		cmd(0x80);	//Set cursor to 1st line
		if(data == 'A'|| data=='a')	
		{			 
	  		string("SOUND SENSOR ON");
			sound_sensor();
		
		}
		else 
		{
		   cmd(0x80);	//Set cursor to 1st line	
		   string("INVALID");
		   cmd(0xC0); //Set cursor to 2nd line
		   string("ENTER A ONLY");

		   IOCLR0 = 0x00010000;	  // To off bulb (connected to p0.16)
		   IOCLR0 = 0x00020000;	  // To off bulb (connected to p0.17)
		}

	}                               
}

