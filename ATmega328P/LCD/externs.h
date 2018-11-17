/**************************************************************
 Target MCU & clock speed: ATmega328P @ 1Mhz internal
 Name    : externs.h
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
extern uint8_t wd;
extern uint8_t glbWords0[10][2][17];
extern uint8_t glbWords1[10][2][17];

extern uint8_t prevMenuCnt;
extern uint8_t prevMenuLayer;;

extern uint8_t program_author[];
extern uint8_t program_version[];
extern uint8_t program_email[];
extern uint8_t program_date[];

extern uint8_t menu_str1[];
extern uint8_t menu_str2[];

extern int8_t dht_getdata(int8_t *, int8_t *);
extern int8_t dht_getdata_EXT16MHZ(int8_t *, int8_t *);
extern int8_t dht_getdata_INT8MHZ(int8_t *, int8_t *);

extern int8_t dht_gettemperaturehumidity(int8_t *, int8_t *);
extern void ioinit (void);

extern int8_t hour, min, sec;
extern int8_t year, month, date;
extern int8_t monthEndDate, day;
extern int8_t accumulatedHour, accumulatedMin, accumulatedSec;
//extern uint8_t hourlyAdjusted;

extern void proceedClock();

extern void WDT_Init(void);
extern void initINT(void);
extern void check_wdt(void);
extern void setup_wdt(void);
extern void init_devices(void);

extern void parseCompileTime(void);
extern void calcDay(void);

// Function Prototypes
extern void chkButtonAndToggleBacklight();
extern void config();
extern void turnOnLCDBacklight();
extern void turnOffLCDBacklight();
extern void lcd_SysTime();
extern void lcd_dispON();
extern void lcd_dispOFF();
extern void lcd_dispRealClock();
extern void lcd_dispAccumulatedTime();
extern void lcd_showDHT11();
extern void lcd_testString();
extern void lcd_dispProgInfo();
extern void lcd_dispLineOneTwo(uint8_t *, uint8_t *);
extern void lcd_dispWords(uint8_t );
extern void lcd_dispMenu();
extern void lcd_padSpace(uint8_t *);
extern void lcd_write_4(uint8_t);
extern void lcd_write_instruction_4d(uint8_t);
extern void lcd_write_character_4d(uint8_t);
extern void lcd_write_string_4d(uint8_t *);
extern void lcd_init_4d(void);


//UTIL.C
extern void sysClockTest();
extern void countWithinOneSec(uint8_t *, uint8_t);
extern void countButton(void);
extern void showMenuMsg(uint8_t, uint8_t);
extern void adjustTime(uint8_t *, int8_t *, int8_t, int8_t);
//extern void adjustHour();
//extern void adjustMin();
//extern void adjustSec();

//extern void assignGlbWords0Bio0();
extern void assignGlbWords0Bio1();
extern void assignGlbWords0Bio2();
