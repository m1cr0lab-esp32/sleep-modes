---
layout: default
title: Overview
nav_order: 1
date: 2020-03-23
permalink: /
---

{% include header.md %}

The other day, I was reading a discussion on the RNT Lab forum [about ESP32 sleep modes][origin]. And it seemed to me that things were not very clear, especially about the light sleep mode. So I jumped at the opportunity to do some tests of my own, which I'm sharing with you today to try to clear up the fog.
{: .fs-6 .fw-300 .text-justify }


## Why Is This Tutorial Of Interest To You?

My initial intention was to do some experimentation with the different sleep modes of the ESP32. But as I set up the circuit that I'm going to propose to you, it appeared to me that I could take advantage of it to detail a lot of other things. And notably by taking care to address issues I've encountered in other [RNT Lab][rntlab] posts over the last few weeks.

So I guess that even if you're not primarily interested in sleep modes, you'll find a lot of interesting things in this tutorial that you can use in many other applications. The topics covered appear in the side menu . Take a look at it, I'm sure you'll find something to suit your needs.

As a result, this tutorial is finally quite dense, as I tried to detail all the points I covered to be as clear as possible. I hope that I succeeded and that you will be able to apply these concepts in your own projects. I would also like to take this opportunity to point out that I will be happy to answer any questions you may have. Please leave me a message [on the thread dedicated to this tutorial][discussion], on the RNT Lab forum.


## Overview Of The Experiment

To illustrate my point, I chose to implement a rather simple circuit:

- We will control a 3-LED ramp, which we will be able to switch on in turn with a push button. Each press of the button will increment an `ledIndex` counter, which will indicate the rank of LED to be lit, so `ledIndex` will take its values in the [0,2] range cyclically:

  - 0 for the red LED
  - 1 for the yellow LED
  - 2 for the green LED
  - then we go back to 0.

- A second push button will put the ESP32 to sleep for a few seconds and we'll see which LED lights up when it wakes up. In other words, we'll see if the `ledIndex` value is well saved in memory during the sleep phase, and how to make it so if it is not, depending on the sleep mode chosen.

![Breadboard]({{ site.baseurl }}/images/breadboard.jpg){: width="680" .shadow }
{: .img-center }

I propose, through this relatively simple exercise, to explore some programming techniques that you can reproduce in your own projects, to properly manage buttons, or to control the time flow and regulate the frequency of execution of the main loop, or make your microcontroller able to perform several tasks simultaneously.


## Preliminary Readings

To fully understand what we're doing here, I suggest you take a look at the following documents:

- [ESP32 Timer Wake Up from Deep Sleep][rnt-wakeup] *(Random Nerd Tutorials)*
- [ESP32 Sleep Modes][esp32-sm] *(Espressif)*
- [ESP32 Datasheet][esp32-ds] *(Espressif)*


[Parts and Wiring]({{ site.baseurl }}/parts-and-wiring/){: .btn .btn-purple }
{: .navigator }


[origin]:     https://rntlab.com/question/how-to-use-esp32-light-sleep/
[rnt]:        https://randomnerdtutorials.com/
[rntlab]:     https://rntlab.com/forum/
[discussion]: https://rntlab.com/question/
[rnt-wakeup]: https://randomnerdtutorials.com/esp32-timer-wake-up-deep-sleep/
[esp32-sm]:   https://docs.espressif.com/projects/esp-idf/en/latest/api-reference/system/sleep_modes.html
[esp32-ds]:   https://www.espressif.com/sites/default/files/documentation/esp32_datasheet_en.pdf