#include "session.h"
#include <Arduino.h>

void setup()
{
    pinMode(GPIO_NUM_21, OUTPUT); // Initialize the LED pin
    pinMode(GPIO_NUM_32, OUTPUT);

    if (!session_init())
    {
        while (1)
        {
            digitalWrite(GPIO_NUM_21, !digitalRead(GPIO_NUM_21));
            delay(5000);
        }
    }
}
void loop()
{
    int request = session_request();

    digitalWrite(GPIO_NUM_32, LOW);

    if (request == SESSION_REQ_TOGGLE_LED)
    {
        static uint8_t state = LOW;
        state = (state == LOW) ? HIGH : LOW;
        digitalWrite(GPIO_NUM_21, state);

        uint8_t buffer[2];
        buffer[1] = state;
        buffer[0] = (state == digitalRead(GPIO_NUM_21)) ? SESSION_RES_OKAY : SESSION_RES_ERROR;

        if (!session_response(buffer, sizeof(buffer)))
        {
            request = SESSION_REQ_ERROR;
        }
    }
    else if (request == SESSION_REQ_TEMPERATURE)
    {
        uint8_t buffer[7] = {0};
        buffer[0] = SESSION_RES_OKAY;
        sprintf((char *)&buffer[1], "%2.2f", temperatureRead());

        if (!session_response(buffer, sizeof(buffer)))
        {
            request = SESSION_REQ_ERROR;
        }
    }
    else
    {
        ;
    }

    if (request == SESSION_REQ_ERROR)
    {
        digitalWrite(GPIO_NUM_32, HIGH);
    }
}