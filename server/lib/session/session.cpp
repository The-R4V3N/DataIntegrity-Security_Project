#include "session.h"
#include <Arduino.h>
#include <mbedtls/md.h>
#include <mbedtls/pk.h>
#include <mbedtls/rsa.h>
#include <mbedtls/aes.h>
#include "communication.h"
#include <mbedtls/entropy.h>
#include <mbedtls/ctr_drbg.h>

#define SESSION_CLOSE 0xFF

enum
{
    STATUS_OKAY,
    STATUS_ERROR,
    STATUS_EXPIRED,
    STATUS_HASH_ERROR,
    STATUS_BAD_REQUEST,
    STATUS_INVALID_SESSION,
    LED_TOGGLED
};

constexpr int AES_SIZE{32};
constexpr int DER_SIZE{294};
constexpr int RSA_SIZE{256};
constexpr int HASH_SIZE{32};
constexpr int EXPONENT{65537};
constexpr int AES_BLOCK_SIZE{16};

static mbedtls_aes_context aes_ctx;
static mbedtls_md_context_t hmac_ctx;
static mbedtls_pk_context client_ctx;
static mbedtls_pk_context server_ctx;
static mbedtls_entropy_context entropy;
static mbedtls_ctr_drbg_context ctr_drbg;

static uint64_t session_id{0};
static uint32_t prev_millis{0};
static uint8_t aes_key[AES_SIZE]{0};
static uint8_t enc_iv[AES_BLOCK_SIZE]{0};
static uint8_t dec_iv[AES_BLOCK_SIZE]{0};
static const uint8_t secret_key[HASH_SIZE] = {0x29, 0x49, 0xde, 0xc2, 0x3e, 0x1e, 0x34, 0xb5, 0x2d, 0x22, 0xb5,
                                              0xba, 0x4c, 0x34, 0x23, 0x3a, 0x9d, 0x3f, 0xe2, 0x97, 0x14, 0xbe,
                                              0x24, 0x62, 0x81, 0x0c, 0x86, 0xb1, 0xf6, 0x92, 0x54, 0xd6};

bool session_response(const uint8_t *res, size_t size);

static size_t client_read(uint8_t *buf, size_t blen)
{
    size_t length = communication_read(buf, blen);

    if (length > HASH_SIZE)
    {
        length -= HASH_SIZE;
        uint8_t hmac[HASH_SIZE]{0};
        mbedtls_md_hmac_starts(&hmac_ctx, secret_key, HASH_SIZE);
        mbedtls_md_hmac_update(&hmac_ctx, buf, length);
        mbedtls_md_hmac_finish(&hmac_ctx, hmac);
        if (0 != memcmp(hmac, buf + length, HASH_SIZE))
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

static bool client_write(uint8_t *buf, size_t dlen)
{
    mbedtls_md_hmac_starts(&hmac_ctx, secret_key, HASH_SIZE);
    mbedtls_md_hmac_update(&hmac_ctx, buf, dlen);
    mbedtls_md_hmac_finish(&hmac_ctx, buf + dlen);
    dlen += HASH_SIZE;

    return communication_write(buf, dlen);
}

static void exchange_public_keys(uint8_t *buf)
{
    session_id = 0;
    size_t olen, length;

    mbedtls_pk_init(&client_ctx);
    uint8_t cipher[3 * RSA_SIZE + HASH_SIZE] = {0};

    assert(0 == mbedtls_pk_parse_public_key(&client_ctx, buf, DER_SIZE));
    assert(MBEDTLS_PK_RSA == mbedtls_pk_get_type(&client_ctx));

    assert(DER_SIZE == mbedtls_pk_write_pubkey_der(&server_ctx, buf, DER_SIZE));

    assert(0 == mbedtls_pk_encrypt(&client_ctx, buf, DER_SIZE / 2, cipher,
                                   &olen, RSA_SIZE, mbedtls_ctr_drbg_random, &ctr_drbg));

    assert(0 == mbedtls_pk_encrypt(&client_ctx, buf + DER_SIZE / 2, DER_SIZE / 2,
                                   cipher + RSA_SIZE, &olen, RSA_SIZE, mbedtls_ctr_drbg_random, &ctr_drbg));

    length = 2 * RSA_SIZE;
    assert(client_write(cipher, length));

    length = client_read(cipher, sizeof(cipher));
    assert(length == 3 * RSA_SIZE);

    assert(0 == mbedtls_pk_decrypt(&server_ctx, cipher, RSA_SIZE, buf, &olen, RSA_SIZE,
                                   mbedtls_ctr_drbg_random, &ctr_drbg));

    length = olen;
    assert(0 == mbedtls_pk_decrypt(&server_ctx, cipher + RSA_SIZE, RSA_SIZE, buf + length,
                                   &olen, RSA_SIZE, mbedtls_ctr_drbg_random, &ctr_drbg));

    length += olen;
    assert(0 == mbedtls_pk_decrypt(&server_ctx, cipher + 2 * RSA_SIZE, RSA_SIZE, buf + length,
                                   &olen, RSA_SIZE, mbedtls_ctr_drbg_random, &ctr_drbg));

    length += olen;
    assert(length == (DER_SIZE + RSA_SIZE));

    mbedtls_pk_init(&client_ctx);
    assert(0 == mbedtls_pk_parse_public_key(&client_ctx, buf, DER_SIZE));
    assert(MBEDTLS_PK_RSA == mbedtls_pk_get_type(&client_ctx));

    assert(0 == mbedtls_pk_verify(&client_ctx, MBEDTLS_MD_SHA256, secret_key, HASH_SIZE, buf + DER_SIZE, RSA_SIZE));

    strcpy((char *)buf, "OKAY");
    assert(0 == mbedtls_pk_encrypt(&client_ctx, buf, strlen((const char *)buf),
                                   cipher, &olen, RSA_SIZE, mbedtls_ctr_drbg_random, &ctr_drbg));

    length = RSA_SIZE;
    assert(client_write(cipher, length));
}

static void session_establish(uint8_t *buf)
{
    session_id = 0;
    size_t olen, length;
    uint8_t cipher[2 * RSA_SIZE]{0};

    assert(0 == mbedtls_pk_decrypt(&server_ctx, buf, RSA_SIZE, cipher, &olen,
                                   RSA_SIZE, mbedtls_ctr_drbg_random, &ctr_drbg));
    length = olen;

    assert(0 == mbedtls_pk_decrypt(&server_ctx, buf + RSA_SIZE, RSA_SIZE, cipher + length,
                                   &olen, RSA_SIZE, mbedtls_ctr_drbg_random, &ctr_drbg));

    length += olen;
    assert(length == RSA_SIZE);

    assert(0 == mbedtls_pk_verify(&client_ctx, MBEDTLS_MD_SHA256, secret_key, HASH_SIZE, cipher, RSA_SIZE));

    uint8_t *ptr{(uint8_t *)&session_id};
    for (size_t i = 0; i < sizeof(session_id); i++)
    {
        ptr[i] = random(1, 0x100);
    }

    for (size_t i = 0; i < sizeof(enc_iv); i++)
    {
        enc_iv[i] = random(0x100);
    }
    memcpy(dec_iv, enc_iv, sizeof(dec_iv));

    for (size_t i = 0; i < sizeof(aes_key); i++)
    {
        aes_key[i] = random(0x100);
    }

    assert(0 == mbedtls_aes_setkey_enc(&aes_ctx, aes_key, sizeof(aes_key) * CHAR_BIT));

    memcpy(buf, &session_id, sizeof(session_id));
    length = sizeof(session_id);

    memcpy(buf + length, enc_iv, sizeof(enc_iv));
    length += sizeof(enc_iv);

    memcpy(buf + length, aes_key, sizeof(aes_key));
    length += sizeof(aes_key);

    assert(0 == mbedtls_pk_encrypt(&client_ctx, buf, length, cipher, &olen,
                                   RSA_SIZE, mbedtls_ctr_drbg_random, &ctr_drbg));

    length = RSA_SIZE;
    assert(client_write(cipher, length));
}

bool session_init(void)
{
    bool status = false;

    if (communication_init())
    {
        // HMAC-SHA256
        mbedtls_md_init(&hmac_ctx);

        if (0 == mbedtls_md_setup(&hmac_ctx, mbedtls_md_info_from_type(MBEDTLS_MD_SHA256), 1))
        {
            // AES-256
            mbedtls_aes_init(&aes_ctx);

            uint8_t initial[AES_SIZE]{0};
            mbedtls_entropy_init(&entropy);
            mbedtls_ctr_drbg_init(&ctr_drbg);
            for (size_t i = 0; i < sizeof(initial); i++)
            {
                initial[i] = random(0x100);
            }

            if (0 == mbedtls_ctr_drbg_seed(&ctr_drbg, mbedtls_entropy_func, &entropy, initial, sizeof(initial)))
            {
                // RSA-2048
                mbedtls_pk_init(&server_ctx);
                if (0 == mbedtls_pk_setup(&server_ctx, mbedtls_pk_info_from_type(MBEDTLS_PK_RSA)))
                {
                    status = (0 == mbedtls_rsa_gen_key(mbedtls_pk_rsa(server_ctx), mbedtls_ctr_drbg_random, &ctr_drbg, RSA_SIZE * CHAR_BIT, EXPONENT));
                }
            }
        }
    }

    return status;
}

int session_request(void)
{
    uint8_t response = STATUS_OKAY;
    int request = SESSION_OKAY;
    uint8_t buffer[DER_SIZE + RSA_SIZE] = {0};

    size_t length = client_read(buffer, sizeof(buffer));

    if (length == DER_SIZE)
    {
        exchange_public_keys(buffer);
    }
    else if (length == 2 * RSA_SIZE)
    {
        session_establish(buffer);
    }
    else if (length == AES_BLOCK_SIZE)
    {
        if (session_id != 0)
        {
            uint8_t temp[AES_BLOCK_SIZE]{0};

            if (0 == mbedtls_aes_crypt_cbc(&aes_ctx, MBEDTLS_AES_DECRYPT, AES_BLOCK_SIZE, dec_iv, buffer, temp))
            {
                if (temp[AES_BLOCK_SIZE - 1] == 9)
                {
                    if (0 == memcmp(&session_id, &temp[1], sizeof(session_id)))
                    {
                        switch (temp[0])
                        {
                        case SESSION_CLOSE:
                            session_id = 0;
                            break;

                        case SESSION_TOGGLE_LED:
                            response = LED_TOGGLED;
                            break;
                        case SESSION_TEMPERATURE:
                            request = (int)temp[0];
                            break;
                        default:
                            request = SESSION_ERROR;
                            response = STATUS_BAD_REQUEST;
                            break;
                        }
                    }
                    else
                    {
                        request = SESSION_ERROR;
                        response = STATUS_INVALID_SESSION;
                    }
                }
                else
                {
                    request = SESSION_ERROR;
                    response = STATUS_BAD_REQUEST;
                }
            }
            else
            {
                request = SESSION_ERROR;
                response = STATUS_ERROR;
            }
        }
        else
        {
            request = SESSION_ERROR;
            response = STATUS_INVALID_SESSION;
        }
    }
    else
    {
        request = SESSION_ERROR;
        response = STATUS_HASH_ERROR;
    }

    if ((request == SESSION_CLOSE) || (request == SESSION_ERROR))
    {
        assert(session_response(&response, sizeof(response)));
    }

    return request;
}

bool session_response(const uint8_t *res, size_t size)
{
    bool status = false;
    uint8_t response[AES_BLOCK_SIZE] = {0};
    uint8_t buffer[AES_BLOCK_SIZE + HASH_SIZE] = {0};

    switch (res[0])
    {
    case SESSION_OKAY:
        response[0] = STATUS_OKAY;
        break;
    case SESSION_ERROR:
        response[0] = STATUS_ERROR;
        break;
    default:
        response[0] = res[0];
        break;
    }

    if (size > 1)
    {
        memcpy(response + 1, res + 1, size - 1);
    }

    if (0 == mbedtls_aes_crypt_cbc(&aes_ctx, MBEDTLS_AES_ENCRYPT, sizeof(response), enc_iv, response, buffer))
    {
        status = client_write(buffer, AES_BLOCK_SIZE);
    }

    return status;
}
