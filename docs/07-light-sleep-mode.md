---
layout: default
title: Light Sleep Mode
nav_order: 8
date: 2020-03-21
permalink: /light-sleep-mode/
---

{% include header.md %}


## Light Sleep Mode

In this mode, digital peripherals, most of the RAM, and CPUs are clock-gated. When light sleep mode exits, peripherals and CPUs resume operation, their internal state is preserved. Clock gating is a technique to save power consumption in digital circuitry by disabling the clock pulses to flip-flops, which in turn disables the switching states. As switching states consume a lot of power, hence if you disable it, we can save a lot of power.

![ESP32 in light sleep mode]({{ site.baseurl }}/images/esp32-light-sleep.png){: width="400" }
{: .img-center .with-caption .no-bottom-margin }

Light Sleep Mode
{: .caption }

Now, just before the `waitForNextCycle()` function, let's define the `lightSleep()` function that will put the ESP32 into sleep for `SLEEP_DURATION` microseconds:

```cpp
// ----------------------------------------------------------------------------
// Microcontroller sleep modes
// ----------------------------------------------------------------------------

void lightSleep() {
    esp_sleep_enable_timer_wakeup(SLEEP_DURATION);
    esp_light_sleep_start();
}
```

Well, now we've got everything we need to put the microcontroller to sleep. And we're going to do it with our second button (the one on the right), the `sleepButton`:

```cpp
// ----------------------------------------------------------------------------
// Main control loop
// ----------------------------------------------------------------------------

void loop() {
    readButton(&shiftButton);
    readButton(&sleepButton);     // <-- add this line

    if (pressed(&shiftButton)) {
        ++ledIndex %= LED_NUMBER;
        updateLED();
    }

    if (released(&sleepButton)) { //
        lightSleep();             // <-- and this block
    }                             //

    waitForNextCycle();
}
```

You can see that we need to add to the loop the routine that reads the state of the `sleepButton`. And this time, we wait until `sleepButton` is **released** before triggering the call to the `lightSleep()` function.

All right, let's see how it goes. Compile and upload the program to your board... you should get this:

<div class="video-wrapper">
    <video class="video shadow" autoplay muted loop>
    <source src="{{ site. baseurl }}/videos/demo-light-sleep.mp4" type="video/mp4" />
    Your browser does not support the video tag.
    </video>
</div>

Everything's working perfectly. The LEDs go out when the ESP32 enters the sleep phase. The small red LED on the board does not go out because it is not directly connected to the microcontroller circuit. It is simply a power LED.

On the other hand, you will probably have noticed that when you press the `shiftButton`, and as long as you hold it down, a small blue LED also lights up on the board. It's perfectly normal. Remember that the reading pin of this button is the GPIO2 pin. This pin happens to be directly connected to the blue LED on the board. Therefore, when the button is pressed, and as long as it is held in this state, a HIGH logic signal is sent on the pin, so the blue LED is powered with 3.3V and therefore it lights up. As soon as the button is released, the signal returns to LOW and the LED is no longer powered, so it goes out.

Well, now I'd like to focus on a crucial point regarding the light sleep mode. I would like to ensure that when the ESP32 wakes up, the active LED (the one designated by `ledIndex`) starts flashing briefly before lighting up permanently. To do this, let's define a `flashActiveLED` function to make the active LED flash:

```cpp
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
```

I see you coming...  
*"ROFL, he used a delay!"*{: style="color:#a00;" }
{: .caption style="margin-bottom:.5em;" }

<i class="far fa-grin-squint-tears" style="color:#a00;"></i>
{: .caption style="font-size:1.5em;line-height:1em;" }

I didn't say it was forbidden to use the `delay()` function... just that it was better to avoid it when you want to be able to run several tasks simultaneously. But in this case, its use is entirely appropriate. Indeed, when the ESP32 wakes up, you want to make the LED blink, without anything to interrupt this behaviour. Once the flashing is completed, the program resumes its normal course.

Then let's call this function right after the directive that puts the chip to light sleep:

```cpp
void loop() {
    readButton(&shiftButton);
    readButton(&sleepButton);

    if (pressed(&shiftButton)) {
        ++ledIndex %= LED_NUMBER;
        updateLED();
    }

    if (released(&sleepButton)) {
        lightSleep();
        flashActiveLED(); // <-- add this to make the active LED blink for a short time
    }

    waitForNextCycle();
}
```

Compile and upload this new program and let's see what happens:

<div class="video-wrapper">
    <video class="video shadow" autoplay muted loop>
    <source src="{{ site. baseurl }}/videos/demo-light-sleep-flashing.mp4" type="video/mp4" />
    Your browser does not support the video tag.
    </video>
</div>

Perfect! It's exactly what we expected! But if you take a closer look, you need to understand something essential that is unique to the light sleep mode.

```cpp
if (released(&sleepButton)) {
    // ESP32 is going to sleep
    lightSleep();

    // then... it's sleeping

    // when it's time to wake up, the program resumes here as if nothing had happened.
    flashActiveLED();
}
```

So it's as if the microcontroller had taken a little nap when the `lightSleep()` function was called... and then simply woke up to pick up his work where he left off. Well, that's exactly what happens in light sleep mode. And that's what makes it special compared to the deep sleep and hibernation modes.

This behaviour is a direct result of the clock gating mechanism. Before entering light sleep mode, the ESP32 preserves its internal state and resumes operation upon exit from the sleep. It is known **Full RAM Retention**.


## The Resulting Code

Here's the complete code you should have by now:

```cpp
// ----------------------------------------------------------------------------
// Definition of LED properties
// ----------------------------------------------------------------------------

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
        lightSleep();
        flashActiveLED(); // flashes the active LED at wake up
    }

    waitForNextCycle();
}
```

Okay, now let's turn to deep sleep mode...


[Sleep Modes]({{ site.baseurl }}/sleep-modes/){: .btn }
[Deep Sleep Mode]({{ site.baseurl }}/deep-sleep-mode/){: .btn .btn-purple }
{: .navigator }