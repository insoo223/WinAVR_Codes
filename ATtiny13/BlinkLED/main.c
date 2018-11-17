#include <avr/io.h>
#include <util/delay.h>

int main(void)
{
    DDRB = 0b00000001;           /* make the LED pin an output */

    for(;;)
    {
        PORTB ^= 1 << PB0;    /* toggle the LED */
        _delay_ms(150);  /* max is 262.14 ms / F_CPU in MHz */
    }

    return 0; // never reach here
}
