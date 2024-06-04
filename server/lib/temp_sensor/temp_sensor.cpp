/**
 * @file temp_sensor.cpp
 * @author Oliver Joisten (contact@oliver-joisten.se)
 * @brief  This file contains the implementation of the temperature sensor module.
 * @version 0.1
 * @date 2024-05-25
 *
 * @copyright Copyright (c) 2024
 *
 */

#include "temp_sensor.h"

void temp_sensor_init(void)
{
    ;
}

float temp_sensor_read(void)
{
    uint8_t temp_farenheit = temperatureRead();
    float temp_celsius = (temp_farenheit - 32) / 1.8;
    
    return (temp_celsius);
}
