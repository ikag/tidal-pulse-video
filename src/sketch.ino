#include <TVout.h>
#include <fontALL.h>
#include <pollserial.h>

//#define DEBUG

#define W 128
#define H 96

#define DEFAULT_DELAY 250

#define PB1_DRAW   4
#define PB2_MANUAL 11
#define PB3        6
#define PB4        12
#define PB5        10

#define POT1_POSX  A0       // position X
#define POT2_POSY  A1       // position Y
#define POT3_ANGLE A2       // angle
#define POT4_SIZE  A3       // size
#define POT5_TEMPO A4       // tempo / step size
#define POT6_PDUR  A5       // pulse duration

#define SAMPLES 16

TVout tv;
pollserial pserial;

bool tick, draw;
int step, oldStep;
byte x, y, size, shape;
byte new_x, new_y, new_size, new_shape;
unsigned long pulse_dur;
unsigned long pulse_ctr;

byte samples_i = 0;
byte x_samp[SAMPLES] = {};
byte y_samp[SAMPLES] = {};
byte size_samp[SAMPLES] = {};
unsigned int x_total, x_avg;
unsigned int y_total, y_avg;
unsigned int size_total, size_avg;


void setup()  {
    tv.begin(PAL, W, H);
    tv.set_hbi_hook(pserial.begin(9600));

    initOverlay();

    tv.select_font(font6x8);
    tv.fill(0);

    draw = false;

    step = 0;
    pulse_ctr = 0;
    pulse_dur = DEFAULT_DELAY;

    // Fill buffers with default parameters for x, y and shape
    for (byte i = 0; i < SAMPLES; i++) {
        x_samp[i] = tv.hres() / 2;
        y_samp[i] = tv.vres() / 2;
        size_samp[i] = tv.vres() / 3;

        x_total += x_samp[i];
        y_total += y_samp[i];
        size_total += size_samp[i];
    }

    // Use pull-up resistors on push button inputs, as they are wired to ground
    pinMode(PB1_DRAW, INPUT_PULLUP);
    pinMode(PB2_MANUAL, INPUT_PULLUP);
    pinMode(PB3, INPUT_PULLUP);
    pinMode(PB4, INPUT_PULLUP);
    pinMode(PB5, INPUT_PULLUP);
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
    if (digitalRead(PB1_DRAW) == HIGH) {
        pserial.print('0');
    } else {
        pserial.print('1');
    }
    pserial.print(' ');
    if (digitalRead(PB2_MANUAL) == HIGH) {
        pserial.print('0');
    } else {
        pserial.print('1');
    }
    pserial.print(' ');
    if (digitalRead(PB3) == HIGH) {
        pserial.print('0');
    } else {
        pserial.print('1');
    }
    pserial.print(' ');
    if (digitalRead(PB4) == HIGH) {
        pserial.print('0');
    } else {
        pserial.print('1');
    }
    pserial.print(' ');
    if (digitalRead(PB5) == HIGH) {
        pserial.print('0');
    } else {
        pserial.print('1');
    }
    pserial.println(';');
#endif

    // Read Size pot
    size_total -= size_samp[samples_i];
    size_samp[samples_i] = map(analogRead(POT4_SIZE), 0, 1023, 0, H/2);
    size_total += size_samp[samples_i];

    // Read X pot
    x_total -= x_samp[samples_i];
    x_samp[samples_i] = map(analogRead(POT1_POSX), 0, 1023, new_size, W-new_size);
    x_total += x_samp[samples_i];

    // Read Y pot
    y_total -= y_samp[samples_i];
    y_samp[samples_i] = map(analogRead(POT2_POSY), 0, 1023, new_size, H-new_size);
    y_total += y_samp[samples_i];

    // Increment samples index
    samples_i++;
    if (samples_i >= SAMPLES) samples_i = 0;

    // Calculate averages from totals
    x_avg = x_total / SAMPLES;
    y_avg = y_total / SAMPLES;
    size_avg = size_total / SAMPLES;

    pulse_ctr++;
    if (pulse_ctr > pulse_dur) {
        //pulse_dur = map(analogRead(POT6_PDUR), 0, 1023, 1, 10) * 100;
        pulse_ctr = 0;
        //draw = false;
        tv.clear_screen();
    }

    // check if data has been sent from the computer:
    if (pserial.available()) {
        // read the most recent byte (which will be from 0 to 255):
        int val = pserial.read();
        oldStep = step;
        step = val;
        tick = (val == 0);
        if (tick) draw = true;
    } else {
        tick = false;
    }

    if (draw) {
        int val = analogRead(POT3_ANGLE);
        if (val < 341) {
            new_shape = 0;
        } else if (val < 682) {
            new_shape = 1;
        } else {
            new_shape = 2;
        }
        //new_shape = (analogRead(POT3_ANGLE) < 512) ? 0 : 1;
        new_size = size_avg;
        new_x = x_avg;
        new_y = y_avg;

        if (tick || new_shape != shape || new_size != size || new_x != x || new_y != y) {
            shape = new_shape;
            size = new_size;
            x = new_x;
            y = new_y;

            tv.clear_screen();

            switch (shape) {
                case 0:
                    tv.draw_line(x-size, y+size, x+size, y+size, WHITE);
                    tv.draw_line(x+size, y+size, x, y-size, WHITE);
                    tv.draw_line(x, y-size, x-size, y+size, WHITE);
                    break;
                case 1:
                    tv.draw_rect(x-size, y-size, size*2, size*2, WHITE);
                    break;
                case 2:
                    tv.draw_circle(x, y, size, WHITE);
                    break;
/*
                case 2:
                    switch (step) {
                        case 0:
                            tv.draw_line(x-size, y-size, x-size, y+size, WHITE);
                            break;
                        case 1:
                            tv.draw_line(x-size, y+size, x+size, y+size, WHITE);
                            break;
                        case 2:
                            tv.draw_line(x+size, y+size, x+size, y-size, WHITE);
                            break;
                        case 3:
                            tv.draw_line(x-size, y-size, x+size, y-size, WHITE);
                            break;
                    }
                    break;
*/
            }
        }
    }
}
