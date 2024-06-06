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

#define ON "ON"
#define OFF "OFF"

/* Private typedef -----------------------------------------------------------*/

/* Private macro -------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/

/* Static Assertions ---------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/

/* Private user code ---------------------------------------------------------*/

/* Exported user code --------------------------------------------------------*/

/**
 * @brief Initializes the system setup.
 * 
 * This function sets up the necessary pins for the LED and Relay. It also checks if the session is initialized
 * and blinks the LED if it is not. 
 * 
 * @note This function assumes that the GPIO_NUM_21 pin is used for the LED and GPIO_NUM_32 pin is used for the Relay.
 * 
 */
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
            delay(500);                                          /**< Delay for 0.5 seconds */
        }
    }
}

/**
 * @brief The main loop function that handles different session requests.
 * 
 * This function is responsible for handling different session requests and executing the corresponding actions.
 * It receives a request from the session_request() function and performs the necessary operations based on the request type.
 * The function supports the following request types:
 * @retval #SESSION_ESTABLISH: Establishes a session with the client.
 * @retval #SESSION_CLOSE: Closes the current session.
 * @retval #SESSION_GET_TEMP: Retrieves the temperature reading and sends it as a response.
 * @retval #SESSION_TOGGLE_LED: Toggles the state of an LED and sends the updated state as a response.
 * 
 * @note This function assumes that the necessary GPIO pins have been configured and initialized.
 * 
 * @note The function uses the session_establish(), session_close(), session_response(), and temperatureRead() functions to perform the required operations.
 * 
 * @note If an error occurs during the execution of a request, the function sets the request to SESSION_ERROR and takes appropriate action.
 * 
 * @note If the request is SESSION_ERROR, the function sets the GPIO_NUM_32 pin to HIGH.
 */
void loop()
{
    char response[8] = {0};     /**< Response buffer */
    static uint8_t state = LOW; /**< LED state */

    request_t request = session_request(); /**< Get the session request */
    digitalWrite(GPIO_NUM_32, LOW);        /**< Reset the Relay pin */

    /* Handle the session request */
    switch (request)
    {
    /* Handle the session establish request */    
    case SESSION_ESTABLISH:
        if (!session_establish())
        {
            request = SESSION_ERROR;
        }
        break;
    /* Handle the session closed request */
    case SESSION_CLOSE:
        session_close();
        if (!session_response(true, nullptr, 0))
        {
            request = SESSION_ERROR;
        }
        break;
    /* Handle the session get temperature request */
    case SESSION_GET_TEMP:
        sprintf((char *)&response, "%2.2f", temperatureRead());

        if (!session_response(true, (const uint8_t *)response, strlen(response)))
        {
            request = SESSION_ERROR;
        }
        break;
    /* Handle the session toggle LED request */
    case SESSION_TOGGLE_LED:
        state = (state == LOW) ? HIGH : LOW;
        digitalWrite(GPIO_NUM_21, state);
        strcpy(response, (LOW == digitalRead(GPIO_NUM_21)) ? OFF : ON);

        if (!session_response((state == digitalRead(GPIO_NUM_21)), (const uint8_t *)response, strlen(response)))
        {
            request = SESSION_ERROR;
        }
        break;

    default:
        break;
    }

    /* Handle the session error */
    if (request == SESSION_ERROR)
    {
        digitalWrite(GPIO_NUM_32, HIGH);
    }
}