/**************************************************************
 Target MCU & clock speed: ATmega328P @ 1Mhz internal
 Name    : globals.c
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
#include <avr/pgmspace.h>
#include "defines.h"



uint8_t prevMenuCnt;
uint8_t prevMenuLayer;

int8_t hour=0, min=0, sec=0;
int8_t year=0, month=0, date=0;
int8_t monthEndDate, day=0;
int8_t accumulatedHour=0, accumulatedMin=0,accumulatedSec=0;
//uint8_t hourlyAdjusted=0;

// Program ID
uint8_t program_author[]   = "Insoo Kim     ";
uint8_t program_version[]  = "LCD-ATmega328p";
uint8_t program_email[]    = "insoo@hotmail";
uint8_t program_date[]     = "May 23, 2015  ";

// Menu info
uint8_t menu_str1[]   = "1:Rdy or PrevSel";
uint8_t menu_str2[]  =  "2CK3TH4BL5AJ6AT";

