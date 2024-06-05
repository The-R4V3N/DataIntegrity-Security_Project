/**
 * @file communication.cpp
 * @author Oliver Joisten (contact@oliver-joisten.se)
 * @brief
 * @version 0.1
 * @date 2024-05-29
 *
 * @copyright Copyright (c) 2024
 *
 */

    /* Includes ------------------------------------------------------------------*/

#include "communication.h"
#include <Arduino.h>

    /* Private define ------------------------------------------------------------*/

    /* Private typedef -----------------------------------------------------------*/

    /* Private macro -------------------------------------------------------------*/

#define BAUDRATE 115200 /**< Baudrate for the Serial Communication */

    /* Private variables ---------------------------------------------------------*/

    /* Static Assertions ---------------------------------------------------------*/

    /* Private function prototypes -----------------------------------------------*/

    /* Private user code ---------------------------------------------------------*/

    /* Exported user code --------------------------------------------------------*/

bool communication_init(void)
{
    Serial.begin(BAUDRATE); /**< Initialize the Serial Communication */
    return Serial;          /**< Return the Serial Communication */
} 
size_t communication_read(uint8_t *buffer, size_t blen)
{
    /* Wait for the data to be available */
    while (0 == Serial.available())
    {
        ;
    }

    return Serial.readBytes(buffer, blen); /**< Read the data from the Serial Communication */
}

bool communication_write(uint8_t *buffer, size_t dlen)
{
    return (dlen == Serial.write(buffer, dlen)); /**< Write the data to the Serial Communication */
}

