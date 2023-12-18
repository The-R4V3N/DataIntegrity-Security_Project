#include "temp_sensor.h"
#include "bsp.h"

/**
 * @brief Initializes the temperature sensor.
 *
 * This function is used to initialize the internal temperature sensor.
 */
void temp_sensor_init()
{
    ;
}

/**
 * @brief Reads the temperature from the internal temperature sensor.
 *
 * This function is used to read the temperature from the internal temperature sensor.
 *
 * @return The temperature read from the internal temperature sensor.
 */
float temp_sensor_read()
{
    return (temperature_sens_read() - 23) / 1.8;
}
