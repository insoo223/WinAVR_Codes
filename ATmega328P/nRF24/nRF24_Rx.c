/**************************************************************
 Target MCU & clock speed: ATmega328P @ 8Mhz internal
 Name    : nRF24_Rx.c
 Author  : Insoo Kim (insoo@hotmail.com)
 Created    : April 19, 2015
 Updated    : April 25, 2015

 Description:
 HEX size[Byte]: 1436 out of 32K

 Ref:
 * RF_Tranceiver.c
 *
 * Created: 2012-08-10 15:24:35
 *  Author: Kalle
 *  Atmega88

 ** http://gizmosnack.blogspot.kr/2013/04/tutorial-nrf24l01-and-avr.html
 ** https://gist.github.com/klalle/5652658
 *****************************************************************/

#include <avr/io.h>
#include <stdio.h>
#define F_CPU 8000000UL  // 8 MHz
#include <util/delay.h>
#include <avr/interrupt.h>

#include "nRF24L01.h"

#define halfSec 500

#define sckPin PB5
#define misoPin PB4
#define mosiPin PB3
#define csnPin PB2
#define cePin PB1
#define ledPin PB0

#define dataLen 5
uint8_t *data;
uint8_t *arr;

uint8_t totalSerialOut=0;

//**************** FP: Function Prototypes
void USART_Tx_asciiCode(char);

/*****************8MHz - 1MHz*****************************/
void clockprescale(void)
{
    //Prepare the chip for a change of clock prescale
    //  (CLKPCE=1 and the rest zeros)
    CLKPR = 0b10000000;
    CLKPR = 0b00000000;
    //Wanted clock prescale
    //(CLKPCE=0 and the four first bits CLKPS0-3 sets division factor = 1)
    //See page 38 in datasheet
}
////////////////////////////////////////////////////


/*****************USART*****************************/
//Initiering

void USART_init(void)
{
    DDRD |= (1<<1);	//Set TXD (PD1) as output for USART

    unsigned int USART_BAUDRATE = 9600;		//Same as in "terminal.exe"
    unsigned int ubrr = (((F_CPU / (USART_BAUDRATE * 16UL))) - 1);	//baud prescale calculated according to F_CPU-define at top

    /*Set baud rate */
    UBRR0H = (unsigned char)(ubrr>>8);
    UBRR0L = (unsigned char)ubrr;

    /*	Enable receiver and transmitter */
    UCSR0B = (1<<RXEN0)|(1<<TXEN0);

    /* Set frame format: 8data, 2stop bit, The two stop-bits does not seem to make any difference in my case!?*/
    UCSR0C = (1<<USBS0)|(3<<UCSZ00);

}

void USART_Transmit(uint8_t data)
{
    /* Wait for empty transmit buffer */
    while ( !( UCSR0A & (1<<UDRE0)) );
    /* Put data into buffer, sends the data */
    UDR0 = data;
}//USART_Transmit

//-------------------------------------
void USART_Tx_Test()
{
    char c;
    uint8_t n=0;

    for (c=0x30; c<0x7f; c++)
    {
        if (n % 16 == 0)
        {
            USART_Transmit(10);
            USART_Transmit(13);
        }
        n++;

        USART_Transmit(c);
        PORTB|=_BV(ledPin);
        _delay_ms(halfSec/4);
        PORTB&=~_BV(ledPin);
        _delay_ms(halfSec/4);
    }

}//USART_Tx_Test

void USART_Rx_Test()
{
    /*
    1. Open a serial monitor like, HyterTerminal, Arduino etc
    2. Send some char or sentence and return
    3. Exactly same char or sentence will be display in your serial monitor
    */
    ;
}
//-------------------------------------

uint8_t USART_Receive( void )
{
    /* Wait for data to be received */
    //This loop is only needed if you not use the interrupt...
    while ( !(UCSR0A & (1<<RXC0)) );

    /* Get and return received data from buffer */
    return UDR0; //Return the received byte
}

/*****************SPI*****************************/
//initiering
void SPI_init(void)
{
    //Set SCK (PB5), MOSI (PB3) , CSN (SS & PB2) & CE  as outport
    //DDRB |= (1<<DDB5) | (1<<DDB3) | (1<<DDB2) |(1<<DDB1);
    DDRB |= (1<<sckPin) | (1<<mosiPin) | (1<<csnPin) |(1<<cePin);

    // Enable SPI, Master, set clock rate fck/16 ..
    SPCR |= (1<<SPE)|(1<<MSTR); // |(1<<SPR0);// |(1<<SPR1);

    //chip select is active low,
    //so make it unavailable by now.
    PORTB|=_BV(csnPin);

    PORTB&=~_BV(cePin);
}//SPI_init

//Note that these functions always returns something,
//this returned message is only cared fore when reading data
//from the nRF
//(a more appropriate name of the function might be "Write_Read_Byte_SPI").
char SPI_ByteReadWrite(unsigned char cData)
{
    //Load byte to Data register
    SPDR = cData;

    /* Wait for transmission complete */
    while(!(SPSR & (1<<SPIF)));

    return SPDR;
}//SPI_ByteReadWrite
////////////////////////////////////////////////////


/*****************in/out***************************/
void ioinit(void)
{
    DDRB |= _BV(ledPin); //output
    /*
    DDRB |= _BV(mosiPin); //output
    DDRB |= _BV(cePin); //output
    DDRB |= _BV(csnPin); //output
    DDRB &= ~_BV(misoPin); //input
    */
}
////////////////////////////////////////////////////


/*****************interrupt***************************/

void INT0_interrupt_init(void)
{
    //Extern interrupt on INT0, make sure input!
    DDRD &= ~(1<<DDD2);

    EICRA |=  (1<<ISC01);// INT0 falling edge	PD2
    EICRA  &=  ~(1<<ISC00);// INT0 falling edge	PD2

    EIMSK |=  (1<<INT0);	//enablar int0
    //sei();              // Enable global interrupts gÃ¶rs sen
}

//Enable interrupt that triggers on USART-data is received,
void USART_interrupt_init(void)
{
    UCSR0B |= (1<<RXCIE0);
}

//////////////////////////////////////////////////////

//Reading a register on the nRF
uint8_t GetReg(uint8_t reg)
{
    //The nRF starts listening for commands when the CSN-pin goes low.
    //after a delay of 10us it accepts a single byte through SPI
    _delay_us(10);

    //PORTB&=~_BV(csnPin);	//CSN low
    PORTB&=~_BV(csnPin);

    _delay_us(10);
    //"reg" register will be read back
    //R_Register set the nRF as read-mode
    SPI_ByteReadWrite(R_REGISTER + reg);

    _delay_us(10);
    //send NOP (dummy byte) once
    //to receive back the 1st byte in the "reg" register
    reg = SPI_ByteReadWrite(NOP);

    _delay_us(10);
    //PORTB|=_BV(csnPin);
    //CSN goes High to do nothing
    PORTB|=_BV(csnPin);

    return reg;	// Return the read register
}//GetReg


/*****************nrf-setup***************************/
uint8_t *WriteToNrf(uint8_t ReadWrite, uint8_t reg, \
                    uint8_t *val, uint8_t antVal)
{
    int i;
    const uint8_t delayAdj = 10;
    static uint8_t ret[dataLen];

    cli();	//disable global interrupt

    if (ReadWrite == W)
    {
        reg = W_REGISTER + reg;	//ex: reg = EN_AA: 0b0010 0000 + 0b0000 0001 = 0b0010 0001
    }


    _delay_us(delayAdj);
    PORTB&=~_BV(csnPin);

    _delay_us(delayAdj);
    SPI_ByteReadWrite(reg);
    _delay_us(delayAdj);

    for(i=0; i<antVal; i++)
    {
        if (ReadWrite == R && reg != W_TX_PAYLOAD)
        {
            ret[i]=SPI_ByteReadWrite(NOP);
            _delay_us(delayAdj);
        }
        else
        {
            SPI_ByteReadWrite(val[i]);
            _delay_us(delayAdj);
        }
    }
        PORTB|=_BV(csnPin);

    sei(); //enable global interrupt

    return ret;	//returnerar en array
}//WriteToNrf


//initierar nrf'en (obs nrfen mÃ¥ste vala i vila nÃ¤r detta sker CE-lÃ¥g)
void nRF24L01_init(void)
{
    //allow radio to reach power down if shut down
    _delay_ms(100);

    //array of integers to send to "WriteToNrf"
    uint8_t val[5];

    //EN_AA - (auto-acknowledgements)
    //Transmitter gets automatic response
    //from receiver when successful transmission. (a lovely function!)
    //Enable auto acknowledgement data pipe 0
    val[0]=0x01;
    WriteToNrf(W, EN_AA, val, 1);

    //SETUP_RETR (the setup for "EN_AA")
    //"2" sets it up to 750uS delay between every retry
    //(at least 500us at 250kbps
    //  and if payload >5bytes in 1Mbps,
    //  and if payload >15byte in 2Mbps)
    //"F" is number of retries (1-15, now 15)
    val[0]=0x2F;
    WriteToNrf(W, SETUP_RETR, val, 1);

    //enable data pipe 0
    val[0]=0x01;
    WriteToNrf(W, EN_RXADDR, val, 1);

    //RF_Adress width setup
    //0x03 as 5 bytes of AW
    val[0]=0x03;
    WriteToNrf(W, SETUP_AW, val, 1);

    //RF channel setup
    //0x01 as 2.401GHz (both for Tx & Rx)
    val[0]=0x01;
    WriteToNrf(W, RF_CH, val, 1);

    //RF setup
    //bit2&1 are for Tx power and bit 0 is for LNA gain
    //11 of bit 2&1 as -0dB power
    //00 as -18dB power
    //val[0]=0x07; //-0dB
    val[0]=0x01; //-18dB
    WriteToNrf(W, RF_SETUP, val, 1);

    //Rx RF_Address setupt 5 bytes
    //if EN_AA is enabled, set Tx & Rx addresses as same
    int i;
    for(i=0; i<5; i++)
    {
        val[i]=0x12;
    }
    WriteToNrf(W, RX_ADDR_P0, val, 5);

    //Tx RF_Address setupt 5 bytes
    for(i=0; i<5; i++)
    {
        val[i]=0x12;	//RF channel registry 0b10111100 x 5 - skriver samma RF_Adress 5ggr fÃ¶r att fÃ¥ en lÃ¤tt och sÃ¤ker RF_Adress (samma pÃ¥ Reciverns chip och pÃ¥ RX-RF_Adressen ovan om EN_AA enablats!!!)
    }
    WriteToNrf(W, TX_ADDR, val, 5);

    //payload width setup -
    //How many bytes to send per transmission? 1-32bytes
    val[0]=dataLen;
    WriteToNrf(W, RX_PW_P0, val, 1);

    //CONFIG reg setup
    //Tx mode, CRC 2bytes, and do not raise interrupt for MAX_RT
    //0b0000 1110 config registry
    //bit "4": 1=mask_Max_RT,i.e., IRQ-interrupt is not triggered if Tx failed
    //bit "3": 1=enable CRC,  0=disable CRC
    //bit "2": 1=CRC 2bytes,  0=CRC 1byte
    //bit "1": 1=power up,  0=power down
    //bit "0": 1=Reciver 0=Transmitter
    val[0]=0x1F; //Rx
    //val[0]=0x1E; //Tx

    WriteToNrf(W, CONFIG, val, 1);

    //device need 1.5ms to reach standby mode
    _delay_ms(100);

    //sei();
}//nRF24L01_init

void ChangeAddress(uint8_t adress)
{
    _delay_ms(100);
    uint8_t val[5];
    //RX RF_Adress setup 5 byte - vÃ¤ljer RF_Adressen pÃ¥ Recivern (MÃ¥ste ges samma RF_Adress om Transmittern har EN_AA pÃ¥slaget!!!)
    int i;
    for(i=0; i<5; i++)
    {
        val[i]=adress;	//RF channel registry 0b10101011 x 5 - skriver samma RF_Adress 5ggr fÃ¶r att fÃ¥ en lÃ¤tt och sÃ¤ker RF_Adress (samma pÃ¥ transmitterns chip!!!)
    }
    WriteToNrf(W, RX_ADDR_P0, val, 5); //0b0010 1010 write registry - eftersom vi valde pipe 0 i "EN_RXADDR" ovan, ger vi RF_Adressen till denna pipe. (kan ge olika RF_Adresser till olika pipes och dÃ¤rmed lyssna pÃ¥ olika transmittrar)

    //TX RF_Adress setup 5 byte
    for(i=0; i<5; i++)
    {
        val[i]=adress;	//RF channel registry 0b10111100 x 5
    }
    WriteToNrf(W, TX_ADDR, val, 5);
    _delay_ms(100);
}
/////////////////////////////////////////////////////

/*****************Funktioner***************************/
void reset(void)
{
    _delay_us(10);
    PORTB&=~_BV(csnPin);	//CSN low
    _delay_us(10);
    SPI_ByteReadWrite(W_REGISTER + STATUS);
    _delay_us(10);
    SPI_ByteReadWrite(0b01110000);
    _delay_us(10);
    PORTB|=_BV(csnPin);	//CSN IR_High
}

/*********************Reciverfunktioner********************************/
void receive_payload(void)
{
    sei();		//Enable global interrupt

    PORTB|=_BV(cePin);	//CE IR_High
    _delay_ms(1000);
    PORTB&=~_BV(cePin);

    cli();	//Disable global interrupt
}

void transmit_payload(uint8_t *W_buff)
{
    WriteToNrf(R, FLUSH_TX, W_buff, 0);

    WriteToNrf(R, W_TX_PAYLOAD, W_buff, dataLen);

    //sei();	//enable global interrupt-
    //USART_Transmit(GetReg(STATUS));

    _delay_ms(10);
    //CE high to Tx
    PORTB|=_BV(cePin);
    //delay at least 10us!
    _delay_us(20);
    //CE low to stop Tx
    PORTB&=~_BV(cePin);
    _delay_ms(10);

    //cli();	//Disable global interrupt

}//transmit_payload

void nRF24L01_Tx_Test()
{
    uint8_t W_buffer[5];
    int i;
    uint8_t val[5];

    for(i=0; i<5; i++)
    {
        W_buffer[i] = 0x41+i;
        USART_Transmit(W_buffer[i]);
        _delay_ms(halfSec/5);
    }

    transmit_payload(W_buffer);

    _delay_ms(halfSec);
    if ((GetReg(STATUS) & 1<<MAX_RT))
    {
        //FAIL
        USART_Transmit(0x40); // @
        val[0] = STATUS | _BV(MAX_RT);
        WriteToNrf(W, STATUS, val, 1);

    }
    else
        USART_Transmit(0x24); // $


}//nRF24L01_Tx_Test

void nRF24L01_WriteRead_Test_Addr()
{
    uint8_t val[5], dummy, *rxAddr;
    int i;

    //Rx address
    val[0] = 0x01;
    WriteToNrf(W, EN_RXADDR, val, 1);
    for(i=0; i<5; i++)
        val[i] = 0x12;
    WriteToNrf(W, RX_ADDR_P0, val, 5);

    //to suppress compile-time warning
    rxAddr = &dummy;
    rxAddr = WriteToNrf(R, RX_ADDR_P0, rxAddr, 5);

    for(i=0; i<5; i++)
    {
        USART_Tx_asciiCode(*(rxAddr+i));
        //USART_Tx_asciiCode(0x12);
        USART_Transmit(0x20);
    }
        USART_Transmit(10);
        USART_Transmit(13);

}//nRF24L01_WriteRead_Test_Addr

void halfSec_Test()
{
    PORTB |= _BV(ledPin);
    _delay_ms(halfSec);
    PORTB &= ~_BV(ledPin);
    _delay_ms(halfSec);

}
/////////////////////////////////////////////////////
void USART_Tx_asciiCode(char n)
{
    char n16, n1;

    //n16 = n/16
    n16=n>>4;

    if (n16 < 10)
        n16 = n16 + 0x30; //'0'
    else
        n16 = n16 - 10 + 0x41; //'A'

    n1=n%16;
    if (n1 < 10)
        n1 = n1 + 0x30;
    else
        n1 = n1 - 10 + 0x41;
    USART_Transmit(n16);
    USART_Transmit(n1);
}//USART_Tx_asciiCode

int main(void)
{
    //int k=0;
    uint8_t i;
    uint8_t n=0;

    clockprescale();
    USART_init();
    SPI_init();
    ioinit();
    INT0_interrupt_init();
    USART_interrupt_init();

    nRF24L01_init();

    PORTB|=_BV(ledPin);
    _delay_ms(halfSec*2);
    PORTB&=~_BV(ledPin);
    _delay_ms(halfSec*2);
    //sei();
    while(1)
    {
        //Wait for USART-interrupt to send data...
        //Tx nRF24
        //if (k++ % 10 == 0)
        //{
            USART_Transmit(10);
            USART_Transmit(13);
        //}
        USART_Tx_asciiCode(++n);
        USART_Transmit(0x5E); // '^'
        reset();
        receive_payload();
        data = WriteToNrf(R, R_RX_PAYLOAD, data, dataLen);
        //reset();
        for (i=0; i<dataLen; i++)
            USART_Transmit(data[i]);
        //nRF24L01_Tx_Test();
        //nRF24L01_WriteRead_Test_Addr();
        //USART_Tx_asciiCode(GetReg(CONFIG));
        //USART_Tx_asciiCode(GetReg(SETUP_RETR));

        //Rx nRF24
        //reset();
        //receive_payload();

        //_delay_ms(500);
        USART_Transmit(0x7E); // '~'
        _delay_ms(1000);


    }
    return 0;
}//main

//--------------------------
ISR(INT0_vect)
{
    cli();	//Disable global interrupt
    PORTB&=~_BV(cePin);

    PORTB|=_BV(ledPin); //led on
    _delay_ms(500);
    PORTB&=~_BV(ledPin); //led off

    //Receiver function to print out on usart:
    data=WriteToNrf(R, R_RX_PAYLOAD, data, dataLen);
    reset();
    int i;
    for (i=0;i<dataLen;i++)
    {
        USART_Transmit(data[i]);
    }
    sei();

}

//Vector that triggers when computer sends something to the Atmega88
ISR(USART_RX_vect)
{
    uint8_t W_buffer[dataLen];	//Creates a buffer to receive data with specified length (ex. dataLen = 5 bytes)

    int i;

    for (i=0;i<dataLen;i++)
    {
        if (totalSerialOut % 16 == 0)
        {
            USART_Transmit(10);
            USART_Transmit(13);
        }
        W_buffer[i]=USART_Receive();	//receive the USART
        USART_Transmit(W_buffer[i]);	//Transmit the Data back to the computer to make sure it was correctly received
        //This probably should wait until all the bytes is received, but works fine in to send and receive at the same time... =)
        totalSerialOut++;
    }

    reset();	//reset irq

    if (W_buffer[0]=='9')
    {
        ChangeAddress(0x13);	//change address to send to different receiver
        transmit_payload(W_buffer);
        ChangeAddress(0x12);
    }
    else
    {
        transmit_payload(W_buffer);
    }

    USART_Transmit('#');


}
