---
layout: default
title: Deep Sleep Mode
nav_order: 9
date: 2020-03-22
permalink: /deep-sleep-mode/
---

{% include header.md %}


## Deep Sleep Mode

Deep sleep mode of ESP32 is very efficient and consumes very less power when powered from battery sources. In this mode, the CPU, most of the RAM and all the digital peripherals are powered off. The only parts of the chip that remains powered on are: RTC controller, RTC peripherals (including ULP co-processor), and RTC memories (slow and fast).

![ESP32 in deep sleep mode]({{ site.baseurl }}/images/esp32-deep-sleep.png){: width="400" }
{: .img-center .with-caption .no-bottom-margin }

Deep Sleep Mode
{: .caption }

Let's take the code from the previous chapter and add support for deep sleep mode:

```cpp
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
```

Now simply modify the main loop to put the ESP32 into deep sleep when `sleepButton` is released:

```cpp
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
        deepSleep();      // <-- replace lightSleep() by deepSleep()
        flashActiveLED(); // flashes the active LED at wake up
    }

    waitForNextCycle();
}
```

Compile, upload the new program and see what happens:

<div class="video-wrapper">
    <video class="video shadow" autoplay muted loop>
    <source src="{{ site. baseurl }}/videos/demo-deep-sleep.mp4" type="video/mp4" />
    Your browser does not support the video tag.
    </video>
</div>

- When the ESP32 wakes up, the active LED is always the first LED of the ramp.
- The active LED stops flashing immediately after waking up.

What the hell happened? You don't get the same behavior at all as after a light sleep.
<i class="far fa-meh"></i>

During deep sleep mode, the main CPU is powered down, while the ULP coprocessor remains receptive to the timers or can perform sensor measurements and wakes up the main system when a timer is triggered, or based on the data measured by the sensors. This sleep pattern is known as **ULP sensor-monitored pattern**.

Along with the CPU, the main memory of the chip is also disabled. So, everything stored in that memory is wiped out and cannot be accessed. This is the reason why, after waking up, the active LED is that of index 0 (the red LED), regardless of the `ledIndex` value before going into deep sleep. The current value of `ledIndex`, which is stored in RAM, was not retained after the sleep phase.

However, the **RTC memory** is kept powered on. So, its content is preserved during deep sleep and can be retrieved after we wake the chip up. That’s the reason, for example, the chip stores Wi-Fi and Bluetooth connection data in RTC memory before disabling them. And in the same way, we need to store the `ledIndex` value in the RTC memory to preserve it during deep sleep.

So, if you want to use the data over reboot, store it into the RTC memory by defining a global variable with `RTC_DATA_ATTR` attribute:

```cpp
// ----------------------------------------------------------------------------
// Definition of LED properties
// ----------------------------------------------------------------------------

const gpio_num_t LED_PINS[] = { GPIO_NUM_27, GPIO_NUM_25, GPIO_NUM_32 };
const uint8_t    LED_NUMBER = 3;

// defines the index of the active LED:
//
// - this statement is enough to go into light sleep
// uint8_t ledIndex = 0;
//
// - you have to save the data in the RTC memory when you go into deep sleep
RTC_DATA_ATTR uint8_t ledIndex = 0;
```

Now, again, compile, upload the new program and see what happens:

<div class="video-wrapper">
    <video class="video shadow" autoplay muted loop>
    <source src="{{ site. baseurl }}/videos/demo-deep-sleep-rtc.mp4" type="video/mp4" />
    Your browser does not support the video tag.
    </video>
</div>

Okay, that's much better. However, when the chip wakes up, the active LED still doesn't blink.
<i class="far fa-meh"></i>

In deep sleep mode, power is shut off to the entire chip except RTC module. So, any data that is not in the RTC recovery memory is lost, and the chip will thus **restart with a reset**. This means program execution starts from the beginning once again. This is the reason why the flashing routine is never reached.

```cpp
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
        deepSleep();

        // this part of the code is never reached after a deep sleep!

        flashActiveLED(); // flashes the active LED only after a light sleep
    }

    waitForNextCycle();
}
```

When the ESP32 wakes up, it reboots and the program execution restarts from the beginning. Therefore, if you wish to make the active LED blink after the reboot, you have to call the `flashActiveLED()` function within the `setup()` function, making sure that it is only executed when the microcontroller comes out of a deep sleep :

```cpp
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
```

Note that it is necessary here to make a forward declaration of the `flashActiveLed()` function just **before** the `setup()` function, because its definition is specified **after** the `setup()` function.

Compile and upload this new version of the program, and this time you can see that we are getting the desired behavior:

<div class="video-wrapper">
    <video class="video shadow" autoplay muted loop>
    <source src="{{ site. baseurl }}/videos/demo-deep-sleep-flashing.mp4" type="video/mp4" />
    Your browser does not support the video tag.
    </video>
</div>


## The Resulting Code

At this point, your code should conform to this:

```cpp
// ----------------------------------------------------------------------------
// Definition of LED properties
// ----------------------------------------------------------------------------

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
```


## To Go One Step Further

Now I would just like to draw your attention to an additional possibility concerning the waking phase after a deep sleep. This possibility is rarely discussed, but is nevertheless [documented by Espressif][wakeup-stub].

<div class="important" markdown="1">
ESP32 supports running a “deep sleep wake stub” when coming out of deep sleep. This function runs immediately as soon as the chip wakes up - before any normal initialisation, bootloader, or ESP-IDF code has run. After the wake stub runs, the SoC can go back to sleep or continue to start ESP-IDF normally.

Deep sleep wake stub code is loaded into “RTC Fast Memory” and any data which it uses must also be loaded into RTC memory. RTC memory regions hold their contents during deep sleep.
</div>

The wake stub in ESP-IDF is called `esp_wake_deep_sleep()`. This function runs whenever the SoC wakes from deep sleep. There is a default version of this function provided in ESP-IDF, but the default function is weak-linked so if your app contains a function named `esp_wake_deep_sleep()` then this will override the default.

If supplying a custom wake stub, the first thing it does should be to call the predefined `esp_default_wake_deep_sleep()` function.

It is not necessary to implement `esp_wake_deep_sleep()` in your app in order to use deep sleep. It is only necessary if you want to have special behavior immediately on wake.

Wake stub code must be resident in RTC Fast Memory. Without going into too much detail, here's a simple way to do it:

```cpp
void RTC_IRAM_ATTR esp_wake_deep_sleep() {
    esp_default_wake_deep_sleep();
    // add additional functionality here
}
```

You have to use the `RTC_IRAM_ATTR` attribute to place the function into RTC memory.

We could therefore, for example, define the following routine:

```cpp
void RTC_IRAM_ATTR esp_wake_deep_sleep() {
    esp_default_wake_deep_sleep();
    // this routine is not necessarily of interest here,
    // this is just an example to illustrate my point.
    ledIndex++;
}
```

Indeed, this routine will work because the `ledIndex` variable is actually hosted in the RTC memory.

If you wanted to define your own wake stub, it is also possible to do this by calling the `esp_set_deep_sleep_wake_stub()` function by passing a reference to your custom function as an argument:

```cpp
void RTC_IRAM_ATTR wake_stub() {
    esp_default_wake_deep_sleep();
    ledIndex++;
}

void deepSleep() {
    esp_sleep_enable_timer_wakeup(SLEEP_DURATION);
    esp_set_deep_sleep_wake_stub(&wake_stub);
    esp_deep_sleep_start();
}
```

There are quite a few subtleties in the definition and use of these wake stubs, but I refer you to the [Espressif's documentation][wakeup-stub] which will bring you the details.

For the time being, I suggest you now switch to the last sleep mode: hibernation.


[Light Sleep Mode]({{ site.baseurl }}/light-sleep-mode/){: .btn }
[Hibernation Mode]({{ site.baseurl }}/hibernation-mode/){: .btn .btn-purple }
{: .navigator }


[wakeup-stub]: https://docs.espressif.com/projects/esp-idf/en/latest/api-guides/deep-sleep-stub.html