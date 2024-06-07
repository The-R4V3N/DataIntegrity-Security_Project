/**
 * @file communication.h
 * @author Oliver Joisten (contact@oliver-joisten.se)
 * @brief
 * @version 0.1
 * @date 2024-06-05
 *
 * @copyright Copyright (c) 2024
 *
 */

#ifndef COMMUNICATION_H
#define COMMUNICATION_H

/* Includes ------------------------------------------------------------------*/

#include <stdint.h>
#include <stddef.h>

/* Exported defines ----------------------------------------------------------*/

/* Exported types ------------------------------------------------------------*/

/* Exported constants --------------------------------------------------------*/

/* Exported macro ------------------------------------------------------------*/

/* Exported functions prototypes ---------------------------------------------*/

/**
 * @brief Initialize the communication module
 *
 * @return true if the communication module was successfully initialized else false
 */
bool communication_init(void);

/**
 * @brief Read data from the communication module
 *
 * @param buffer the buffer to store the data
 * @param dlen the length of the buffer
 * @return size_t the length of the data
 */
bool communication_write(const uint8_t *data, size_t dlen);

/**
 * @brief Read data from the communication module
 *
 * @param buf the buffer to store the data
 * @param blen the length of the buffer
 * @return size_t the length of the data
 */
size_t communication_read(uint8_t *buf, size_t blen);

#endif // COMMUNICATION_H