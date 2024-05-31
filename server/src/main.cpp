#include "session.h"
#include <Arduino.h>

void setup()
{
    pinMode(GPIO_NUM_21, OUTPUT); /**< Initialize the LED pin */
    pinMode(GPIO_NUM_32, OUTPUT); /**< Initialize the Relay pin */

    if (!session_init())
    {
        /* If the session is not initialized, blink the LED */
        while (1)
        {
            digitalWrite(GPIO_NUM_21, !digitalRead(GPIO_NUM_21));
            delay(5000);
        }
    }
}
void loop()
{
    int request = session_request(); /**< Request a session */

    digitalWrite(GPIO_NUM_32, LOW); /**< Initial Relay state */

    /* Handle the request */
    if (request == SESSION_REQ_TOGGLE_LED)
    {
        static uint8_t state = LOW;
        state = (state == LOW) ? HIGH : LOW;
        digitalWrite(GPIO_NUM_21, state);

        uint8_t buffer[2];
        buffer[1] = state;
        buffer[0] = (state == digitalRead(GPIO_NUM_21)) ? SESSION_RES_OKAY : SESSION_RES_ERROR;

        /* Respond to the request */
        if (!session_response(buffer, sizeof(buffer)))
        {
            request = SESSION_REQ_ERROR;
        }
    }
    /* Request the temperature */
    else if (request == SESSION_REQ_TEMPERATURE)
    {
        uint8_t buffer[7] = {0};
        buffer[0] = SESSION_RES_OKAY;
        sprintf((char *)&buffer[1], "%2.2f", temperatureRead());

        /* Respond to the request */
        if (!session_response(buffer, sizeof(buffer)))
        {
            request = SESSION_REQ_ERROR;
        }
    }
    else
    {
        /* Do Nothing */
    }

    /* Handle the error */
    if (request == SESSION_REQ_ERROR)
    {
        digitalWrite(GPIO_NUM_32, HIGH); /**< Turn on the Relay */
    }
}