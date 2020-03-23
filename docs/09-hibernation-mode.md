---
layout: default
title: HIbernation Mode
nav_order: 10
date: 2020-03-23
permalink: /hibernation-mode/
---

{% include header.md %}


## Hibernation

Unlike deep sleep mode, in hibernation mode the chip disables internal 8-MHz oscillator and ULP-coprocessor as well. The RTC recovery memory is also powered down, meaning there’s no way we can preserve any data during hibernation mode.

Everything else is shut off except only one RTC timer on the slow clock and some RTC GPIOs are active. The RTC timer or the RTC GPIOs can wake up the chip from the Hibernation mode.

This reduces power consumption even further.

![ESP32 in hibernation mode]({{ site.baseurl }}/images/esp32-hibernation.png){: width="400" }
{: .img-caption-center }

Hibernation Mode
{: .caption }

Let's take the code from the previous chapter, and add the `hibernate` function to put the ESP32 in hibernation mode:

```cpp
void hibernate() {
    esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH,   ESP_PD_OPTION_OFF);
    esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_SLOW_MEM, ESP_PD_OPTION_OFF);
    esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_FAST_MEM, ESP_PD_OPTION_OFF);
    esp_sleep_pd_config(ESP_PD_DOMAIN_XTAL,         ESP_PD_OPTION_OFF);
    
    deepSleep();
}
```

Then modify the `loop()` function so that the microcontroller goes into hibernation when the `sleepButton` is released:

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
        // set the desired sleep mode from among:
        // lightSleep();
        // deepSleep();
        // hibernate();
        hibernate();

        // is performed only after a light sleep
        flashActiveLED();
    }

    waitForNextCycle();
}
```

Compile and upload the code and let's see what happens:

<video class="img-center shadow" width="340" height="255" autoplay muted loop>
  <source src="{{ site. baseurl }}/videos/demo-hibernation-nothing.mp4" type="video/mp4" />
  Your browser does not support the video tag.
</video>


Once the 4-second sleep timeout period has elapsed, no LEDs light up and it looks like the ESP32 still hasn't woken up. Actually, there's about a 1.2% chance that an LED will light up... But why is that?

The answer is simple. Remember that the RTC memory is no longer powered. So there is no chance that the `ledIndex` value could have been saved in the memory during the sleep phase. In other words, when ESP32 woke up, the memory address of the `ledIndex` variable could contain any value between 0 and 255, since `ledIndex` is an 8-bit integer. So there were 3 chances out of 256 (3/256 ~ 1.2%) that it would contain a value corresponding to the index of one of the LEDs (0, 1 or 2).

For this reason, there is a good chance that no LEDs will light up when the microcontroller is restarted. Here, the only way to save the `ledIndex` value is to write it into the EEPROM.

If you don't know how to use the EEPROM, I recommend you take a look at the following tutorial:

[ESP32 Flash Memory – Store Permanent Data (Write and Read)][rnt-eeprom]

Now, at the very beginning of the program, add the following line:

```cpp
#include <EEPROM.h>
```

Then change the `hibernate()` function to save `ledIndex` value in the EEPROM before hibernating:

```cpp
void hibernate() {
    // the ledIndex value is saved at address 0x0000
    EEPROM.write(0, ledIndex);
    EEPROM.commit();

    esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH,   ESP_PD_OPTION_OFF);
    esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_SLOW_MEM, ESP_PD_OPTION_OFF);
    esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_FAST_MEM, ESP_PD_OPTION_OFF);
    esp_sleep_pd_config(ESP_PD_DOMAIN_XTAL,         ESP_PD_OPTION_OFF);
    
    deepSleep();
}
```

Finish by modifying the `setup()` function so that `ledIndex` value can be initialized with the value stored in the EEPROM:

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

    // sets the EEPROM desired size
    EEPROM.begin(1); // <-- we only need 1 byte to save ledIndex value

    // flash the active LED only after a deep sleep
    if (esp_sleep_get_wakeup_cause() == ESP_SLEEP_WAKEUP_TIMER) {
        ledIndex = EEPROM.read(0); // <-- the value stored at address 0x0000 is assigned to ledIndex
        flashActiveLED();
    }
}
```

Now test your program again. You should find that this time everything works correctly:

<video class="img-center shadow" width="340" height="255" autoplay muted loop>
  <source src="{{ site. baseurl }}/videos/demo-hibernation-flashing.mp4" type="video/mp4" />
  Your browser does not support the video tag.
</video>


## The Resulting Code

To conclude this chapter, here is the complete code of this experiment:

```cpp
#include <EEPROM.h>

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

    // we only need 1 byte to save ledIndex value
    EEPROM.begin(1);

    // flash the active LED only after a deep sleep
    if (esp_sleep_get_wakeup_cause() == ESP_SLEEP_WAKEUP_TIMER) {
        // the value stored at address 0x0000 is assigned to ledIndex
        ledIndex = EEPROM.read(0);
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

void hibernate() {
    // the ledIndex value is saved at address 0x0000
    EEPROM.write(0, ledIndex);
    EEPROM.commit();

    esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH,   ESP_PD_OPTION_OFF);
    esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_SLOW_MEM, ESP_PD_OPTION_OFF);
    esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_FAST_MEM, ESP_PD_OPTION_OFF);
    esp_sleep_pd_config(ESP_PD_DOMAIN_XTAL,         ESP_PD_OPTION_OFF);
    
    deepSleep();
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
        // hibernate();
        hibernate();

        // is performed only after a light sleep
        flashActiveLED();
    }

    waitForNextCycle();
}
```


[Deep Sleep Mode]({{ site.baseurl }}/deep-sleep-mode/){: .btn }
[Time for discussion]({{ site.baseurl }}/time-for-discussion/){: .btn .btn-purple }
{: .navigator }


[rnt-eeprom]: https://randomnerdtutorials.com/esp32-flash-memory/