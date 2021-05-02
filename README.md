# SofronioApolloScale
Yet another smart(sort of) espresso coffee scale.<br />
## Background information
Acaia produces excellent coffee scales, especially their espresso scale, aka. Acaia Lunar. It features some very useful functions, such as:
* Negative weight auto tare
* Positive weight auto tare and auto start timer
* Auto stop timer after last drip
As a heavy espresso drinker, and a new-bie arduino user, I want to use some cheap scales, and find a way to modify them into an Acaia-like espreeso scale.<br />
## Hardware list
* MCU: Arduino Nano
* Loadcell: Comes with the scale
* ADC:HX711 @10Hz
* Display: SSD1306 OLED IIC
* Battery: A Lithium battery which fits inside, or...
* Battery(alternative): Stock AA battery, with 3v to 5v converter.
## Liberaries used
* HX711_ADC https://github.com/olkal/HX711_ADC
* Adafruit_SSD1306 https://github.com/adafruit/Adafruit_SSD1306
* Stopwatch_RT https://github.com/RobTillaart/Stopwatch_RT
