#include <bsp.h>
#include <stdbool.h>
#include "led_driver.h"

static uint8_t pin_num = 0xFFU;  /**< Pin number to initialize the LED driver with.*/
static bool initialized = false; /**< Initialization flag */

/**
 * @brief Initializes the LED driver.
 *
 * This function is used to initialize the LED driver.
 *
 * @param pin The pin number to initialize the LED driver with.
 */
void led_driver_init(uint8_t pin)
{
    pin_num = pin;                          /**< Pin number to initialize the LED driver with.*/
    initialized = true;                     /**< Initialization flag */
    bsp_pin_mode(pin, OUTPUT);              /**< Set the pin mode to OUTPUT */
    bsp_digital_write(pin, LED_DRIVER_LOW); /**< Set the initial state of the LED driver to LOW */
}

/**
 * @brief Sets the state of the LED driver.
 *
 * This function is used to set the state of the LED driver.
 *
 * @param state The state to set the LED driver to.
 * @return True if the state was set successfully, false otherwise.
 */
/**
 * Sets the state of an LED to either high or low, verifying initialization and state validity before applying the change.
 *
 * @param state The desired state of the LED, should be LED_DRIVER_HIGH or LED_DRIVER_LOW.
 * @return true if the LED state was successfully set, false otherwise.
 */
bool led_driver_set_state(uint8_t state)
{
    bool status = false; /**< Status flag */

    /* Check if the LED driver is initialized */
    if (initialized)
    {
        /* Check if the state is valid */
        if ((state == LED_DRIVER_HIGH) || (state == LED_DRIVER_LOW))
        {
            state = (state == LED_DRIVER_HIGH) ? LED_DRIVER_HIGH : LED_DRIVER_LOW;

            bsp_digital_write(pin_num, state); /**< Set the state of the LED driver */

            /* Check if the state was set successfully */
            if (state == bsp_digital_read(pin_num))
            {
                status = true; /**< Set the status flag to true */
            }
        }
    }

    return status;
}