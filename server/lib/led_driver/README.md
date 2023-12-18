# LED Driver Readme

This directory contains the LED Driver for the Olimex ESP32-EVB Development Board.

## Hardware

The Olimex ESP32-EVB is the ultimate IoT board with wired 100Mb Ethernet Interface, Bluetooth LE, WiFi, Remote control IR, and CAN connectivity. The board includes a USB-to-serial converter, it includes two relays which allow you to switch power appliances on and off. The board also features a microSD card slot, which can be used to store data. The board also includes a number of GPIO pins, which are used to interface with the LED.
A detailed description of the board can be found [here](https://www.olimex.com/Products/IoT/ESP32/ESP32-EVB/open-source-hardware).

A LED is connected to GPIO pin 21 of the ESP32.

## Software

The LED Driver is written in Cpp and is based on the ESP-IDF framework. The LED Driver provides an interface for the application to interact with the LED.

### LED Driver Interface

The LED Driver provides the following interface:

```cpp
void led_driver_init();
void led_driver_set_state(bool state);
bool led_driver_get_state();
```

The `led_driver_init()` function initializes the LED Driver. The `led_driver_set_state()` function sets the state of the LED. The `led_driver_get_state()` function returns the state of the LED.

### LED Driver Implementation

The LED Driver is implemented in `led_driver.cpp` and `led_driver.h`. The LED Driver uses the ESP-IDF GPIO API to interact with the LED.

## Usage

The LED Driver is used by the application to interface with the LED. The LED Driver provides a set of functions for the application to interface with the LED.

### LED Driver Initialization

The LED Driver must be initialized before it can be used. The LED Driver is initialized by calling the `led_driver_init()` function.

### LED Driver Unit Tests

The LED Driver Unit Tests are implemented in `led_driver_test.cpp`. The LED Driver Unit Tests use the ESP-IDF GPIO API to interact with the LED.
