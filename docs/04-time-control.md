---
layout: default
title: Time Control
nav_order: 5
date: 2020-03-18
permalink: /time-control/
---

{% include header.md %}


## Timing Regulation Of The Main Loop

In this chapter we will see how to regulate the timing during the execution of the program. That is to say, make sure that all programmed tasks can be completed, but without letting the main loop get out of control and run at full speed if not necessary.

To achieve this, we will make sure that we can fully control the frequency of the main loop's execution cycles. A bit like beating the clock by marking the limits of a time window in which everything that needs to be done has to be done, but nothing more. And if you've completed the tasks that needed to be done during the cycle faster than expected, then just wait for the next one.

In this way it will always be possible to adjust the frequency of execution to the program requirements, without changing the entire program structure.

Let's go back to our initial program which simply animates a ramp of LEDs:

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
// Initialization
// ----------------------------------------------------------------------------

void setup() {
    // configures the LED pins
    for (uint8_t i=0; i<LED_NUMBER; i++) {
        pinMode(LED_PINS[i], OUTPUT);
    }

    // turns on the active LED
    digitalWrite(LED_PINS[ledIndex], HIGH);
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
// Main control loop
// ----------------------------------------------------------------------------

void loop() {
    updateLED();
    ++ledIndex %= LED_NUMBER;
    delay(200);
}
```


### Setting Up

Just before **Initialization** section, let's add this piece of code:

```cpp
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
```

Here we have set a `LOOP_FREQUENCY` of 25 Hz, i.e. the main loop will run 25 times per second. You can see that we are very far from the frequency level observed in the previous chapter, which exceeded 1 MHz (one million cycles per second). And yet, you will soon see that it is more than enough.

`WAIT_PERIOD` corresponds to the period of a cycle, i.e. its duration, expressed here in milliseconds. Thus, for a frequency of 25 Hz, we obtain a period of 1000 / 25 = 40 ms. In other words, the microcontroller will have 40 ms to complete all the tasks it has to do before the next cycle. And that is more than enough time, considering the time needed to complete each of the tasks we will program afterwards.

Next, we defined a `timer` variable as a `Timer` data type, which is composed of 2 state variables:

- `laptime` to save the time (in milliseconds) when the current cycle start.
- `ticks` to count the number of cycles performed since the start of the program.

We must initialize the `timer` variable in the `setup()` function:

```cpp
// ----------------------------------------------------------------------------
// Initialization
// ----------------------------------------------------------------------------

void setup() {
    // configures the LED pins
    for (uint8_t i=0; i<LED_NUMBER; i++) {
        pinMode(LED_PINS[i], OUTPUT);
    }

    // turns on the active LED
    digitalWrite(LED_PINS[ledIndex], HIGH);

    // initializes the timer
    timer = { millis(), 0 };
}
```

Now let's look at precisely how to regulate the frequency of execution of the main loop.


### Implementing The Regulation

Which peripheral modules does the ESP32 need to communicate with? First of all, there is the LED ramp. Then we will have to handles the buttons, but we will deal with the buttons in a second step.

As far as the LED ramp is concerned, we saw earlier that it is characterized by a `ledIndex` status variable, which indicates which LED must be on. We have also written an `updateLED()` function that updates the physical state of the LEDs, sending a logical signal to each of the LEDs depending on whether it should be on (HIGH) or off (LOW). And the logic level of this signal is directly determined from the value of the `ledIndex` status variable.

To synchronize the animation of the LED ramp on our `timer`, here is how we have to proceed :

```cpp
// ----------------------------------------------------------------------------
// Main control loop
// ----------------------------------------------------------------------------

void loop() {
    if (timer.ticks % 5 == 0) {
        ++ledIndex %= LED_NUMBER;
        updateLED();
    }
    
    // once all the tasks that were to be accomplished
    // in one cycle have been completed, here we need to
    // implement a mechanism to wait for the next cycle.
}
```

Remember that each LED had to stay on for 200 ms in our previous experiment. If we wish to observe the same lighting time here, we have to express it according to the number of cycles elapsed, taking into account their frequency.

Knowing that 200 ms represents 1/5 of a second, and that each cycle is performed 25 times per second, this means that each LED must remain on for 25/5 = 5 cycles. In other words, `ledIndex` must be incremented every 5 cycles.

Nevertheless, we must not write it like this. Imagine doubling the frequency of execution. In one second, 50 cycles will be executed. So since we have written that `ledIndex` value changes every 5 cycles, this event will occur 10 times in one second, instead of 5.

In other words, if you wish to keep an LED on for 200 ms regardless of the cycle frequency, you should write :

```cpp
if (timer.ticks % ((200/1000) * LOOP_FREQUENCY) == 0)
// which is equivalent to:
if (timer.ticks % (LOOP_FREQUENCY / 5) == 0)
// which is equivalent to:
if (5 * timer.ticks % LOOP_FREQUENCY == 0)
```

So the right way to write things is:

```cpp
// ----------------------------------------------------------------------------
// Main control loop
// ----------------------------------------------------------------------------

void loop() {
    if (5 * timer.ticks % LOOP_FREQUENCY == 0) {
        ++ledIndex %= LED_NUMBER;
        updateLED();
    }
    
    // once all the tasks that were to be accomplished
    // in one cycle have been completed, here we need to
    // implement a mechanism to wait for the next cycle.
}
```

In this way, the illumination time of an LED remains independent of the selected frequency.

Well now, all that's left to do is to add the end-of-cycle waiting mechanism. To do this, simply calculate the difference between the current date and the start date of the cycle, and check that it does not exceed the `WAIT_PERIOD` duration of a cycle.

This calculation is performed in a `do {} while();` waiting loop, after all the tasks to be performed in the cycle have been completed. And when the end of a cycle is reached, the start time of the new cycle is saved and the number of cycles performed is incremented:

```cpp
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
    if (5 * timer.ticks % LOOP_FREQUENCY == 0) {
        ++ledIndex %= LED_NUMBER;
        updateLED();
    }
    
    waitForNextCycle();
}
```

That's it! It's finally not very complicated, and now we are able to totally control the execution frequency of the main loop cycles. Now, if we look at what we get for this second experiment:

<div class="video-wrapper">
    <video class="video shadow" autoplay muted loop>
    <source src="{{ site. baseurl }}/videos/demo-leds.mp4" type="video/mp4" />
    Your browser does not support the video tag.
    </video>
</div>

The circuit behaves exactly as in the previous experiment, regardless of the frequency. But the important thing to remember here is that no matter how long an LED lights up, we are able to perform other tasks simultaneously. The microcontroller does not get stuck by a `delay()`.


## The Resulting Code

To conclude this chapter, here is the complete code of this experiment:

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
// Initialization
// ----------------------------------------------------------------------------

void setup() {
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
    if (5 * timer.ticks % LOOP_FREQUENCY == 0) {
        ++ledIndex %= LED_NUMBER;
        updateLED();
    }

    waitForNextCycle();
}
```


[Timing Problem]({{ site.baseurl }}/timing-problem/){: .btn }
[Buttons handling]({{ site.baseurl }}/buttons-handling/){: .btn .btn-purple }
{: .navigator }