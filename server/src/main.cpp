/**
 * @file main.cpp
 * @author Oliver Joisten (contact@oliver-joisten.se)
 * @brief 
 * @version 0.1
 * @date 2024-06-05
 * 
 * @copyright Copyright (c) 2024
 * 
 */

    /* Includes ------------------------------------------------------------------*/

#include "session.h"
#include "temp_sensor.h"
#include <Arduino.h>

    /* Private define ------------------------------------------------------------*/

    /* Private typedef -----------------------------------------------------------*/

    /* Private macro -------------------------------------------------------------*/

    /* Private variables ---------------------------------------------------------*/

    /* Static Assertions ---------------------------------------------------------*/

    /* Private function prototypes -----------------------------------------------*/

    /* Private user code ---------------------------------------------------------*/

    /* Exported user code --------------------------------------------------------*/

void setup(void)
{
    pinMode(GPIO_NUM_21, OUTPUT); /**< Initialize the LED pin */
    // TODO Remove this line below it is just for Debugging
    pinMode(GPIO_NUM_32, OUTPUT); /**< Initialize the Relay pin */
    /* Check for initialize Error*/
    if (!session_init())
    {
        /* If the session is not initialized, blink the LED */
        while (1)
        {
            digitalWrite(GPIO_NUM_21, !digitalRead(GPIO_NUM_21)); /**< Toggle the LED */
            delay(1000);                                          /**< Delay for 1 seconds */
        }
    }
}
void loop(void)
{
    int request = session_request(); /**< Request a session */

    // TODO Remove this line below it is just for Debugging if the seesion_request() is not working
    digitalWrite(GPIO_NUM_32, LOW);

    /* Handle the request */
    if (request == SESSION_TOGGLE_LED)
    {
        static uint8_t state = LOW;             /**< The state of the LED initialized as LOW (0) */
        state = (state == LOW) ? HIGH : LOW;    /**< Toggle the state of the LED */
        digitalWrite(GPIO_NUM_21, state);       /**< Toggle the LED */

        uint8_t buffer[2];                                                  /**< The buffer to store the response */
        buffer[1] = digitalRead(GPIO_NUM_21);                               /**< Store the state of the LED */
        buffer[0] = (state == buffer[1]) ? SESSION_OKAY : SESSION_ERROR;    /**< Store the response */

        /* Respond to the request */
        if (!session_response(buffer, sizeof(buffer)))
        {
            request = SESSION_ERROR;   /**< Set the request to error */
        }
    }
    /* Request the temperature */
    else if (request == SESSION_TEMPERATURE)
    {
        uint8_t buffer[7] = {0};                                    /**< The buffer to store the response */
        buffer[0] = SESSION_OKAY;                                   /**< Store the response */
        sprintf((char *)&buffer[1], "%2.2f", temp_sensor_read());   /**< Store the temperature in the buffer */

        /* Respond to the request */
        if (!session_response(buffer, sizeof(buffer)))
        {
            request = SESSION_ERROR;  /**< Set the request to error */
        }
    }
    else
    {
        /* Do Nothing */
    }

    /* Handle the Server error */
    if (request == SESSION_ERROR)
    {
        digitalWrite(GPIO_NUM_32, HIGH); /**< Turn on the Relay */
    }
}
