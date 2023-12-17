#ifndef LED_DRIVER_H
#define LED_DRIVER_H

#include <stdint.h>

#define BAUD_RATE 115200
#define LED_PIN 21
#define LED_DRIVER_LOW 0U
#define LED_DRIVER_HIGH 1U

void led_driver_init(uint8_t pin);
bool led_driver_set_state(uint8_t state);

#endif // LED_DRIVER_H
