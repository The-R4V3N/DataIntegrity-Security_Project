/**
 * @file session.h
 * @author Oliver Joisten (contact@oliver-joisten.se)
 * @brief
 * @version 0.1
 * @date 2024-06-05
 *
 * @copyright Copyright (c) 2024
 *
 */

#ifndef SESSION_H
#define SESSION_H

/* Includes ------------------------------------------------------------------*/

#include <stdint.h>
#include <stddef.h>

/* Exported defines ----------------------------------------------------------*/

/* Exported types ------------------------------------------------------------*/

typedef enum
{
    SESSION_CLOSE,
    SESSION_ERROR,
    SESSION_TOGGLE_LED,
    SESSION_GET_TEMP,

    SESSION_ESTABLISH,
} request_t;

/* Exported constants --------------------------------------------------------*/

/* Exported macro ------------------------------------------------------------*/

/* Exported functions prototypes ---------------------------------------------*/

/**
 * @brief Initialize the communication module
 *
 * @return true if the communication module was successfully initialized
 * @return false if the communication module could not be initialized
 */
bool session_init(void);

/**
 * @brief Close the communication module
 *
 */
void session_close(void);

/**
 * @brief Establish a session
 *
 * @return true if the session was successfully established
 * @return false if the session could not be established
 */
bool session_establish(void);

/**
 * @brief Request a session
 *
 * @return request_t the request
 */
request_t session_request(void);

/**
 * @brief Respond to a session
 *
 * @param success the success of the response
 * @param res the response
 * @param rlen the length of the response
 * @return true if the response was successfully sent
 * @return false if the response could not be sent
 */
bool session_response(bool success, const uint8_t *res, size_t rlen);

#endif /* SESSION_H */