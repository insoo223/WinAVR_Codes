#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <avr/sleep.h>

uint8_t alarmEnable = 0;
uint8_t start = 0;
uint8_t DONE = 0;
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

volatile unsigned char timer_overflow_count = 0;
uint8_t secInt=0, minInt=0;
uint8_t intDrivenAlmEn=0, intDrivenAlmPeriod=3;

//-----------------------------------
ISR(TIM0_OVF_vect)
{
    if (++timer_overflow_count > 70)
    {   // with 1024/256/64 prescaler, a timer overflow occurs 4.6/18/73 times per second accordingly.
        // Toggle Port B pin 4 output state
        PORTB ^= 1<<ledPin;
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

//-----------------------------------
ISR(PCINT0_vect)
{
    // Toggle PBn output state
    //PORTB ^= 1<<PB0;
    //buzz(3);
    /*
    DONE = 0;
    pollingButton();
    */
    timer_overflow_count = 0;
    secInt = 0;
    minInt = 0;
    intDrivenAlmEn=1;

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
    // Enable pin change interrupt for PB3
    //PCMSK |= _BV(PCINT3);
    PCMSK |= _BV(startPin);
    // enable PC(Pin Change) interrupt
    GIMSK |= _BV(PCIE);  //Enable PC interrupt

    //TMR0 interrupt setting
    
	// prescale timer to 1/64th the clock rate
    //TCCR0B |= (1<<CS01) | (1<<CS00);

	// prescale timer to 1/1024th the clock rate
	TCCR0B |= (1<<CS02) | (1<<CS00);

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
  for (i=0; i<times; i++)
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
    DONE = 1;
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
    while(!DONE)
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
        _delay_ms(DURATION/4);
        }
    }//while(!DONE)
}//pollingButton
