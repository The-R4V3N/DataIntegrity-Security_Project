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

bool session_init(void);

int session_request(void);

bool session_response(const uint8_t *res, size_t size);

#endif // SESSION_H
