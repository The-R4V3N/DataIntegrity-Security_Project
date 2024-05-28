/**
 * @file bsp.cpp
 * @author Oliver Joisten (contact@oliver-joisten.se)
 * @brief  This file contains the implementation of the Board Support Package (BSP) for the project.
 * @version 0.1
 * @date 2024-05-25
 *
 * @copyright Copyright (c) 2024
 *
 */

#include <Arduino.h>
#include "bsp.h"

/**
 * @brief Configures the mode of a specified pin on the microcontroller using Arduino pinMode function.
 *
 * @param pin The pin number to configure.
 * @param mode The mode to set for the pin, such as INPUT, OUTPUT, etc.
 */
void bsp_pin_mode(uint8_t pin, uint8_t mode)
{
    pinMode(pin, mode);
}

/**
 * @brief Writes a digital value (HIGH or LOW) to a specified pin on the microcontroller.
 *
 * @param pin The pin number to which the value is written.
 * @param val The value to write to the pin, either HIGH or LOW.
 */
void bsp_digital_write(uint8_t pin, uint8_t val)
{
    digitalWrite(pin, val);
}

/**
 * @brief Reads the digital value (HIGH or LOW) from a specified pin on the microcontroller.
 *
 * @param pin The pin number from which the digital value is to be read.
 * @return An uint8_t value representing the digital state of the pin, which can be either HIGH or LOW.
 */
uint8_t bsp_digital_read(uint8_t pin)
{
    // Simply delegate the task to the digitalRead function from the Arduino library
    return digitalRead(pin);
}
