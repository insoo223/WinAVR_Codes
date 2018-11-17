/********************************************
 Target MCU & clock speed: ATtiny13A @ 9.6Mhz internal
 Name    : PCintWDT_multi.c
 Author  : Insoo Kim (insoo@hotmail.com)
 Date    : April 16, 2015

 Description: Interrupt based timer for 3, 5, 15, or 60 minutes.
 1) Pressing the button, PC interrupt will arise to wake up the MCU of sleep mode
 2)     and enable WDT of 8 second period
 3)     counting upto 3, 5, 15, or 60 min.
 4) Reaching the user selected min, the buzzer will beep three times
 5) The system is operating mostly in power-down sleep mode,
 6)     so that 225 mAh capacity of CR2032 lithium cell battery
 7)     is supposed to last around two years,
 8)      if you use the timer 20 times a day.

 HEX size[Byte]: 648 out of 1024

 Ref:
********************************************/

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <avr/sleep.h>

#define BUTTON_MENU 0
#define menuSelectInterval 8
#define halfSec 60 // 0.5 second checked by oscilloscope
#define shortBuzz 3  // buzzing n times

#define startPin 4  // for 2*8 perf b'd
#define buzzPin 3   // for 2*8 perf b'd
#define ledPin 0  // for 2*8 perf b'd

uint8_t alarm[4] = {3,5,15,60};
uint8_t menuCnt=0;
uint8_t prevLoop=0, curLoop=0, lapse=0;
uint8_t loopCnt=0;

uint8_t sec8=0, minInt=0;
uint8_t intDrivenAlmEn=0, intDrivenAlmPeriod=0;

//-------- FUNCTION PROTOTYPES
// Arduino Sketch C doesn't need to declare function prototypes
// But to conform with ANSI C, here i follow the standard C rules.
void initIO();
void initINT();
void blinkLED(uint8_t, uint8_t);
//void blinkLONGLED(uint8_t);
void buzz(uint8_t);
void countButton(uint8_t);

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
        if (intDrivenAlmEn == 1)
        {
            /*
            //visual cue of time lapse by minute
            //each min flashing if alarm min is equal or less than 5
            if (minInt < 5)
                blinkLED(minInt, 1);
            //5 to 20 min alarm, 5min-flashing is on
            else if (minInt <= 20)
            {
                blinkLED(minInt / 5, 2);
            }
            //longer than 20 min alarm, 10min-flashing is on
            else
            {
                blinkLED(minInt / 10, 3);
            }
            */
        }
    }
    //if alarm is enabled
    if (intDrivenAlmEn == 1)
        //and reached preset alarm period
        if(minInt == intDrivenAlmPeriod)
        {
            buzz(shortBuzz);
            intDrivenAlmEn=0;
            //Disable watchdog timer interrupts
            // and stop counting to save energy
            WDTCR &= ~_BV(WDTIE);
        }
}//ISR(WDT_vect)

uint8_t cntPCINT0 = 0;
//-----------------------------------
ISR(PCINT0_vect)
{
    cntPCINT0++;
    sec8 = 0;
    minInt = 0;
    //PC interrupt is raised at rising & falling edge
    //  i want to do call WDT at rising or falling edge, not for both.
    if (cntPCINT0 % 2 == 0)
    {
        cntPCINT0 = 0;
        countButton(BUTTON_MENU);
    }
}//ISR(PCINT0_vect)

//-----------------------------------
int main()
{
    initIO();
    initINT();

    // Use the Power Down sleep mode
    set_sleep_mode(SLEEP_MODE_PWR_DOWN);

    while(1)
    {
      // go to sleep and wait for interrupt...
      sleep_mode();
    }//while(1)

    return 0;
}//main

//-----------------------------------
void initIO()
{
  DDRB &= ~_BV(startPin); //input
  DDRB |= _BV(buzzPin); //output
  DDRB |= _BV(ledPin); //output
}//initIO

//-----------------------------------
void initINT()
{
    //**** PC(Pin Change) interrupt setting
    // enable PC INT
    GIMSK |= _BV(PCIE);  //Enable PC interrupt
    // Enable pin change interrupt for PB3
    //PCMSK |= _BV(PCINT3);
    PCMSK |= _BV(startPin);

    //**** WDT interrupt setting
    //set prescale timer
    //DS: ch8.5.2 table8.2
    WDTCR |= (1<<WDP3) | (1<<WDP0); // 8s
    //WDTCR |= (1<<WDP3); // 4s

    //enalbe global interrupt
    sei();
}//initINT

//-----------------------------------
void blinkLED(uint8_t num, uint8_t interval)
{
  uint8_t i;

  //blinking means on & off, so that (2*num)
  for (i=0; i<(2*num); i++)
  {
    PORTB ^= _BV(ledPin);
    _delay_ms(halfSec*interval);
  }

  //turn off the LED after blinking
  PORTB &= ~_BV(ledPin);
}//blinkLED

//-----------------------------------
/*
void blinkLONGLED(uint8_t num)
{
  uint8_t i;

  //blinking means on & off, so that (2*num)
  for (i=0; i<(2*num); i++)
  {
    PORTB ^= _BV(ledPin);
    _delay_ms(halfSec);
  }

  //turn off the LED after blinking
  PORTB &= ~_BV(ledPin);
}//blinkLONGLED
*/
//-----------------------------------
void buzz(uint8_t times)
{
  const uint8_t buzzInterval = halfSec/2;
  uint8_t i;

  //beep-beep is sounding on & off, so that (2*num)
  for (i=0; i<(2*times); i++)
  {
    PORTB ^= _BV(buzzPin);
    _delay_ms(buzzInterval);
  }

  //silence after beeping
  PORTB &= ~_BV(buzzPin);
}//buzz

//-----------------------------------
void countButton(uint8_t cate)
{
    uint8_t val;
    uint8_t DONE=0;

    //Disable PC(Pin Change) interrupt
    //  to use the button for user menu selection
    GIMSK &= ~_BV(PCIE);

    loopCnt=0;
    prevLoop=0;
    //visual cue to show being ready to get user input of menuCnt
    blinkLED(1, 1);
    //Get menuCnt by counting the button press
    //If pressing the button within 1 second of interval between each press,
    //  it will be accumulated as "menuCnt".
    //If the interval is over 1 sec, which is menuSelectInterval,
    //  then DONE is set to 1.
    while (!DONE)
    {
        loopCnt++;
        curLoop = loopCnt;
        lapse = curLoop - prevLoop;
        if (lapse > menuSelectInterval)
        {
            if (menuCnt != 0)
                DONE = 1;
        }
        if ((lapse > menuSelectInterval*3) && (menuCnt ==0))
            DONE = 2;
      //check if startPin pressed to 0
      val = PINB & _BV(startPin);
      if (val == 0)
      {
        _delay_ms(halfSec/2); // for debounce
        switch (cate)
        {
          case BUTTON_MENU:
            menuCnt++;
            break;
        } //switch (cate)

        prevLoop = loopCnt;
      }//if(val == 0)

        // should be fast enough to catch button press frequency
        // menuSelectInterval is a times of _delay_ms(halfSec/4);
      _delay_ms(halfSec/4);
  }//while(!DONE)

    //Enable PC(Pin Change) interrupt
    //  so that, set alarm interrupt by PCINT can be done any time
    //  in then middle of counting of already-set-alarm .
    GIMSK |= _BV(PCIE);

    //menuCnt has been set within 3sec of a PCINT occurence
    //  then, play WDT count for a corresponding alarm period.
    if (DONE == 1)
    {
        //interrupt driven alarm is enabled
        intDrivenAlmEn = 1;

        //intDrivenAlmPeriod is to set buzz alarm in ISR(WDT_vect)
        intDrivenAlmPeriod = alarm[menuCnt-1];

        //visual cue to notifiy user selected menuCnt
        blinkLED(menuCnt, 1);

        //reset menuCnt
        menuCnt=0;

        //Enable watchdog timer interrupts
        // and begin counting for alarm
        WDTCR |= _BV(WDTIE);
    }//if (DONE == 1)

}//countButton
