#include <TVout.h>
#include <fontALL.h>

#define W 136
#define H 96

#define DEFAULT_DELTA 1000
#define DEFAULT_DELAY 200

TVout tv;

unsigned long pulse_dur;
unsigned long pulse_delta;

void setup()  {
    tv.begin(PAL, W, H);
    initOverlay();

    tv.select_font(font6x8);
    tv.fill(0);

    pulse_delta = DEFAULT_DELTA;
    pulse_dur = DEFAULT_DELAY;
}

// Initialize ATMega registers for video overlay capability.
// Must be called after tv.begin().
void initOverlay() {
    TCCR1A = 0;
    // Enable timer1.  ICES0 is set to 0 for falling edge detection on input
    // capture pin.
    TCCR1B = _BV(CS10);

    // Enable input capture interrupt
    TIMSK1 |= _BV(ICIE1);

    // Enable external interrupt INT0 on pin 2 with falling edge.
    EIMSK = _BV(INT0);
    EICRA = _BV(ISC01);
}

// Required to reset the scan line when the vertical sync occurs
ISR(INT0_vect) {
    display.scanLine = 0;
}

void loop() {
    tv.draw_circle(tv.hres()/2, tv.vres()/2, tv.vres()/3, WHITE);
    tv.delay(pulse_dur);
    tv.clear_screen();
    tv.delay(pulse_delta);
}
