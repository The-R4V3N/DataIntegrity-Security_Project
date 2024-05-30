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
#include <Arduino.h>
#include <mbedtls/md.h>
#include <mbedtls/pk.h>
#include <mbedtls/rsa.h>
#include <mbedtls/aes.h>
#include <mbedtls/entropy.h>
#include <mbedtls/ctr_drbg.h>

constexpr int AES_SIZE{32};
constexpr int DER_SIZE{294};
constexpr int RSA_SIZE{256};
constexpr int HASH_SIZE{32};
constexpr int EXPONENT{65537};
constexpr int AES_BLOCK_SIZE{16};

static mbedtls_aes_context aesEncryptionContext;
static mbedtls_md_context_t hmacContext;
static mbedtls_pk_context clientPublicKeyContext;
static mbedtls_pk_context server_ctx;
static mbedtls_entropy_context entropy;
static mbedtls_ctr_drbg_context ctr_drbg;
static uint64_t sessionId{0};
static uint8_t aes_key[AES_SIZE]{0};
static uint8_t encryptionInitializationVector[AES_BLOCK_SIZE]{0};
static uint8_t decryptionInitializationVector[AES_BLOCK_SIZE]{0};
static const uint8_t hmac_hash[HASH_SIZE] = {0x29, 0x49, 0xde, 0xc2, 0x3e, 0x1e, 0x34, 0xb5, 0x2d, 0x22, 0xb5,
                                             0xba, 0x4c, 0x34, 0x23, 0x3a, 0x9d, 0x3f, 0xe2, 0x97, 0x14, 0xbe,
                                             0x24, 0x62, 0x81, 0x0c, 0x86, 0xb1, 0xf6, 0x92, 0x54, 0xd6};

static size_t client_read(uint8_t *buffer, size_t blen)
{
    while (0 == Serial.available())
    {
        ;
    }
    size_t length = Serial.readBytes(buffer, blen);
    if (length > HASH_SIZE)
    {
        length -= HASH_SIZE;
        uint8_t hmac[HASH_SIZE]{0};
        mbedtls_md_hmac_starts(&hmacContext, hmac_hash, HASH_SIZE);
        mbedtls_md_hmac_update(&hmacContext, buffer, length);
        mbedtls_md_hmac_finish(&hmacContext, hmac);
        if (0 != memcmp(hmac, buffer + length, HASH_SIZE))
        {
            length = 0;
        }
    }
    else
    {
        length = 0;
    }
    return length;
}

static bool client_write(uint8_t *buffer, size_t dlen)
{
    bool status{false};
    mbedtls_md_hmac_starts(&hmacContext, hmac_hash, HASH_SIZE);
    mbedtls_md_hmac_update(&hmacContext, buffer, dlen);
    mbedtls_md_hmac_finish(&hmacContext, buffer + dlen);
    dlen += HASH_SIZE;
    if (dlen == Serial.write(buffer, dlen))
    {
        Serial.flush();
        status = true;
    }
    return status;
}

bool session_init(void)
{
    mbedtls_md_init(&hmacContext);
    assert(0 == mbedtls_md_setup(&hmacContext, mbedtls_md_info_from_type(MBEDTLS_MD_SHA256), 1));
    // AES-256
    mbedtls_aes_init(&aesEncryptionContext);
    uint8_t initial[AES_SIZE]{0};
    mbedtls_entropy_init(&entropy);
    mbedtls_ctr_drbg_init(&ctr_drbg);
    for (size_t i = 0; i < sizeof(initial); i++)
    {
        initial[i] = random(0x100);
    }
    assert(0 == mbedtls_ctr_drbg_seed(&ctr_drbg, mbedtls_entropy_func, &entropy, initial, sizeof(initial)));
    // RSA-2048
    mbedtls_pk_init(&server_ctx);
    assert(0 == mbedtls_pk_setup(&server_ctx, mbedtls_pk_info_from_type(MBEDTLS_PK_RSA)));
    assert(0 == mbedtls_rsa_gen_key(mbedtls_pk_rsa(server_ctx), mbedtls_ctr_drbg_random,
                                    &ctr_drbg, RSA_SIZE * CHAR_BIT, EXPONENT));
}

int session_request(void)
{
}

bool session_response(const uint8_t *res, size_t size)
{
}