#ifndef SESSION_H
#define SESSION_H

#include <string>
#include "communication.h"
#include "mbedtls/rsa.h"
#include "mbedtls/md.h"

class Session
{
public:
    Session();
    ~Session();

    bool session();
    bool toggle_led();
    bool get_temp();
    void close_session();
    std::string get_last_error();

private:
    bool connect_state;
    std::string last_error;

    mbedtls_rsa_context rsa;
    mbedtls_md_context_t hmac_ctx;

    bool send_request(const std::string &data);
    std::string receive_data(int size);

    void init_rsa();
    void init_hmac();
    void free_rsa();
    void free_hmac();
};

#endif // SESSION_H