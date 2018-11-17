/********************************************
 Target MCU & clock speed: ATtiny13A @ 9.6Mhz internal
 Name    : PCintWDT_clockAlarm.c
 Author  : Insoo Kim (insoo@hotmail.com)
 Created : April 18, 2015
 Updated : April 25, 2015

 Description: Interrupt based moderately accurate clock & timer
    for 3, 5, 15, 30, or 60 minutes.
 1) Pressing the button,once, PC interrupt will arise to wake up the MCU of sleep mode
      and shows the current time:
      hour flash: long-flash 5hours, short-flash 1hour
      minute flash: short-flash the nearest future 10minutes
 2) Pressing the button,2 to 6times, PC interrupt will arise to wake up the MCU of sleep mode
      andalarms of 3, 5, 15, 30, or 60 min.
 3) Reaching the user selected min, the buzzer will beep three times.
 4) The system is operating mostly in power-down sleep mode,
      so that 225 mAh capacity of CR2032 lithium cell battery
      is supposed to last around two years,
       if you use the timer 20 times a day.

 HEX size[Byte]: 994 out of 1024

 Ref:
********************************************/

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <avr/sleep.h>

#define menuSelectInterval 8
#define halfSec 60 // 0.5 second checked by oscilloscope
#define shortBuzz 3  // buzzing n times

#define startPin 4  // tactile switch
#define buzzPin 3   // 5V buzzer
#define ledPin 0  // 3mm LED yellow
//#define debugPin 1 //signal dispatch for debugging

uint8_t alarm[5] = {3,8,15,30,60};
uint8_t menuCnt=0;
uint8_t prevLoop=0, curLoop=0, lapse=0;
uint8_t loopCnt=0;

uint8_t sec8=0, alSec8=0, minInt=0;
uint8_t intDrivenAlmEn=0, intDrivenAlmPeriod=0;
uint8_t timeH=18, timeM=9; //2015.6.25
uint8_t timeHaccumulated = 0;


//-------- FUNCTION PROTOTYPES
// Arduino Sketch C doesn't need to declare function prototype
// But to conform with ANSI C, here i follow the standard C rules.
void initIO();
void initINT();
void blinkLED(uint8_t );
void blinkLONGLED(uint8_t);
void blinkLONG10LED(uint8_t);
void buzz(uint8_t);
void countButton();
void showCurTime();

//-----------------------------------
ISR(WDT_vect)
{
    // WDT prescaler is set for 8 sec
    // 3min = 180sec = 22.5 * 8sec
    //PORTB ^= 1<<debugPin;
    sec8++;

    // 8sec * 7.5 = 60sec = 1min
    if (sec8 > 7)
    {
        timeM++;
        sec8=0;
        //8*8=64sec is counted as 1 min, so that 4sec * 15times = 1min
        // is delayed every 15min, so need to add 1min every 15min
        //Experiments showed that WDT is not exactly 8 sec,
        //  so forget it the following adjustment.
        /*
        if (timeM % 20 == 0)
            timeM++;
            */

        /*
        //visual cue of alarm time lapse
        if (intDrivenAlmEn == 1)
        {
            //visual cue of time lapse by minute
            //each min flashing if alarm min is equal or less than 5
            if (minInt < 5)
                blinkLED(minInt);
            //5 - 19 min alarm, 5min-flashing is long
            //  and min is short on every 5 min.
            else if (minInt < 20)
            {
                if (minInt % 5 == 0)
                {
                    blinkLONGLED(minInt / 5);
                    blinkLED(minInt % 5);
                }
            }//else if (minInt < 20)
            //20min+ alarm, 10min-flashing only on every 10 min.
            else
            {
                if (minInt % 10 == 0)
                    blinkLONG10LED(minInt / 10);
            }//else (minInt >= 20)
        }//if (intDrivenAlmEn == 1)
        */
    }//if (sec8 > 7)

    if (timeM >= 60)
    {
        timeH++;
        timeHaccumulated++;
        //The 1st experiment showed that 1 min delay every two hours.
        //The 2nd one turned out 10 min delay after 15 hours.
        //  so 1/2 + 2/3 = 7/6 min for every hour,
        //  or 7 min for every 6 hours to be added for adjustment.
        //So, add 1 min every hour,and add 1 more min every 6th hour
        if (timeH % 3 == 0)
        {
            timeM = timeM+2;
            //timeM %= 60;
        }
        //add 5 more minutes every 7 hours
        if (timeHaccumulated == 7)
        {
            timeM = timeM+5;
            //timeM %= 60;
            timeHaccumulated = 0;
        }
        timeM %= 60;

    }//if (timeM >= 60)

    if (timeH == 24)
    {
        timeH = 0;
    }//if (timeH == 24)

    //if alarm is enabled
    if (intDrivenAlmEn == 1)
    {
        alSec8++;
        if (alSec8 > 7)
        {
            alSec8=0;
            minInt++;
        }
        //and reached preset alarm period
        if(minInt == intDrivenAlmPeriod)
        {
            cli();
            buzz(shortBuzz);
            blinkLED(3);
            sei();
            intDrivenAlmEn = 0;
            minInt = 0;
            //Disable watchdog timer interrupts
            // and stop counting to save energy
            //WDTCR &= ~_BV(WDTIE);
        }
    }//if (intDrivenAlmEn == 1)
}//ISR(WDT_vect)

uint8_t cntPCINT0 = 0;
//-----------------------------------
ISR(PCINT0_vect)
{
    cntPCINT0++;
    //sec8alarm = 0;
    //minInt = 0;
    //PC interrupt is raised at rising & falling edge
    //  i want to do call WDT at rising or falling edge, not for both.
    if (cntPCINT0 % 2 == 0)
    {
        cntPCINT0 = 0;
        cli();
        countButton();
        sei();
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
  //DDRB |= _BV(debugPin); //output

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

    //Enable watchdog timer interrupts
    // and begin counting for alarm
    WDTCR |= _BV(WDTIE);

    //enalbe global interrupt
    sei();
}//initINT

//-----------------------------------
void blinkLED(uint8_t num)
{
  uint8_t i;

  //blinking means on & off, so that (2*num)
  //PORTB = 0;
  for (i=0; i<(2*num); i++)
  {
    PORTB ^= _BV(ledPin);
    _delay_ms(halfSec/2);
  }

  //turn off the LED after blinking
  PORTB &= ~_BV(ledPin);
}//blinkLED

//-----------------------------------
void blinkLONGLED(uint8_t num)
{
  uint8_t i;

  //blinking means on & off, so that (2*num)
  //PORTB = 0;
  for (i=0; i<(2*num); i++)
  {
    PORTB ^= _BV(ledPin);
    _delay_ms(halfSec);
  }

  //turn off the LED after blinking
  PORTB &= ~_BV(ledPin);
}//blinkLONGLED

//-----------------------------------
void blinkLONG10LED(uint8_t num)
{
  uint8_t i;

  //blinking means on & off, so that (2*num)
  //PORTB = 0;
  for (i=0; i<(2*num); i++)
  {
    PORTB ^= _BV(ledPin);
    _delay_ms(halfSec*2);
  }

  //turn off the LED after blinking
  PORTB &= ~_BV(ledPin);
}//blinkLONG10LED

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
void countButton()
{
    uint8_t val;
    const uint8_t loopInterval = halfSec/4;
    uint8_t DONE=0;

    //Disable PC(Pin Change) interrupt
    //  to use the button for user menu selection
    //GIMSK &= ~_BV(PCIE);

    loopCnt=0;
    prevLoop=0;
    //visual cue to show being ready to get user input of menuCnt
    blinkLED(1);

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
            else
                //wait another 1 sec to get user's menuCnt
                ;
        }
        //if user has not chosen menuCnt > 0, and time lapse over 3sec,
        //forget about it, and call DONE as 2, and eventually go to sleep.
        if ((lapse > menuSelectInterval*3) && (menuCnt ==0))
            DONE = 2;

      //check if startPin pressed to 0
      //AVR equivalent to Arduino digitalRead(startPin)
      val = PINB & _BV(startPin);
      if (val == 0)
      {
        // for debounce
        _delay_ms(halfSec/2);
        menuCnt++;

        //Pressing the button, lap time calculation should be reset
        //to give 1 sec of time to choose menuCnt
        prevLoop = loopCnt;
      }//if(val == 0)

        // should be fast enough to catch button press frequency
        // menuSelectInterval is a multiple of times of _delay_ms(halfSec/4);
      _delay_ms(loopInterval);
  }//while(!DONE)

    //Enable PC(Pin Change) interrupt
    //  so that, set alarm interrupt by PCINT can be done any time
    //  in then middle of counting of already-set-alarm .
    //GIMSK |= _BV(PCIE);

    //menuCnt has been set within 3sec of a PCINT occurence
    //  then, play WDT count for a corresponding alarm period.
    if (DONE == 1)
    {
        //visual cue to notifiy user selected menuCnt
        blinkLED(menuCnt);
        _delay_ms(halfSec);

        if (menuCnt == 1)
        {
            showCurTime();
        }
        else
        {
            //interrupt driven alarm is enabled
            intDrivenAlmEn = 1;

            //intDrivenAlmPeriod is to set buzz alarm in ISR(WDT_vect)
            if (menuCnt<7)
                intDrivenAlmPeriod = alarm[menuCnt-2];
            else
                intDrivenAlmPeriod = alarm[0];
            minInt = 0;
        }


        //reset menuCnt
        menuCnt=0;

        //Enable watchdog timer interrupts
        // and begin counting for alarm
        //WDTCR |= _BV(WDTIE);
    }//if (DONE == 1)

}//countButton

void showCurTime()
{
    uint8_t h5, h, m10;

    h5 = (timeH) / 5;
    h = (timeH) % 5;

    m10 = timeM / 10;

    blinkLONGLED(h5);
    blinkLED(h);

    _delay_ms(halfSec);
    //show the nearest 10 min,
    //i.e. if current min is 24 then blink 3 times
    // 52 then 6 times,
    // which means the flashed hour+1 is the hour coming within 10 min
    blinkLED(m10+1);
}
