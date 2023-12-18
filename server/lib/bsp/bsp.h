#ifndef BSP_H
#define BSP_H

#include <stdint.h>

#ifndef OUTPUT
#define OUTPUT 0x03
#endif

/**
 * @brief Sets the mode of a pin in the BSP.
 *
 * This function is used to set the mode of a pin in the Board Support Package (BSP).
 *
 * @param pin The pin number to set the mode for.
 * @param mode The mode to set for the pin.
 */
void bsp_pin_mode(uint8_t pin, uint8_t mode);

/**
 * @brief Writes a value to a pin in the BSP.
 *
 * This function is used to write a value to a pin in the Board Support Package (BSP).
 *
 * @param pin The pin number to write the value to.
 * @param val The value to write to the pin.
 */
void bsp_digital_write(uint8_t pin, uint8_t val);

/**
 * @brief Reads a value from a pin in the BSP.
 *
 * This function is used to read a value from a pin in the Board Support Package (BSP).
 *
 * @param pin The pin number to read the value from.
 * @return The value read from the pin.
 */
uint8_t bsp_digital_read(uint8_t pin);

#endif /* BSP_H */
