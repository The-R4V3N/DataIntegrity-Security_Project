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
#include "bsp.h"

void temp_sensor_init()
{
    ;
}

float temp_sensor_read()
{
    return (temp_sensor_read() - 23) / 1.8;
}
