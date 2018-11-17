/**************************************************************
 Target MCU & internal clock speed: ATmega328p @ 1Mhz
 Name    : DHT11_ATmega328p.c
 Author  : Insoo Kim (insoo@hotmail.com)
 Date    : May 13, 2015

 Description: Read temperature & humidity from DHT11 sensor,
 and display T&H on two digits 7 segment LEDs for around 1 second each, continuously.
 HEX size[Byte]: 728 out of 1024

 Ref:
 ** Written for and tested with a custom board with ATtiny13A run on 9.6Mhz int osc
 ** Functions regarding DHT11, i.e."dht_getdata", are refered from the following link of David Gironi.
 ** But, specific time delays have been measured using Hantek USB digital oscilloscope (6022BE) and modified to fit into my custom ATtiny13A board.
 ** http://davidegironi.blogspot.kr/2013/02/reading-temperature-and-humidity-on-avr.html#.VSRiHSlCNSV
 *****************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "../dht11/DHT11_ATmega328p.h"


//#define DHT11_PIN PORTB1
#define debug_PIN PORTB5

//Define functions
//======================
void ioinit(void);
int8_t dht_getdata(int8_t *, int8_t *);


void ioinit (void);
void getDHT11(void);

//----------------------------------------------------------
void getDHT11(void)
{

    int8_t temperature = 0;
    int8_t humidity = 0;

    ioinit();


    if(dht_gettemperaturehumidity(&temperature, &humidity) != -1)
    {
        ;
    }
    else
    {
        ;
    }

    _delay_ms(90);
}//getDHT11

//----------------------------------------------------------
void ioinit (void)
{
    //DDRB  = 0xff; //1 = output, 0 = input

    //DDRB &= ~_BV(DHT11_PIN); //input
    DDRB |= _BV(debug_PIN); //output

    //PORTB = 0x00;
}//ioinit


/*
 * get data from sensor
 */
//----------------------------------------------------------
int8_t dht_getdata(int8_t *temperature, int8_t *humidity)
{
    uint8_t bits[5];
    uint8_t i,j = 0;

    //memset(bits, 0, sizeof(bits));

    //reset port
    //*** high
    DHT_DDR |= (1<<DHT_INPUTPIN); //output
    DHT_PORT |= (1<<DHT_INPUTPIN); //high

    //_delay_ms(15); // 124ms by real measurement of OSC (ATtiny13a at 9.6Mhz int)
    //PORTB |= _BV(debug_PIN);
    _delay_ms(12); // 96ms by real measurement of OSC (ATmega328p at 16Mhz ext, F_CPU=16000000UL)
    //PORTB &= ~_BV(debug_PIN);

    //MCU START REQ 1
    //send Start request for at least 18ms from MCU
    //by making the data line LOW
    // *** low
    DHT_PORT &= ~_BV(DHT_INPUTPIN);

    //PORTB |= _BV(debug_PIN);
    // 24ms by real measurement of OSC
    // (ATmega328p at 16Mhz ext, F_CPU=16000000UL)
    _delay_ms(3);
    //PORTB &= ~_BV(debug_PIN);

    //MCU START REQ 2
    //then MCU make the data line HIGH for  20-40us
    //and wait response from DHT11
    // *** high
	DHT_PORT |= _BV(DHT_INPUTPIN); //high
    //PORTB |= _BV(debug_PIN);
	_delay_us(5); //40ms (ATmega328p at 16Mhz ext)
    //PORTB &= ~_BV(debug_PIN);

    //make the data line as INPUT
	DHT_DDR &= ~_BV(DHT_INPUTPIN);
    //check start condition 1 (DHT11 response to LOW for 80us)
    if((DHT_PIN & _BV(DHT_INPUTPIN)))
    {
        *temperature = 91;
        return -1;
    }

    PORTB |= _BV(debug_PIN);
    _delay_us(10); // 80ms (ATmega328p at 16Mhz ext)
    PORTB &= ~_BV(debug_PIN);
    //_delay_us(500);
    //_delay_ms(30);
    //check start condition 2
    if(!(DHT_PIN & (1<<DHT_INPUTPIN)))
    {
        *temperature =92;
        return -1;
    }

    DHT_DDR &= ~(1<<DHT_INPUTPIN); //input

    DHT_PORT |= _BV(debug_PIN); //high debug pin
    _delay_us(10); // 80ms (ATmega328p at 16Mhz ext)
    DHT_PORT &= ~_BV(debug_PIN); //low debug pin

    //read the data
    uint16_t timeoutcounter = 0;
    for (j=0; j<5; j++)
    { //read 5 byte
        uint8_t result=0;

        for(i=0; i<8; i++)
        {//read every bit
            timeoutcounter = 0;
            while(!(DHT_PIN & (1<<DHT_INPUTPIN)))
            { //wait for an high input (non blocking)
                timeoutcounter++;
                if(timeoutcounter > DHT_TIMEOUT)
                {
                    *temperature = 93;
                    return -1; //timeout
                }
            }

            //_delay_us(6); // this is critical time delay to read correct data
            //if input is high after 30 us, get result
            //PORTB |= _BV(debug_PIN);
            _delay_us(4);
            //PORTB &= ~_BV(debug_PIN);
            if(DHT_PIN & (1<<DHT_INPUTPIN))
                result |= (1<<(7-i));

            timeoutcounter = 0;
            while(DHT_PIN & (1<<DHT_INPUTPIN))
            { //wait until input get low (non blocking)
                timeoutcounter++;
                if(timeoutcounter > DHT_TIMEOUT)
                {
                    *temperature = 94;
                    return -1; //timeout
                }
            }
        }
        bits[j] = result;
    }

    //reset port
    DHT_DDR |= (1<<DHT_INPUTPIN); //output
    DHT_PORT |= (1<<DHT_INPUTPIN); //low
    _delay_ms(15);

    //check checksum
    if ((uint8_t)(bits[0] + bits[1] + bits[2] + bits[3]) == bits[4])
    {
        //return temperature and humidity
        if (bits[2] !=0)
        {
            *temperature = bits[2];
            *humidity = bits[0];
        }
        else
        {
            *temperature = 97;
            *humidity = 98;
        }

        return 0;
    }

    return -1;
}//dht_getdata

int8_t dht_getdata_INT8MHZ(int8_t *temperature, int8_t *humidity)
{
    uint8_t bits[5];
    uint8_t i,j = 0;

    //memset(bits, 0, sizeof(bits));

    //reset port
    //*** high
    DHT_DDR |= (1<<DHT_INPUTPIN); //output
    DHT_PORT |= (1<<DHT_INPUTPIN); //high

    //_delay_ms(15); // 124ms by real measurement of OSC (ATtiny13a at 9.6Mhz int)
    //PORTB |= _BV(debug_PIN);
    _delay_ms(100); // 99ms by ATmega328p at 8Mhz int, F_CPU=1000000UL
    //PORTB &= ~_BV(debug_PIN);

    //MCU START REQ 1
    //send Start request for at least 18ms from MCU
    //by making the data line LOW
    // *** low
    DHT_PORT &= ~_BV(DHT_INPUTPIN);

    //PORTB |= _BV(debug_PIN);
    // 20ms by real measurement of OSC
    // ATmega328p at 8Mhz int, F_CPU=1000000UL
    _delay_ms(20);
    //PORTB &= ~_BV(debug_PIN);

    //MCU START REQ 2
    //then MCU make the data line HIGH for  20-40us
    //and wait response from DHT11
    // *** high
	DHT_PORT |= _BV(DHT_INPUTPIN); //high
    //PORTB |= _BV(debug_PIN);
	//_delay_us(10); //80us (ATmega328p at 16Mhz ext)
    //PORTB &= ~_BV(debug_PIN);

    //make the data line as INPUT
	DHT_DDR &= ~_BV(DHT_INPUTPIN);
    //PORTB |= _BV(debug_PIN);
	_delay_us(40); //40us ATmega328p at 8Mhz int, F_CPU=1000000UL
    //PORTB &= ~_BV(debug_PIN);
    //check start condition 1 (DHT11 response to LOW for 80us)
    if((DHT_PIN & _BV(DHT_INPUTPIN)))
    {
        *temperature = 91;
        return -1;
    }

    //PORTB |= _BV(debug_PIN);
    _delay_us(80); // 80us ATmega328p at 8Mhz int, F_CPU=1000000UL
    //PORTB &= ~_BV(debug_PIN);
    //_delay_us(500);
    //_delay_ms(30);
    //check start condition 2
    if(!(DHT_PIN & (1<<DHT_INPUTPIN)))
    {
        *temperature =92;
        return -1;
    }

    DHT_DDR &= ~(1<<DHT_INPUTPIN); //input

    //DHT_PORT |= _BV(debug_PIN); //high debug pin
    _delay_us(80); //80us ATmega328p at 8Mhz int, F_CPU=1000000UL
    //DHT_PORT &= ~_BV(debug_PIN); //low debug pin

    //read the data
    uint16_t timeoutcounter = 0;
    for (j=0; j<5; j++)
    { //read 5 byte
        uint8_t result=0;

        for(i=0; i<8; i++)
        {//read every bit
            timeoutcounter = 0;
            while(!(DHT_PIN & (1<<DHT_INPUTPIN)))
            { //wait for an high input (non blocking)
                timeoutcounter++;
                if(timeoutcounter > DHT_TIMEOUT)
                {
                    *temperature = 93;
                    return -1; //timeout
                }
            }

            //_delay_us(6); // this is critical time delay to read correct data
            //if input is high after 30 us, get result
            //PORTB |= _BV(debug_PIN);
            _delay_us(30);
            //PORTB &= ~_BV(debug_PIN);
            if(DHT_PIN & (1<<DHT_INPUTPIN))
                result |= (1<<(7-i));

            timeoutcounter = 0;
            while(DHT_PIN & (1<<DHT_INPUTPIN))
            { //wait until input get low (non blocking)
                timeoutcounter++;
                if(timeoutcounter > DHT_TIMEOUT)
                {
                    *temperature = 94;
                    return -1; //timeout
                }
            }
        }
        bits[j] = result;
    }

    //reset port
    DHT_DDR |= (1<<DHT_INPUTPIN); //output
    DHT_PORT |= (1<<DHT_INPUTPIN); //low
    _delay_ms(15);

    //check checksum
    if ((uint8_t)(bits[0] + bits[1] + bits[2] + bits[3]) == bits[4])
    {
        //return temperature and humidity
        if (bits[2] !=0)
        {
            *temperature = bits[2];
            *humidity = bits[0];
        }
        else
        {
            *temperature = 97;
            *humidity = 98;
        }

        return 0;
    }

    return -1;
}//dht_getdata_INT8MHZ

//----------------------------------------------------------
int8_t dht_getdata_EXT16MHZ(int8_t *temperature, int8_t *humidity)
{
    uint8_t bits[5];
    uint8_t i,j = 0;

    //memset(bits, 0, sizeof(bits));

    //reset port
    //*** high
    DHT_DDR |= (1<<DHT_INPUTPIN); //output
    DHT_PORT |= (1<<DHT_INPUTPIN); //high

    //_delay_ms(15); // 124ms by real measurement of OSC (ATtiny13a at 9.6Mhz int)
    //PORTB |= _BV(debug_PIN);
    _delay_ms(12); // 96ms by real measurement of OSC (ATmega328p at 16Mhz ext, F_CPU=16000000UL)
    //PORTB &= ~_BV(debug_PIN);

    //MCU START REQ 1
    //send Start request for at least 18ms from MCU
    //by making the data line LOW
    // *** low
    DHT_PORT &= ~_BV(DHT_INPUTPIN);

    //PORTB |= _BV(debug_PIN);
    // 24ms by real measurement of OSC
    // (ATmega328p at 16Mhz ext, F_CPU=16000000UL)
    _delay_ms(3);
    //PORTB &= ~_BV(debug_PIN);

    //MCU START REQ 2
    //then MCU make the data line HIGH for  20-40us
    //and wait response from DHT11
    // *** high
	DHT_PORT |= _BV(DHT_INPUTPIN); //high
    //PORTB |= _BV(debug_PIN);
	//_delay_us(10); //80us (ATmega328p at 16Mhz ext)
    //PORTB &= ~_BV(debug_PIN);

    //make the data line as INPUT
	DHT_DDR &= ~_BV(DHT_INPUTPIN);
    PORTB |= _BV(debug_PIN);
	_delay_us(5); //40us (ATmega328p at 16Mhz ext)
    PORTB &= ~_BV(debug_PIN);
    //check start condition 1 (DHT11 response to LOW for 80us)
    if((DHT_PIN & _BV(DHT_INPUTPIN)))
    {
        *temperature = 91;
        return -1;
    }

    //PORTB |= _BV(debug_PIN);
    _delay_us(10); // 80ms (ATmega328p at 16Mhz ext)
    //PORTB &= ~_BV(debug_PIN);
    //_delay_us(500);
    //_delay_ms(30);
    //check start condition 2
    if(!(DHT_PIN & (1<<DHT_INPUTPIN)))
    {
        *temperature =92;
        return -1;
    }

    DHT_DDR &= ~(1<<DHT_INPUTPIN); //input

    DHT_PORT |= _BV(debug_PIN); //high debug pin
    _delay_us(10); // 80ms (ATmega328p at 16Mhz ext)
    DHT_PORT &= ~_BV(debug_PIN); //low debug pin

    //read the data
    uint16_t timeoutcounter = 0;
    for (j=0; j<5; j++)
    { //read 5 byte
        uint8_t result=0;

        for(i=0; i<8; i++)
        {//read every bit
            timeoutcounter = 0;
            while(!(DHT_PIN & (1<<DHT_INPUTPIN)))
            { //wait for an high input (non blocking)
                timeoutcounter++;
                if(timeoutcounter > DHT_TIMEOUT)
                {
                    *temperature = 93;
                    return -1; //timeout
                }
            }

            //_delay_us(6); // this is critical time delay to read correct data
            //if input is high after 30 us, get result
            //PORTB |= _BV(debug_PIN);
            _delay_us(4);
            //PORTB &= ~_BV(debug_PIN);
            if(DHT_PIN & (1<<DHT_INPUTPIN))
                result |= (1<<(7-i));

            timeoutcounter = 0;
            while(DHT_PIN & (1<<DHT_INPUTPIN))
            { //wait until input get low (non blocking)
                timeoutcounter++;
                if(timeoutcounter > DHT_TIMEOUT)
                {
                    *temperature = 94;
                    return -1; //timeout
                }
            }
        }
        bits[j] = result;
    }

    //reset port
    DHT_DDR |= (1<<DHT_INPUTPIN); //output
    DHT_PORT |= (1<<DHT_INPUTPIN); //low
    _delay_ms(15);

    //check checksum
    if ((uint8_t)(bits[0] + bits[1] + bits[2] + bits[3]) == bits[4])
    {
        //return temperature and humidity
        if (bits[2] !=0)
        {
            *temperature = bits[2];
            *humidity = bits[0];
        }
        else
        {
            *temperature = 97;
            *humidity = 98;
        }

        return 0;
    }

    return -1;
}//dht_getdata_EXT16MHZ

/*
 * get temperature
 */
//----------------------------------------------------------
int8_t dht_gettemperature(int8_t *temperature)
{
    int8_t humidity = 0;
    return dht_getdata(temperature, &humidity);
}//dht_gettemperature

/*
 * get humidity
 */
int8_t dht_gethumidity(int8_t *humidity)
{
    int8_t temperature = 0;
    return dht_getdata(&temperature, humidity);
}//dht_gethumidity

/*
 * get temperature and humidity
 */
//----------------------------------------------------------
int8_t dht_gettemperaturehumidity(int8_t *temperature, int8_t *humidity)
{
    //return dht_getdata(temperature, humidity);
    //return dht_getdata_EXT16MHZ(temperature, humidity);
    return dht_getdata_INT8MHZ(temperature, humidity);

}//dht_gettemperaturehumidity



