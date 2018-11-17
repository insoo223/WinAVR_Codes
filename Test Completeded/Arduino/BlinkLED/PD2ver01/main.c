
/**************************************
* Target: Arduino Uno R3 16Mhz External Crystal
* Code Name: BlinkLED_PD2ver01.c
* Author: Insoo Kim (insoo@hotmail.com)
* Created:	 2017.5.22 (Mon)
* Updated: 2017.5.22
* Hex Size[Byte]: 166 out of 32K
* Desc: WinAVR as a professional Arduino development tool.
	Blink LED at PD2 every second.
	
* Revision history: 
  2017.5.22 The initial trial & success using Arduino Uno R3 board (Made in China)
* Refs: 
  **************************************/

#include <avr/io.h>
#include <util/delay.h>

#define delayPeriod 1000 

#define ledPin PD2

void initIO(void);

int main(void)
{
    initIO();

    while(1)
    {
        PORTD ^= 1 << ledPin;    /* toggle the LED */
        _delay_ms(delayPeriod);  /* max is 262.14 ms / F_CPU in MHz */
    }

    return 0; // never reach here
}//main

//-----------------------------------
void initIO()
{
  DDRD |= _BV(ledPin); //output
}//initIO
