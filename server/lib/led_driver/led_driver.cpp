#include <bsp.h>
#include <stdbool.h>
#include "led_driver.h"

static uint8_t pin_num = 0xFFU;
static bool initialized = false;

/**
 * @brief Initializes the LED driver.
 *
 * This function is used to initialize the LED driver.
 *
 * @param pin The pin number to initialize the LED driver with.
 */
void led_driver_init(uint8_t pin)
{
    pin_num = pin;
    initialized = true;
    bsp_pin_mode(pin, OUTPUT);
    bsp_digital_write(pin, LED_DRIVER_LOW);
}

/**
 * @brief Sets the state of the LED driver.
 *
 * This function is used to set the state of the LED driver.
 *
 * @param state The state to set the LED driver to.
 * @return True if the state was set successfully, false otherwise.
 */
bool led_driver_set_state(uint8_t state)
{
    bool status = false;

    if (initialized)
    {
        if ((state == LED_DRIVER_HIGH) || (state == LED_DRIVER_LOW))
        {
            state = (state == LED_DRIVER_HIGH) ? LED_DRIVER_HIGH : LED_DRIVER_LOW;

            bsp_digital_write(pin_num, state);

            if (state == bsp_digital_read(pin_num))
            {
                status = true;
            }
        }
    }

    return status;
}