---
layout: default
title: Timing Problem
nav_order: 4
date: 2020-03-18
permalink: /timing-problem/
---

{% include header.md %}


## The Pitfalls Of The delay() Function

Let's take the code snippet corresponding to the execution of the main loop:

```cpp
// ----------------------------------------------------------------------------
// Main control loop
// ----------------------------------------------------------------------------

void loop() {
    updateLED();
    ++ledIndex %= LED_NUMBER;
    delay(200);
}
```

Even if this code does the job, the use of the `delay()` function has some annoying drawbacks. Indeed, as its name suggests, it imposes a waiting period on the microcontroller during which it is not supposed to do anything else but... **wait**!

In other words, during this period, the microcontroller is unable to perform any other task, such as reading the value of a signal from a sensor on an input pin, or sending a signal to an output pin to control a peripheral module, or performing mathematical operations, etc.

It doesn't help us at all, because we'll need to read the state of the buttons to be able to control the behavior of the microcontroller.

One possibility would be to limit oneself to very short break times, but this is ultimately not acceptable. If you want to make an LED flash slowly, the pause times would be much too long. Another possibility would be to use interruptions, but their use can be tricky, especially for beginners, and is only possible with very short treatments. In this case, reading the status of simple buttons might be fine, but I would like to show you another way of doing it that does not require the use of interruptions and which has the advantage of being more flexible to handle.


## High Frequency Of The Main Loop

Another aspect may be worth looking into: as things stand, and if no `delay` is used, the execution of the main loop is extremely fast here. In other words, the frequency of its execution cycles is extremely high. Let's do a little test to get an idea.

Compile and upload this new program to your ESP32:

```cpp
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
```

Each `loop()` cycle, a `counter` variable is incremented. And every second, the difference between the current `counter` value and its `last` stored value is calculated (at the previous second). This allows us to deduce how many cycles have taken place in a second, and therefore the frequency of the main loop cycles.

You should get the following trace on the serial monitor:  
*Remember to set the bitrate of the serial monitor to 115200 bps...*{: .text-grey }

```
counter:          0 -->     478617 => frequency:     478617
counter:     478617 -->    1508360 => frequency:    1029743
counter:    1508360 -->    2538313 => frequency:    1029953
counter:    2538313 -->    3568267 => frequency:    1029954
counter:    3568267 -->    4598221 => frequency:    1029954
counter:    4598221 -->    5628175 => frequency:    1029954
counter:    5628175 -->    6658128 => frequency:    1029953
counter:    6658128 -->    7688082 => frequency:    1029954
counter:    7688082 -->    8718036 => frequency:    1029954
counter:    8718036 -->    9747990 => frequency:    1029954
counter:    9747990 -->   10777944 => frequency:    1029954
counter:   10777944 -->   11807898 => frequency:    1029954
counter:   11807898 -->   12837850 => frequency:    1029952
counter:   12837850 -->   13867803 => frequency:    1029953
counter:   13867803 -->   14897756 => frequency:    1029953
counter:   14897756 -->   15927709 => frequency:    1029953
counter:   15927709 -->   16957662 => frequency:    1029953
counter:   16957662 -->   17987615 => frequency:    1029953
counter:   17987615 -->   19017568 => frequency:    1029953
counter:   19017568 -->   20047521 => frequency:    1029953
counter:   20047521 -->   21077474 => frequency:    1029953
counter:   21077474 -->   22107427 => frequency:    1029953
counter:   22107427 -->   23137380 => frequency:    1029953
...
```

As you can see, the cycle frequency is stabilizing at a value well above 1 MHz, which is huge!

A high frequency may indeed be required for some applications. But for today's application, you don't need to have such a high velocity. We will see in the next chapter how to regulate this frequency of execution by adjusting it to our needs. And at the same time, this process will allow us to manage several things at once, without being blocked by the nature of the delay() function.


[LED Setup]({{ site.baseurl }}/led-setup/){: .btn }
[Time Control]({{ site.baseurl }}/time-control/){: .btn .btn-purple }
{: .navigator }