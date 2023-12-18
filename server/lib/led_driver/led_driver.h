#ifndef LED_DRIVER_H
#define LED_DRIVER_H

#include <stdint.h>

#define BAUD_RATE 115200
#define LED_PIN 21
#define LED_DRIVER_LOW 0U
#define LED_DRIVER_HIGH 1U

/**
 * @brief Initializes the LED driver.
 *
 * This function is used to initialize the LED driver.
 *
 * @param pin The pin number to initialize the LED driver with.
 */
void led_driver_init(uint8_t pin);

/**
 * @brief Sets the state of the LED driver.
 *
 * This function is used to set the state of the LED driver.
 *
 * @param state The state to set the LED driver to.
 * @return True if the state was set successfully, false otherwise.
 */
bool led_driver_set_state(uint8_t state);

#endif // LED_DRIVER_H
