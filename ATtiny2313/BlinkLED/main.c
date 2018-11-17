/********************************************
 Target MCU & clock speed: ATtiny2313 @ 8Mhz internal
 Name    : BlinkLED.c
 Author  : Insoo Kim (insoo@hotmail.com)
 Date    : April 30, 2015

 Description: Blink LED at PB0 every second.

 HEX size[Byte]: 90 out of 2K

 Ref:
********************************************/

#include <avr/io.h>
#include <util/delay.h>

#define halfSec 500 // 0.5 second checked by oscilloscope

#define ledPin PB0

void initIO();

int main(void)
{
    initIO();

    while(1)
    {
        PORTB ^= 1 << ledPin;    /* toggle the LED */
        _delay_ms(halfSec*2);  /* max is 262.14 ms / F_CPU in MHz */
    }

    return 0; // never reach here
}//main

//-----------------------------------
void initIO()
{
  DDRB |= _BV(ledPin); //output
}//initIO
