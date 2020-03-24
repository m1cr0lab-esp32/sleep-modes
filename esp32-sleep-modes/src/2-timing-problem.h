/**
 * ----------------------------------------------------------------------------
 * Experimenting with ESP32 sleep modes
 * ----------------------------------------------------------------------------
 * Please visit the tutorial:
 * @see https://iw4rr10r.github.io/esp32-sleep-modes/
 * ----------------------------------------------------------------------------
 * © 2020 Stéphane Calderoni
 * ----------------------------------------------------------------------------
 * Timing Problem | High Frequency Of The Main Loop
 * ----------------------------------------------------------------------------
 */

#include <Arduino.h>

uint32_t counter;
uint32_t last;

void setup() {
    Serial.begin(115200);
    delay(500);
    counter = 0;
    last    = 0;
}

void loop() {
    counter++;
    if (micros() % 1000000 == 0) {
        Serial.printf("counter: %10u --> %10u => frequency: %10u\n", last, counter, counter - last);
        last = counter;
    }
}