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

#include <stddef.h>
#include <stdint.h>

    /* Exported defines ----------------------------------------------------------*/

    /* Exported types ------------------------------------------------------------*/

    enum
    {
        SESSION_OKAY,
        SESSION_ERROR,
        SESSION_TOGGLE_LED,
        SESSION_TEMPERATURE,
        SESSION_EXPIRED
    };

    /* Exported constants --------------------------------------------------------*/

    /* Exported macro ------------------------------------------------------------*/

    /* Exported functions prototypes ---------------------------------------------*/

    /**
     * @brief Initialize the session
     *
     * @return true if the session is initialized successfully else false.
     */
    bool session_init(void);

    /**
     * @brief Request a session
     *
     * @retval SESSION_TOGGLE_LED if the request is to toggle the LED
     * @retval SESSION_TEMPERATURE if the request is to get the temperature
     * @retval SESSION_EXPIRED if the session has expired
     * @retval SESSION_ERROR if the request is invalid
     * @retval SESSION_OKAY if the request is successful
     * @retval SESSION_ERROR if the request is unsuccessful
     * @retval SESSION_ERROR if the request is invalid
     *
     * @return int the request
     */
    int session_request(void);

    /**
     * @brief Respond to a session
     *
     * @param res the response buffer
     * @param size the size of the response buffer
     *
     * @retval STATUS_OK if the response is successful
     * @retval STATUS_ERROR if the response is unsuccessful
     *
     * @return true if the response is successful else false.
     */
    bool session_response(const uint8_t *res, size_t size);

#endif // SESSION_H
