#ifndef BSP_H
#define BSP_H

#include <stdint.h>

#ifndef OUTPUT
#define OUTPUT 0x03
#endif

void bsp_pin_mode(uint8_t pin, uint8_t mode);
void bsp_digital_write(uint8_t pin, uint8_t val);
uint8_t bsp_digital_read(uint8_t pin);

#endif /* BSP_H */
