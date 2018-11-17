/********************************************
 Target MCU & clock speed: ATtiny13A @ 9.6Mhz internal
 Name    : PCintWDT_multi_visualLapTime.c
 Author  : Insoo Kim (insoo@hotmail.com)
 Date    : April 15, 2015

 Description: Polling based timer for 3, 5, or 15 minutes.
 1) Pressing the button to select alarm period
 2)     counting upto 3, 5, or 15 min.
 3) Reaching the user selected min, the buzzer will beep three times
 4) The system is operating mostly in power-down sleep mode,
 5)     so that 225 mAh capacity of CR2032 lithium cell battery
 6)     is supposed to last around two years,
 7)      if you use the timer 20 times a day.

 HEX size[Byte]: 696 out of 1024

 Ref:
********************************************/
#include <avr/io.h>
#include <util/delay.h>

uint8_t alarmEnable = 0;
uint8_t start = 0;
uint8_t BLINK_NOTICED = 0;

#define BUTTON_MENU 0
#define BUTTON_TEMP_ALARM_NUM 1

#define DURATION 60 // for 0.5 sec
#define menuSelCompleteINTERVAL  4
#define shortBuzz 3  // buzzing 3 times
#define longBuzz 10  // buzzing 10 times

#define startPin 4  // for 2*8 perf b'd
#define buzzPin 3   // for 2*8 perf b'd
#define ledPin 0  // for 2*8 perf b'd

uint8_t clockCnt;
uint8_t secCnt;
uint8_t minCnt;
uint8_t alarm[3] = {3, 5, 15};

uint8_t menuCnt=0, tempAlarmCnt=0;
uint8_t prevLoop=0, curLoop=0, lapse=0;
uint8_t loopCnt=0;

//-------- FUNCTION PROTOTYPES
// Arduino Sketch C doesn't need to declare function prototypes
// But to conform with ANSI C, here i follow the standard C rules.
void initIO();
void startClock(uint8_t );
void countButton(uint8_t);
void blinkLED(uint8_t );
void buzz(uint8_t);
void chkAlarm(uint8_t );
void chkInterval();
void pollingButton();

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

    while(1)
    {
      if (debugMode)
      {
        blinkLED(3);
        //chkInterval();
      }
      else
      {
        pollingButton();
      }//else (debugMode == 0)
    }//while(1)

    return 0;
}//main

//-----------------------------------
void startClock(uint8_t alarmMin)
{
  start = 1;

  clockCnt=0;
  secCnt=0;
  minCnt=0;

  while (start)
  {
    clockCnt++;
    if (clockCnt % 2 == 0)
      secCnt++;

    //check minute
    if (secCnt == 60)
    {
      minCnt++;
      clockCnt = 0;
      blinkLED(menuCnt);
      //blinkLED(menuCnt) routine consumes around 1 sec, so we need to complement the loss
      secCnt = 1;
    }//if (secCnt == 60)

    //========== check Alarm enable status
    if (alarmEnable == 1)
    {
      //digitalWrite(ledPin, HIGH);
      chkAlarm(alarmMin);
    }
    else
    {
      //digitalWrite(ledPin, 0);
      start = 0;
    }

    _delay_ms(DURATION);        // _delay_ms in between reads for stability
  }//while (start)
}//startClock

//-----------------------------------
void countButton(uint8_t cate)
{
  uint8_t val;

  //check if startPin pressed to 0
  val = PINB & _BV(startPin);
  if (val == 0)
  {
    _delay_ms(DURATION); // for debounce
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
void chkAlarm(uint8_t num)
{
  //if the current minute has reached to alarm set
  if(num == minCnt)
  {
    //buzzing
    buzz(shortBuzz);
    //disable alarm setting
    alarmEnable = 0;
    //turn off the set alarm LED
    //digitalWrite(ledPin, 0);
    PORTB &= ~_BV(ledPin);
    //reset menu selection count
    menuCnt=0;
    //prevMS = millis();
    prevLoop = loopCnt;
    start = 0;
    BLINK_NOTICED = 0;
  }//if(num == minCnt)
}//chkAlarm

//-----------------------------------
void chkInterval()
{
    PORTB ^= _BV(ledPin);
    _delay_ms(DURATION);
}//chkInterval

//-----------------------------------
void pollingButton()
{
    loopCnt++;
    if (menuCnt <= 3)
        countButton(BUTTON_MENU);
    else if (menuCnt == 4)
        countButton(BUTTON_TEMP_ALARM_NUM);

    curLoop = loopCnt;
    lapse = curLoop - prevLoop;

    if (lapse > menuSelCompleteINTERVAL)
    {
        if (menuCnt != 0)
        {
          loopCnt = 0;
          if (!BLINK_NOTICED)
          {
                blinkLED(menuCnt);
          }//if (!BLINK_NOTICED)

          switch (menuCnt)
          {
            case 1:
              alarmEnable = 1;
              startClock(alarm[0]);
              break;
            case 2:
              alarmEnable = 1;
              startClock(alarm[1]);
              break;
            case 3:
              alarmEnable = 1;
              startClock(alarm[2]);
              break;
          }//switch (menuCnt)
        }//if (menuCnt != 0)

        //when menuCnt == 4, buttonCount function counts "tempAlarmCnt"
        if (tempAlarmCnt != 0)
        {
          loopCnt = 0;
          if (!BLINK_NOTICED)
          {
            blinkLED(menuCnt);
          }//if (!BLINK_NOTICED)
          //DONE_incUnit = 1;
          blinkLED(tempAlarmCnt);
          alarm[0] = tempAlarmCnt;
          tempAlarmCnt = 0;
          menuCnt = 1;
        }//if (tempAlarmCnt != 0)
    }//if (lapse > menuSelCompleteINTERVAL)

    if (!start)
    {
        //_delay_ms should be short enough
        //to catch button press by user
        _delay_ms(DURATION/7);
    }
}//pollingButton
