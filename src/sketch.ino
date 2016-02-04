#include <TVout.h>
#include <fontALL.h>
#include <pollserial.h>

#define W 136
#define H 96

#define DEFAULT_DELAY 50

TVout tv;
pollserial pserial;

unsigned long pulse_dur;

void setup()  {
    tv.begin(PAL, W, H);
    tv.set_hbi_hook(pserial.begin(9600));

    initOverlay();

    tv.select_font(font6x8);
    tv.fill(0);

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
    // check if data has been sent from the computer:
    if (pserial.available()) {
        // read the most recent byte (which will be from 0 to 255):
        int val = pserial.read();
        if (val) {
            tv.draw_circle(tv.hres()/2, tv.vres()/2, tv.vres()/3, WHITE);
            tv.delay(pulse_dur);
            tv.clear_screen();
        }
    }
}
