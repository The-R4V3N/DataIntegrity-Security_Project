/**
 * @file session.cpp
 * @author Oliver Joisten (contact@oliver-joisten.se)
 * @brief
 * @version 0.1
 * @date 2024-05-29
 *
 * @copyright Copyright (c) 2024
 *
 */

#include "session.h"
#include <iostream>
#include <vector>
#include <cstring>

// External HMAC key provided by the security module
extern unsigned char hmac_key[];
extern size_t hmac_key_len;

Session::Session() : connect_state(false)
{
    init_rsa();
    init_hmac();
    communication_init();
}

Session::~Session()
{
    close_session();
    free_rsa();
    free_hmac();
}

void Session::init_rsa()
{
    mbedtls_rsa_init(&rsa, MBEDTLS_RSA_PKCS_V15, 0);
    // Initialize and generate RSA key pair here, or load existing keys
}

void Session::init_hmac()
{
    mbedtls_md_init(&hmac_ctx);
    mbedtls_md_setup(&hmac_ctx, mbedtls_md_info_from_type(MBEDTLS_MD_SHA256), 1);
}

void Session::free_rsa()
{
    mbedtls_rsa_free(&rsa);
}

void Session::free_hmac()
{
    mbedtls_md_free(&hmac_ctx);
}

bool Session::session()
{
    return connect_state;
}

bool Session::toggle_led()
{
    if (connect_state)
    {
        return send_request("0x49");
    }
    return false;
}

bool Session::get_temp()
{
    if (connect_state)
    {
        return send_request("0x54");
    }
    return false;
}

void Session::close_session()
{
    if (connect_state)
    {
        send_request("0x10");
        connect_state = false;
    }
}

bool Session::send_request(const std::string &data)
{
    unsigned char hmac_value[32];
    mbedtls_md_context_t hmac_ctx_local;
    mbedtls_md_init(&hmac_ctx_local);
    mbedtls_md_setup(&hmac_ctx_local, mbedtls_md_info_from_type(MBEDTLS_MD_SHA256), 1);

    mbedtls_md_hmac_starts(&hmac_ctx_local, hmac_key, hmac_key_len);
    mbedtls_md_hmac_update(&hmac_ctx_local, (unsigned char *)data.c_str(), data.length());
    mbedtls_md_hmac_finish(&hmac_ctx_local, hmac_value);
    mbedtls_md_free(&hmac_ctx_local);

    std::string request_data = data + std::string((char *)hmac_value, 32);
    bool success = client_write((uint8_t *)request_data.c_str(), request_data.size());
    if (!success)
    {
        last_error = "Connection Error: Could not write data.";
        close_session();
        return false;
    }
    return true;
}

std::string Session::receive_data(int size)
{
    std::vector<uint8_t> buffer(size + 32);
    size_t read_bytes = client_read(buffer.data(), buffer.size());

    if (read_bytes != buffer.size())
    {
        last_error = "Connection Error: Could not read data.";
        close_session();
        return "";
    }

    unsigned char hmac_value[32];
    mbedtls_md_context_t hmac_ctx_local;
    mbedtls_md_init(&hmac_ctx_local);
    mbedtls_md_setup(&hmac_ctx_local, mbedtls_md_info_from_type(MBEDTLS_MD_SHA256), 1);

    mbedtls_md_hmac_starts(&hmac_ctx_local, hmac_key, hmac_key_len);
    mbedtls_md_hmac_update(&hmac_ctx_local, buffer.data(), size);
    mbedtls_md_hmac_finish(&hmac_ctx_local, hmac_value);
    mbedtls_md_free(&hmac_ctx_local);

    if (memcmp(hmac_value, buffer.data() + size, 32) != 0)
    {
        last_error = "Hash Error";
        close_session();
        return "";
    }
    return std::string(buffer.begin(), buffer.begin() + size);
}

std::string Session::get_last_error()
{
    return last_error;
}
