# SofronioApolloScale
Yet another smart(sort of) espresso coffee scale, modded dirty but cheap.<br />
## Background information
Acaia produces excellent coffee scales, especially their espresso scale, aka. Acaia Lunar. It features some very useful functions, such as:
* Negative weight auto tare
* Positive weight auto tare and auto start timer
* Auto stop timer after last drip
* ...
As a heavy espresso drinker, and a new-bie arduino user, I want to use some cheap scales, and find a way to modify them into an Acaia-like espreeso scale.<br />
## Research
I bought nearly every coffee scale under 200 RMB(￥) or about 30 USD($). I took them apart, did some research, and found the following information:
* All scales share one same component called Loadcell. Loadcell usually has 4 wires, which should be in red, black, green and white.
* The Loadcell does nothing with reading speed.
* The Loadcell dees little with reading accuricy. (in coffee making)
* Some large scales don't have much space inside, and supprisingly, some quite compact scales do.(good for mod)

## Hardware list
* MCU: Arduino Nano (￥21.6 $3.34)
* Loadcell: Comes with the scale (￥15.3 $2.36)
* ADC:HX711 @10Hz (￥2.45 $0.38)
* Display: 1.3 inch SSD1306 OLED IIC (￥22.5 $3.48)
* Battery: A lithium battery which fits inside 200mAh(type 402030) (￥10.8 $1.67)
* 3.7v to 5V converter(for lithium battery, with charge function) (￥3.3 $0.51)
* Battery(if you want to use the original battery bay): Any stock AA/AAA battery comes with the scale
* 1.53with 3v to 5v converter. (￥2.1 $0.32)
* Total cost（~￥75.95 ~$11.73）which is significantly less than an Acaia.

## Liberaries used
* HX711_ADC https://github.com/olkal/HX711_ADC
* Adafruit_SSD1306 https://github.com/adafruit/Adafruit_SSD1306
* Stopwatch_RT https://github.com/RobTillaart/Stopwatch_RT

## Fucntion achieved
* Negative weight auto tare
* Positive weight auto tare and auto start timer
* Auto stop timer after last drip
* Brew ratio(now only fixed as 20g coffee, which I usually use)

## Fucntion to do
* Button support
* Low power mode(however, I want to use an On/Off switch to save the fuzz)
* Coffee bean/ground holder cup memerize(by weight/by nfc)
* Espresso machine wireless control(phycicle push button/use relay)

## Function won't be done
* Bluetooth support for smartphone
which I still don't get the point to use phone during coffee making, I used the Acaia Pearl S and Acaia Lunar's bluetooth app for the first week, then I'd never use my phone to record any data.
