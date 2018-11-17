/**************************************************************
 Target MCU & clock speed: ATmega328P @ 1Mhz internal
 Name    : strFunc.c
 Author  : Insoo Kim (insoo@hotmail.com)
 Created : May 15, 2015
 Updated : May 24, 2015

 Description: Get system compile time & date and display on LCD 2*16
    Button toggling to turn on or off the backlight of LCD

 HEX size[Byte]:

 Ref:
    Donald Weiman    (weimandn@alfredstate.edu)
    http://web.alfredstate.edu/weimandn/programming/lcd/ATmega328/LCD_code_gcc_4d.html
 *****************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>
#include <avr/sleep.h>
#include "externs.h"
#include "defines.h"
#include <util/delay.h>

/*
extern uint8_t hour, min, sec;
extern uint8_t year, month, date;
extern uint8_t monthEndDate, day;
*/

void parseCompileTime()
{
    char *p;
    char sTime[4][3];
    char sDate[4][10];
    uint8_t n=0;
    uint16_t yearLong;

    p = strtok(__TIME__,":");
    while (*p)
    {
        strcpy((char *)&sTime[n++], p);
        p=strtok(NULL, ":");
    }
    hour=atoi(sTime[0]);
    min=atoi(sTime[1]);
    //give some delay (8 seconds), to compile & upload by human click
    //sec=atoi(sTime[2]) + 8;

    //if you change power source to battery,
    //  you'd better put more time allowance
    sec=atoi(sTime[2]) + 15;
    //hour = atoi(sHour);

    n=0;
    p = strtok(__DATE__," ");
    while (*p)
    {
        strcpy((char *)&sDate[n++], p);
        p=strtok(NULL, " ");
    }

    if ( strcmp(sDate[0], "Jan") == 0 )
        month = 1;
    else if ( strcmp(sDate[0], "Feb") == 0 )
        month = 2;
    else if ( strcmp(sDate[0], "Mar") == 0 )
        month = 3;
    else if ( strcmp(sDate[0], "Apr") == 0 )
        month = 4;
    else if ( strcmp(sDate[0], "May") == 0 )
        month = 5;
    else if ( strcmp(sDate[0], "Jun") == 0 )
        month = 6;
    else if ( strcmp(sDate[0], "Jul") == 0 )
        month = 7;
    else if ( strcmp(sDate[0], "Aug") == 0 )
        month = 8;
    else if ( strcmp(sDate[0], "Sep") == 0 )
        month = 9;
    else if ( strcmp(sDate[0], "Oct") == 0 )
        month = 10;
    else if ( strcmp(sDate[0], "Nov") == 0 )
        month = 11;
    else if ( strcmp(sDate[0], "Dec") == 0 )
        month = 12;

    date=atoi(sDate[1]);
    yearLong=atoi(sDate[2]);
    year=yearLong%1000;

    calcDay();
}//parseCompileTime

void calcDay()
{
    day = (date%7);
}

