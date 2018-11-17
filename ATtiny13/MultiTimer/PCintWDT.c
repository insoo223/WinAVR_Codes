/**************************************************************
 Target MCU & clock speed: ATtiny13A @ 9.6Mhz internal
 Name    : PCintWDT.c
 Author  : Insoo Kim (insoo@hotmail.com)
 Date    : April 15, 2015

 Description: Interrupt based timer for 3 minutes.
 1) Pressing the button, PC interrupt will arise to wake up the MCU of sleep mode
 2)     and enable WDT of 8 second period
 3)     counting upto 3 min.
 4) Reaching 3 min, the buzzer will beep three times
 5) The system is operating mostly in power-down sleep mode,
 6)     so that 225 mAh capacity of CR2032 lithium cell battery
 7)     is supposed to last around two years,
 8)      if you use the timer 20 times a day.

 HEX size[Byte]: 460 out of 1024

 Ref:
 *****************************************************************/

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <avr/sleep.h>

#define BUTTON_MENU 0

#define DURATION 60 // for 2*8 perf b'd
#define shortBuzz 3  // buzzing 3 times

#define startPin 4  // for 2*8 perf b'd
#define buzzPin 3   // for 2*8 perf b'd
#define ledPin 0  // for 2*8 perf b'd

uint8_t menuCnt=0;
uint8_t prevLoop=0, curLoop=0, lapse=0;
uint8_t loopCnt=0;

//-------- FUNCTION PROTOTYPES
// Arduino Sketch C doesn't need to declare function prototypes
// But to conform with ANSI C, here i follow the standard C rules.
void initIO();
void blinkLED(uint8_t );
void buzz(uint8_t);

uint8_t sec8=0, minInt=0;
uint8_t intDrivenAlmEn=0, intDrivenAlmPeriod=0;

//-----------------------------------
ISR(WDT_vect)
{
    // WDT prescaler is set for 8 sec
    // 3min = 180sec = 22.5 * 8sec
    //PORTB ^= 1<<ledPin;
    sec8++;
    // 8sec * 7.5 = 60sec = 1min
    if (sec8 > 7)
    {
        minInt++;
        sec8=0;
    }

    if (intDrivenAlmEn == 1)
        if(minInt == intDrivenAlmPeriod)
        {
            buzz(shortBuzz);
            blinkLED(3);
            intDrivenAlmEn=0;
            // Disable watchdog timer interrupts
            WDTCR &= ~_BV(WDTIE);
        }
}//ISR(WDT_vect)

uint8_t cntPCINT0 = 0;
//-----------------------------------
ISR(PCINT0_vect)
{
    cntPCINT0++;
    //WDT_overflow_count = 0;
    sec8 = 0;
    minInt = 0;
    if (cntPCINT0 % 2 == 0)
    {
        // Enable watchdog timer interrupts
        WDTCR |= _BV(WDTIE);

        cntPCINT0 = 0;
        intDrivenAlmEn = 1;
        intDrivenAlmPeriod = 3; //countButton(BUTTON_MENU);
        blinkLED(1);
    }
}//ISR(PCINT0_vect)

//-----------------------------------
void initIO()
{
  DDRB &= ~_BV(startPin); //input
  DDRB |= _BV(buzzPin); //output
  DDRB |= _BV(ledPin); //output
}//initIO

//-----------------------------------
int main()
{
    initIO();

    //PC interrupt setting
    // enable PC(Pin Change) interrupt
    GIMSK |= _BV(PCIE);  //Enable PC interrupt
    // Enable pin change interrupt for PB3
    //PCMSK |= _BV(PCINT3);
    PCMSK |= _BV(startPin);

    //WDT interrupt setting
    // temporarily prescale timer to n sec so we can measure current
    //DS: ch8.5.2 table8.2
    WDTCR |= (1<<WDP3) | (1<<WDP0); // 8s
    //WDTCR |= (1<<WDP3); // 4s

    // Enable watchdog timer interrupts
    //WDTCR |= _BV(WDTIE);


    sei();

    // Use the Power Down sleep mode
    set_sleep_mode(SLEEP_MODE_PWR_DOWN);
    //set_sleep_mode(SLEEP_MODE_IDLE);
    while(1)
    {
      // go to sleep and wait for interrupt...
      sleep_mode();
    }//while(1)

    return 0;
}//main

//-----------------------------------
void blinkLED(uint8_t num)
{
  uint8_t i;
  for (i=0; i<(2*num); i++)
  {
    PORTB ^= _BV(ledPin);
    _delay_ms(DURATION/2);
  }
  PORTB &= ~_BV(ledPin);
}//blinkLED

//-----------------------------------
void buzz(uint8_t times)
{
  const uint8_t buzzInterval = DURATION/2;
  uint8_t i;
  for (i=0; i<2*times; i++)
  {
    PORTB ^= _BV(buzzPin);
    _delay_ms(buzzInterval);
  }
  PORTB &= ~_BV(buzzPin);
}//buzz

