/**************************************
* Target: Arduino Uno R3 16Mhz External Crystal
* Code Name: BlinkLED_PD2ver00.c
* Author: Insoo Kim (insoo@hotmail.com)
* Created:	 2017.5.20 (Sat)
* Updated: 2017.5.22
* Hex Size[Byte]: 164
* Desc: WinAVR as a professional Arduino development tool.
* Revision history: 
  2017.5.22 Add tail name as ver00, for ver01 has been tested using _BV()macro
  2017.5.20 The initial trial & success using Arduino Uno R3 board (Made in China)
* Refs: 
  **************************************/

#include <avr/io.h>
#include <util/delay.h>

int main(void)
{
    //if you set output as input, then LED should be very dim.
	DDRD = 0b00000100;           /* make the LED pin an output */
	

    for(;;)
    {
        PORTD ^= 1 << PD2;    /* toggle the LED */
        _delay_ms(1000);  /* max is 262.14 ms / F_CPU in MHz */
    }

    return 0; // never reach here
}
