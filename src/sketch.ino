#include <TVout.h>
#include <fontALL.h>
#include <pollserial.h>

#define DEBUG

#define W 128
#define H 96

#define DEFAULT_DELAY 75

#define PB1_DRAW   3
#define PB2_MANUAL 4
#define PB3        5
#define PB4        10 
#define PB5        11

#define POT1_POSX  A0       // position X
#define POT2_POSY  A1       // position Y
#define POT3_ANGLE A2       // angle
#define POT4_SIZE  A3       // size
#define POT5_TEMPO A4       // tempo / step size
#define POT6_PDUR  A5       // pulse duration

TVout tv;
pollserial pserial;

bool tick, draw;
byte x, y, size, shape;
byte new_x, new_y, new_size, new_shape;
unsigned long pulse_dur;
unsigned long pulse_ctr;

void setup()  {
    tv.begin(PAL, W, H);
    tv.set_hbi_hook(pserial.begin(9600));

    initOverlay();

    pinMode(PB1_DRAW, INPUT_PULLUP);
    pinMode(PB2_MANUAL, INPUT_PULLUP);
    pinMode(PB3, INPUT_PULLUP);
    pinMode(PB4, INPUT_PULLUP);
    pinMode(PB5, INPUT_PULLUP);

    tv.select_font(font6x8);
    tv.fill(0);

    draw = false;

    pulse_ctr = 0;
    pulse_dur = DEFAULT_DELAY;

    // Set default parameters for shape
    x = tv.hres() / 2;
    y = tv.vres() / 2;
    size = tv.vres() / 3;
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
#ifdef DEBUG
    int sensorValue, value;

    // Print pot values
    for (int i = 0; i < 6; i++) {
        sensorValue = analogRead(i);
        value = map(sensorValue, 0, 1023, 0, 255);

        pserial.print(value, DEC);
        pserial.print(' ');
    }
    pserial.print(digitalRead(PB1_DRAW));
    pserial.print(' ');
    pserial.print(digitalRead(PB2_MANUAL));
    pserial.print(' ');
    pserial.print(digitalRead(PB3));
    pserial.print(' ');
    pserial.print(digitalRead(PB4));
    pserial.print(' ');
    pserial.print(digitalRead(PB5));
    pserial.println(';');
#endif

    // check if data has been sent from the computer:
    if (pserial.available()) {
        // read the most recent byte (which will be from 0 to 255):
        int val = pserial.read();
        tick = !val;
        if (tick) draw = true;
    } else {
        tick = false;
    }

    if (draw) {
        new_shape = (analogRead(POT3_ANGLE) < 512) ? 0 : 1;
        new_size = map(analogRead(POT4_SIZE), 0, 1023, 0, H/2);
        new_x = map(analogRead(POT1_POSX), 0, 1023, new_size, W-new_size);
        new_y = map(analogRead(POT2_POSY), 0, 1023, new_size, H-new_size);

        if (tick || new_shape != shape || new_size != size || new_x != x || new_y != y) {
            shape = new_shape;
            size = new_size;
            x = new_x;
            y = new_y;

            tv.clear_screen();

            switch (shape) {
            case 0:
                tv.draw_circle(x, y, size, WHITE);
                break;
            case 1:
                tv.draw_rect(x-size, y-size, size*2, size*2, WHITE);
                break;
            }
        }
    }

    pulse_ctr++;
    if (pulse_ctr > pulse_dur) {
        pulse_dur = map(analogRead(POT6_PDUR), 0, 1023, 0, 1000);
        pulse_ctr = 0;
        draw = false;
        tv.clear_screen();
    }
}
