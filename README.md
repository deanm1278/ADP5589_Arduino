# ADP5589_Arduino
arduino library for Analog Devices ADP5589 Keypad controller

Datasheet for part can be found here:
http://www.analog.com/media/en/technical-documentation/data-sheets/ADP5589.pdf

This library provides a simple arduino interface to the ADP5589 keypad controller.
It allows you to set active keys, use GPIOs, and register callbacks when specific key events are detected.

Not the most useful library to most people seeing as the IC only comes in SMD packages and is not
compatible with the 5V logic most standard arduino boards use, but its ok.
