#include <avr/interrupt.h>
#include <avr/sleep.h>

ISR(WDT_vect) {
    // Toggle PB0 output state
    PORTB ^= 1<<PB0;
}

int main(void) {
    // Set up PB0 mode to output
    DDRB = 1<<DDB0;

    // temporarily prescale timer to 4s so we can measure current
    WDTCR |= (1<<WDP3); // (1<<WDP2) | (1<<WDP0);

    // Enable watchdog timer interrupts
    WDTCR |= (1<<WDTIE);

    sei(); // Enable global interrupts

    // Use the Power Down sleep mode
    set_sleep_mode(SLEEP_MODE_PWR_DOWN);

    for (;;) {
        sleep_mode();   // go to sleep and wait for interrupt...
    }
}
