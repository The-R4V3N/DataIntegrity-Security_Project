#ifndef SESSION_H
#define SESSION_H

#include <stddef.h>
#include <stdint.h>

enum
{
    SESSION_REQ_OKAY,
    SESSION_REQ_ERROR,
    SESSION_REQ_TOGGLE_LED,
    SESSION_REQ_TEMPERATURE
};

enum
{
    SESSION_RES_OKAY,
    SESSION_RES_ERROR,
    SESSION_RES_EXPIRED,
    SESSION_RES_INVALID,
    SESSION_RES_HASH_ERROR,
    SESSION_RES_BAD_REQUEST
};

bool session_init(void);

int session_request(void);

bool session_response(const uint8_t *res, size_t size);

#endif // SESSION_H
