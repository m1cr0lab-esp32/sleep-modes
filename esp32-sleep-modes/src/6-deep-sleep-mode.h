/**
 * ----------------------------------------------------------------------------
 * Experimenting with ESP32 sleep modes
 * ----------------------------------------------------------------------------
 * Please visit the tutorial:
 * @see https://m1cr0lab-esp32.github.io/sleep-modes/
 * ----------------------------------------------------------------------------
 * © 2020 Stéphane Calderoni
 * ----------------------------------------------------------------------------
 * Deep Sleep Mode
 * ----------------------------------------------------------------------------
 */

#include <Arduino.h>

// ----------------------------------------------------------------------------
// Definition of LED properties
// ----------------------------------------------------------------------------
//                                  RED        YELLOW        GREEN
//                                   0            1            2
const gpio_num_t LED_PINS[] = { GPIO_NUM_27, GPIO_NUM_25, GPIO_NUM_32 };
const uint8_t    LED_NUMBER = 3;

// defines the index of the active LED:
//
// - this statement is enough to go into light sleep
// uint8_t ledIndex = 0;
//
// - you have to save the data in the RTC memory when you go into deep sleep
RTC_DATA_ATTR uint8_t ledIndex = 0;

// ----------------------------------------------------------------------------
// Definition of time control parameters
// ----------------------------------------------------------------------------

const uint16_t LOOP_FREQUENCY = 25;                    // Hz
const uint16_t WAIT_PERIOD    = 1000 / LOOP_FREQUENCY; // ms

struct Timer {
    uint32_t laptime;
    uint32_t ticks;
};

Timer timer;

// ----------------------------------------------------------------------------
// Definition of button properties
// ----------------------------------------------------------------------------

struct Button {
    gpio_num_t pin;
    uint8_t    state;
};

Button shiftButton = { GPIO_NUM_4, 0 };
Button sleepButton = { GPIO_NUM_2, 0 };

// ----------------------------------------------------------------------------
// Definition of sleep mode properties
// ----------------------------------------------------------------------------
//                           seconds
//                              v
const uint32_t SLEEP_DURATION = 4 * 1000000; // µs

// ----------------------------------------------------------------------------
// Initialization
// ----------------------------------------------------------------------------

// forward declaration
void flashActiveLED();

void setup() {
    // configure the button pins
    pinMode(shiftButton.pin, INPUT);
    pinMode(sleepButton.pin, INPUT);

    // configures the LED pins
    for (uint8_t i=0; i<LED_NUMBER; i++) {
        pinMode(LED_PINS[i], OUTPUT);
    }

    // turns on the active LED
    digitalWrite(LED_PINS[ledIndex], HIGH);

    // initializes the timer
    timer = { millis(), 0 };

    // flash the active LED only after a deep sleep
    if (esp_sleep_get_wakeup_cause() == ESP_SLEEP_WAKEUP_TIMER) {
        flashActiveLED();
    }
}

// ----------------------------------------------------------------------------
// Active LED lighting
// ----------------------------------------------------------------------------

void updateLED() {
    for (uint8_t i=0; i<LED_NUMBER; i++) {
        digitalWrite(LED_PINS[i], i == ledIndex ? HIGH : LOW);
    }
}

void flashActiveLED() {
    for (uint8_t i=0; i<=10; i++) {
        digitalWrite(LED_PINS[ledIndex], i % 2 == 0 ? HIGH : LOW);
        delay(100);
    }
}

// ----------------------------------------------------------------------------
// Button status handling
// ----------------------------------------------------------------------------

void readButton(Button *b) {
    bool pressed = digitalRead(b->pin) == HIGH;

    if (pressed) {
             if (b->state < 0xfe) b->state++;
        else if (b->state == 0xfe) b->state = 2;
    } else if (b->state) {
        b->state = b->state == 0xff ? 0 : 0xff;
    }
}

bool pressed(Button *b) {
    return b->state == 1;
}

bool released(Button *b) {
    return b->state == 0xff;
}

bool held(Button *b) {
    return b->state > 1 && b->state < 0xff;
}

// ----------------------------------------------------------------------------
// Microcontroller sleep modes
// ----------------------------------------------------------------------------

void lightSleep() {
    esp_sleep_enable_timer_wakeup(SLEEP_DURATION);
    esp_light_sleep_start();
}

void deepSleep() {
    esp_sleep_enable_timer_wakeup(SLEEP_DURATION);
    esp_deep_sleep_start();
}

// ----------------------------------------------------------------------------
// Time control of the main loop
// ----------------------------------------------------------------------------

void waitForNextCycle() {
    uint32_t now;
    do { now = millis(); } while (now - timer.laptime < WAIT_PERIOD);
    timer.laptime = now;
    timer.ticks++;
}

// ----------------------------------------------------------------------------
// Main control loop
// ----------------------------------------------------------------------------

void loop() {
    readButton(&shiftButton);
    readButton(&sleepButton);

    if (pressed(&shiftButton)) {
        ++ledIndex %= LED_NUMBER;
        updateLED();
    }

    if (released(&sleepButton)) {
        // set the desired sleep mode from among:
        // lightSleep();
        // deepSleep();
        deepSleep();

        // is performed only after a light sleep
        flashActiveLED();
    }

    waitForNextCycle();
}