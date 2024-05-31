#ifndef SESSION_H
#define SESSION_H

#include <stddef.h>
#include <stdint.h>

enum
{
    SESSION_OKAY,
    SESSION_ERROR,
    SESSION_TOGGLE_LED,
    SESSION_TEMPERATURE
};

/**
 * @brief Initialize the session
 *
 * @return true if the session is initialized successfully else false.
 */
bool session_init(void);

/**
 * @brief Request a session
 *
 * @return int the request type
 */
int session_request(void);

/**
 * @brief Respond to a session
 *
 * @param res the response buffer
 * @param size the size of the response buffer
 * @return true if the response is successful else false.
 */
bool session_response(const uint8_t *res, size_t size);

#endif // SESSION_H
