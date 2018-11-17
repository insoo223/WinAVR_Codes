/********************************************
 Target MCU & clock speed: ATtiny13A @ 9.6Mhz internal
 Name    : PCintTMR0.c
 Author  : Insoo Kim (insoo@hotmail.com)
 Date    : April 15, 2015

 Description: TMR0 Interrupt based timer for 3minutes.
 1) Pressing the button, PC interrupt will arise to wake up the MCU of sleep mode
 2)     and enable TMR0 of some prescaled period
 3)     counting upto 3 min.
 4) Reaching the user selected min, the buzzer will beep three times
 5) The system is operating mostly in power-down sleep mode,
 6)     so that 225 mAh capacity of CR2032 lithium cell battery
 7)     is supposed to last around two years,
 8)      if you use the timer 20 times a day.

 HEX size[Byte]: 860 out of 1024

 Ref:
********************************************/
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <avr/sleep.h>

uint8_t alarmEnable = 0;
uint8_t start = 0;
uint8_t BLINK_NOTICED = 0;

#define BUTTON_MENU 0
#define BUTTON_TEMP_ALARM_NUM 1

#define DURATION 60 // for 2*8 perf b'd
#define menuSelCompleteINTERVAL  4
#define shortBuzz 3  // buzzing 3 times
#define longBuzz 10  // buzzing 10 times

#define startPin 4  // for 2*8 perf b'd
#define buzzPin 3   // for 2*8 perf b'd
#define ledPin 0  // for 2*8 perf b'd

uint8_t clockCnt=0;
uint8_t secCnt=0;
uint8_t minCnt=0;
uint8_t alarm[3] = {3, 5, 15};

uint8_t menuCnt=0, tempAlarmCnt=0;
uint8_t prevLoop=0, curLoop=0, lapse=0;
uint8_t loopCnt=0;

//-------- FUNCTION PROTOTYPES
// Arduino Sketch C doesn't need to declare function prototypes
// But to conform with ANSI C, here i follow the standard C rules.
void initIO();
uint8_t countButton(uint8_t);
void blinkLED(uint8_t );
void buzz(uint8_t);

volatile unsigned char timer_overflow_count = 0;
uint8_t secInt=0, minInt=0;
uint8_t intDrivenAlmEn=0, intDrivenAlmPeriod=0;

//-----------------------------------
ISR(TIM0_OVF_vect)
{
    if (++timer_overflow_count > 70)
    {   // with 1024/256/64 prescaler, a timer overflow occurs 4.6/18/73 times per second accordingly.
        // Toggle Port B pin 4 output state
        //PORTB ^= 1<<ledPin;
        timer_overflow_count = 0;
        secInt++;
        if (secInt == 60)
        {
            minInt++;
            secInt=0;
        }
        if (intDrivenAlmEn == 1)
            if(minInt == intDrivenAlmPeriod)
            {
                buzz(shortBuzz);
                intDrivenAlmEn=0;
            }
    }
}//ISR(TIM0_OVF_vect)

uint8_t cntPCINT0 = 0;
//-----------------------------------
ISR(PCINT0_vect)
{
    // Toggle PBn output state
    //PORTB ^= 1<<PB0;
    //buzz(3);
    cntPCINT0++;
    timer_overflow_count = 0;
    secInt = 0;
    minInt = 0;
    if (cntPCINT0 % 2 == 0)
    {
        cntPCINT0 = 0;
        blinkLED(2);
        intDrivenAlmEn = 1;
        intDrivenAlmPeriod = 3; //countButton(BUTTON_MENU);
        blinkLED(intDrivenAlmPeriod);
        //sleep_mode();
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
    uint8_t debugMode = 0;
    initIO();

    //PC interrupt setting
    // enable PC(Pin Change) interrupt
    GIMSK |= _BV(PCIE);  //Enable PC interrupt
    // Enable pin change interrupt for PB3
    //PCMSK |= _BV(PCINT3);
    PCMSK |= _BV(startPin);

    //TMR0 interrupt setting
    // prescale timer to 1/64th the clock rate
    TCCR0B |= (1<<CS01) | (1<<CS00);
    // enable timer overflow interrupt
    TIMSK0 |=1<<TOIE0;

    sei();

    // Use the Power Down sleep mode
    //set_sleep_mode(SLEEP_MODE_PWR_DOWN);
    set_sleep_mode(SLEEP_MODE_IDLE);
    while(1)
    {
      if (debugMode)
      {
        blinkLED(3);
        //chkInterval();
      }
      else
      {
          // go to sleep and wait for interrupt...
          sleep_mode();
      }//else (debugMode == 0)
    }//while(1)

    return 0;
}//main

//-----------------------------------
uint8_t countButton(uint8_t cate)
{
  uint8_t val, DONE=0;

  loopCnt = 0;
  prevLoop = 0;
  while(!DONE)
  {
      loopCnt++;
      curLoop = loopCnt;
      lapse = curLoop - prevLoop;

      //check if startPin pressed to 0
      val = PINB & _BV(startPin);
      if (val == 0)
      {
        _delay_ms(DURATION/2); // for debounce
        switch (cate)
        {
          case BUTTON_MENU:
            menuCnt++;
            break;
          case BUTTON_TEMP_ALARM_NUM:
            tempAlarmCnt++;
            break;
        } //switch (cate)

        prevLoop = loopCnt;
      }//if(val == 0)

      if (lapse > menuSelCompleteINTERVAL)
      {
          if(menuCnt != 0)
          {
              loopCnt = 0;
              if (!BLINK_NOTICED)
                blinkLED(menuCnt);
              DONE = 1;
          }
      }
  }//while(!DONE)

  return alarm[menuCnt];

}//countButton

//-----------------------------------
void blinkLED(uint8_t num)
{
  uint8_t i;
  for (i=0; i<(2*num); i++)
  {
    //digitalWrite(ledPin, HIGH);
    PORTB ^= _BV(ledPin);
    _delay_ms(DURATION/2);
  }
  BLINK_NOTICED = 1;
  PORTB &= ~_BV(ledPin);
  _delay_ms(DURATION*4);
}//blinkLED

//-----------------------------------
void buzz(uint8_t times)
{
  const uint8_t buzzInterval = DURATION/2;
  uint8_t i;
  for (i=0; i<2*times; i++)
  {
    //digitalWrite(buzzPin, HIGH);
    PORTB ^= _BV(buzzPin);
    _delay_ms(buzzInterval);
  }
  PORTB &= ~_BV(buzzPin);
}//buzz

//-----------------------------------

