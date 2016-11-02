#include <TVout.h>
#include <fontALL.h>
#include <pollserial.h>

#define W 128
#define H 96

#define DEFAULT_DELAY 75

TVout tv;
pollserial pserial;

unsigned long pulse_dur;

void setup()  {
    tv.begin(PAL, W, H);
    tv.set_hbi_hook(pserial.begin(9600));

    initOverlay();

    tv.select_font(font6x8);
    tv.fill(0);

    tv.draw_circle(tv.hres()/2, tv.vres()/2, tv.vres()/3, WHITE);

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
  return;
  
    // check if data has been sent from the computer:
    if (pserial.available()) {
        int sensorValue = analogRead(A2);
        pulse_dur = map(sensorValue, 0, 1023, 0, 1000);

        if (pulse_dur == 0) return;

        // read the most recent byte (which will be from 0 to 255):
        int val = pserial.read();
        if (!val) {
            tv.draw_circle(tv.hres()/2, tv.vres()/2, tv.vres()/3, WHITE);
        }

        // dots
        if (val == 0) {
            tv.draw_rect(10, 10, 5, 5, WHITE);
        } else if (val == 1) {
            tv.draw_rect(W-20, 10, 5, 5, WHITE);
        } else if (val == 2) {
            tv.draw_rect(10, H-20, 5, 5, WHITE);
        } else {
            tv.draw_rect(W-20, H-20, 5, 5, WHITE);
        }

        tv.delay(pulse_dur);
        tv.clear_screen();
    }
}
