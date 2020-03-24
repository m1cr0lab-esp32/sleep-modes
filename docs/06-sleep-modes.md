---
layout: default
title: Sleep Modes
nav_order: 7
date: 2020-03-20
permalink: /sleep-modes/
---

{% include header.md %}


## ESP32 Sleep Modes

[Espressif's documentation][esp32-datasheet] details all the modules that make up the ESP32. All these modules are functional when the microcontroller is in **active mode**:

![ESP32 in active mode]({{ site.baseurl }}/images/esp32-active.png){: width="400" }
{: .img-center .with-caption .no-bottom-margin }

Active Mode
{: .caption }

However, there is the possibility of operating the ESP32 in degraded mode by deactivating some of its modules, and thus making it consume less energy. **Modem sleep** mode, for example, disables all circuits responsible for the operation of the WiFi, Bluetooth and radio modules:

![ESP32 in modem sleep mode]({{ site.baseurl }}/images/esp32-modem-sleep.png){: width="400" }
{: .img-center .with-caption .no-bottom-margin }

Modem Sleep Mode
{: .caption }

But the CPU remains functional, as well as the peripheral modules that allow the microcontroller to communicate with the outside world through its GPIOs. Radio-frequency communication modules consume a lot of power and it can be very useful to deactivate them when they are not in use, especially when the ESP32 is running on battery power.

If we still need to keep WiFi or Bluetooth connections active, then we have to wake the module in between a certain interval, which will switch the module in between active and modem sleep modes. This sleep pattern is called as **Association sleep pattern**.

But it's still possible to go further, by deactivating the processing cores to save even more energy. And that's what we're going to be interested in here. We will explore how the microcontroller behaves in more energy-efficient sleep modes:

- [Light Sleep Mode][light-sleep]
- [Deep Sleep Mode][deep-sleep]
- [Hibernation Mode][hibernation]

However, from the moment you put the ESP32 into sleep, you also have to ask yourself how to **wake it up**! Rui & Sara have published comprehensive tutorials on the different possibilities that exist to wake up the ESP32. And I encourage you to study them in order to have a clear vision of what it is possible to do:

- [Complete Guide on ESP32 Deep Sleep][rnt-deep-sleep]
- [ESP32 Timer Wake Up from Deep Sleep][rnt-timer-wakeup]
- [ESP32 External Wake Up from Deep Sleep][rnt-ext-wakeup]
- [ESP32 Touch Wake Up from Deep Sleep][rnt-touch-wakeup]

These tutorials largely cover the deep sleep mode, but we will see together how things happen in the 3 above-mentioned sleep modes. I have also chosen to focus here only on the **timer wake-up**, so as not to complicate things with new devices to manage.

I now propose to take the code we obtained in the previous chapter, and to add the following definition, just after the **Definition of button properties** section:

```cpp
// ----------------------------------------------------------------------------
// Definition of sleep mode properties
// ----------------------------------------------------------------------------
//                           seconds
//                              v
const uint32_t SLEEP_DURATION = 4 * 1000000; // µs
```

This constant sets the sleep time to 4 seconds. Indeed, when we invoke the appropriate function to put ESP32 to sleep with a timer wake-up, we will need to specify the sleep duration. This duration must be expressed in microseconds.


## The Resulting Code

Here's the complete code you should have by now:

```cpp
// ----------------------------------------------------------------------------
// Definition of LED properties
// ----------------------------------------------------------------------------
//                                  RED        YELLOW        GREEN
//                                   0            1            2
const gpio_num_t LED_PINS[] = { GPIO_NUM_27, GPIO_NUM_25, GPIO_NUM_32 };
const uint8_t    LED_NUMBER = 3;

// defines the index of the active LED
uint8_t ledIndex = 0;

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
}

// ----------------------------------------------------------------------------
// Active LED lighting
// ----------------------------------------------------------------------------

void updateLED() {
    for (uint8_t i=0; i<LED_NUMBER; i++) {
        digitalWrite(LED_PINS[i], i == ledIndex ? HIGH : LOW);
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

    if (pressed(&shiftButton)) {
        ++ledIndex %= LED_NUMBER;
        updateLED();
    }

    waitForNextCycle();
}
```

Come on, let's start by experimenting with the light sleep mode...


[Buttons handling]({{ site.baseurl }}/buttons-handling/){: .btn }
[Light Sleep Mode]({{ site.baseurl }}/light-sleep-mode/){: .btn .btn-purple }
{: .navigator }


[esp32-datasheet]:  https://www.espressif.com/sites/default/files/documentation/esp32_datasheet_en.pdf
[light-sleep]:      {{ site.baseurl }}/light-sleep-mode/
[deep-sleep]:       {{ site.baseurl }}/deep-sleep-mode/
[hibernation]:      {{ site.baseurl }}/hibernation-mode/
[rnt-deep-sleep]:   https://randomnerdtutorials.com/esp32-deep-sleep-arduino-ide-wake-up-sources/
[rnt-timer-wakeup]: https://randomnerdtutorials.com/esp32-timer-wake-up-deep-sleep/
[rnt-ext-wakeup]:   https://randomnerdtutorials.com/esp32-external-wake-up-deep-sleep/
[rnt-touch-wakeup]: https://randomnerdtutorials.com/esp32-touch-wake-up-deep-sleep/