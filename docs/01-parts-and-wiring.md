---
layout: default
title: Parts and Wiring
nav_order: 2
date: 2020-03-17
permalink: /parts-and-wiring/
---

{% include header.md %}


## Required Parts

To follow this tutorial you need the following parts:

- 1 [ESP32][parts-esp32]
- 5 [resistors][parts-res] (1 x 220 Ω, 2 x 100 Ω and 2 x 10 kΩ)
- 3 [LEDs][parts-led]  
    I used a red, a yellow and a green one, but you can use the LEDs of your choice. You'll just have to think about adapting the value of the protection resistors that you'll add to them in series.
- 2 [push buttons][parts-btn]
- 2 [breadboards][parts-bb]
- [Jumper wires][parts-wire1] (male to male) or a *"hook-up"* [wire spool set][parts-wire2] (22 AWG)
- a [wire stripper][parts-strip] (in case you use spools of wire)


## Wiring Diagram

![Wiring diagram]({{ site.baseurl }}/images/wiring.png){: width="459" }
{: .img-center }

Be sure to use the following pin assignment:

- **GPIO27** is connected to the anode of the red LED
- **GPIO25** is connected to the anode of the yellow LED
- **GPIO32** is connected to the anode of the green LED
- **GPIO4** is connected to the ground line of the left button (LED shifter)
- **GPIO2** is connected to the ground line of the right button (sleep trigger)


## Use Of Pull-Down Resistors

You can notice that we've fitted each button with a *pull-down resistor*. Let's see why...

The digital pins to which the buttons are connected (GPIO2 and GPIO4) must always be able to read a HIGH or LOW signal. And this signal must be able to be inverted depending on whether or not the corresponding button is pressed. Above all, however, care must be taken to ensure that this signal is not floating, otherwise the behaviour of the circuit may become erratic. And this is precisely the role of a pull-up or pull-down resistor.

A pull-up resistor is used to keep the pin in a default HIGH logic state by connecting it to the 3.3V supply line. A pull-down resistor does exactly the opposite and keeps the pin in a default LOW logic state by connecting it to ground. It is important to specify that this is not a specific type of resistor: it is a common resistor. Its place in the circuit gives it this name because of the function it performs.

I preferred to use pull-down resistors here. In this way, the button status reading pin is kept in a default LOW state and, when the button is pressed, its state will change to HIGH.

![Pull-down resistor]({{ site.baseurl }}/images/pull-down-button.png){: width="517" }
{: .img-center }

In such case, the small amount of current flows from the 3.3V source to the ground using the closed switch and pull-down resistor, hence preventing the logic level pin to getting shorted with the 3.3V source.

[Overview]({{ site.baseurl }}/){: .btn }
[LED Setup]({{ site.baseurl }}/led-setup/){: .btn .btn-purple }
{: .navigator }


[parts-esp32]: https://makeradvisor.com/tools/esp32-dev-board-wi-fi-bluetooth/
[parts-res]:   https://makeradvisor.com/tools/resistors-kits/
[parts-led]:   https://makeradvisor.com/tools/3mm-5mm-leds-kit-storage-box/
[parts-btn]:   https://makeradvisor.com/tools/pushbuttons-kit/
[parts-bb]:    https://makeradvisor.com/tools/mb-102-solderless-breadboard-830-points/
[parts-wire1]: https://makeradvisor.com/tools/jumper-wires-kit-120-pieces/
[parts-wire2]: https://thepihut.com/products/prototyping-wire-spool-set
[parts-strip]: https://makeradvisor.com/best-self-adjusting-wire-stripper/