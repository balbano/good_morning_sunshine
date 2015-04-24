Good Morning, Sunshine
======================

*Good Morning, Sunshine* is an electric kettle with no accessible controls. The kettle activates automatically at sunrise. This is a ritual sacrifice. We sacrifice control, we give it back to the sun and the seasons.

The kettle will have an embedded microcontroller with a GPS receiver that activates the kettle on/off switch with a transistor. The GPS receiver tells the arduino both the time and location, which can be used to [calculate the sunrise](http://en.wikipedia.org/wiki/Sunrise_equation#Complete_calculation_on_Earth). This mean that the kettle requires no setup and always works anywhere in the world.

A sort of celestial [Teasmade](http://en.wikipedia.org/wiki/Teasmade).

![Oster kettle](http://www.brendanalbano.com/content/images/2015/04/2015-04-17-15-33-04.jpg)

This Oster electric kettle came apart really easily. I chose a kettle with a digital on/off switch rather than the clicky mechanical type, as I figured it'd be easier to control.

Planning on using an Adafruit GPS board and a Teensy LC (my Arduino Uno was having trouble with the math to calculate sunrise because it doesn't support 64-bit floating point numbers).
