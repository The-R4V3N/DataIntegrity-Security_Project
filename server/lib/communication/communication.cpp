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

#include "communication.h"
#include <Arduino.h>

#define BAUDRATE 115200

bool communication_init()
{
    Serial.begin(BAUDRATE);
    return Serial;
}

size_t client_read(uint8_t *buffer, size_t blen)
{
    while (0 == Serial.available())
    {
        ;
    }

    return Serial.readBytes(buffer, blen);
}

bool client_write(uint8_t *buffer, size_t dlen)
{
    return (dlen == Serial.write(buffer, dlen));
}