#ifndef COMMUNICATION_H
#define COMMUNICATION_H

#include <stddef.h>
#include <stdint.h>

/**
 * @brief Initialize the communication module
 *
 * @return true if the communication module was successfully initialized
 * @return false if the communication module could not be initialized
 */
bool communication_init(void);

/**
 * @brief Read data from the communication module
 *
 * @param buffer the buffer to store the data
 * @param blen the length of the buffer
 * @return size_t the length of the data
 */
size_t client_read(uint8_t *buffer, size_t blen);

/**
 * @brief Write data to the communication module
 *
 * @param buffer the data to write
 * @param dlen the length of the data
 * @return true if the data was successfully written
 * @return false if the data could not be written
 */
bool client_write(uint8_t *buffer, size_t dlen);

#endif // COMMUNICATION_H
