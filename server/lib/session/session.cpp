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
};

constexpr int AES_SIZE{32};       /**< AES Size */
constexpr int DER_SIZE{294};      /**< DER Size */
constexpr int RSA_SIZE{256};      /**< RSA Size */
constexpr int HASH_SIZE{32};      /**< Hash Size */
constexpr int EXPONENT{65537};    /**< Exponent */
constexpr int AES_BLOCK_SIZE{16}; /**< AES Block Size */

static mbedtls_aes_context aesEncryptionContext;                  /**< AES Context */
static mbedtls_md_context_t hmacContext;                          /**< HMAC Context */
static mbedtls_pk_context clientPublicKeyContext;                 /**< Client Public Key Context */
static mbedtls_pk_context server_ctx;                             /**< Server Context */
static mbedtls_entropy_context entropy;                           /**< Entropy Context */
static mbedtls_ctr_drbg_context ctr_drbg;                         /**< CTR DRBG Context */
static uint64_t sessionId{0};                                     /**< Session ID */
static uint8_t aes_key[AES_SIZE]{0};                              /**< AES Key */
static uint8_t encryptionInitializationVector[AES_BLOCK_SIZE]{0}; /**< Encryption Initialization Vector */
static uint8_t decryptionInitializationVector[AES_BLOCK_SIZE]{0}; /**< Decryption Initialization Vector */

/* HMAC Secret */
static const uint8_t hmac_hash[HASH_SIZE] = {0x29, 0x49, 0xde, 0xc2, 0x3e, 0x1e, 0x34, 0xb5, 0x2d, 0x22, 0xb5,
                                             0xba, 0x4c, 0x34, 0x23, 0x3a, 0x9d, 0x3f, 0xe2, 0x97, 0x14, 0xbe,
                                             0x24, 0x62, 0x81, 0x0c, 0x86, 0xb1, 0xf6, 0x92, 0x54, 0xd6};

size_t client_read(uint8_t *buffer, size_t blen)
{
    /* Read the data */
    size_t length = communication_read(buffer, blen);

    if (length > HASH_SIZE)
    {
        /* Verify the HMAC */
        length -= HASH_SIZE;
        uint8_t hmac[HASH_SIZE]{0};
        mbedtls_md_hmac_starts(&hmacContext, hmac_hash, HASH_SIZE);
        mbedtls_md_hmac_update(&hmacContext, buffer, length);
        mbedtls_md_hmac_finish(&hmacContext, hmac);

        /* Check the HMAC */
        if (0 != memcmp(hmac, buffer + length, HASH_SIZE))
        {
            length = 0; /**< HMAC Error */
        }
    }
    else
    {
        length = 0; /**< HMAC Error */
    }
    return length; /**< Return the length */
}

bool client_write(uint8_t *buffer, size_t dlen)
{                                                               /**< Status */
    mbedtls_md_hmac_starts(&hmacContext, hmac_hash, HASH_SIZE); /**< Start the HMAC */
    mbedtls_md_hmac_update(&hmacContext, buffer, dlen);         /**< Update the HMAC */
    mbedtls_md_hmac_finish(&hmacContext, buffer + dlen);        /**< Finish the HMAC */
    dlen += HASH_SIZE;

    return (dlen == communication_write(buffer, dlen)); /**< Return the status */
}

static void exchange_public_keys(uint8_t *buffer)
{
    sessionId = 0;                                         /**< Initial session ID */
    size_t olen, length;                                   /**< Output length */
    mbedtls_pk_init(&clientPublicKeyContext);              /**< Initialize the client public key context */
    uint8_t encryptedData[3 * RSA_SIZE + HASH_SIZE] = {0}; /**< Encrypted data */

    /* Parse the public key */
    assert(0 == mbedtls_pk_parse_public_key(&clientPublicKeyContext, buffer, DER_SIZE));

    /* Check the public key type */
    assert(MBEDTLS_PK_RSA == mbedtls_pk_get_type(&clientPublicKeyContext));

    /* Write the public key */
    assert(DER_SIZE == mbedtls_pk_write_pubkey_der(&server_ctx, buffer, DER_SIZE));

    /* Encrypt the data */
    assert(0 == mbedtls_pk_encrypt(&clientPublicKeyContext, buffer, DER_SIZE / 2,
                                   encryptedData, &olen, RSA_SIZE, mbedtls_ctr_drbg_random, &ctr_drbg));

    /* Encrypt the data */
    assert(0 == mbedtls_pk_encrypt(&clientPublicKeyContext, buffer + DER_SIZE / 2, DER_SIZE / 2,
                                   encryptedData + RSA_SIZE, &olen, RSA_SIZE, mbedtls_ctr_drbg_random, &ctr_drbg));

    length = 2 * RSA_SIZE; /**< Set the length */
    /* Write the encrypted data */
    assert(client_write(encryptedData, length));

    length = client_read(encryptedData, sizeof(encryptedData)); /**< Read the data */

    /* Check length */
    assert(length == 3 * RSA_SIZE);

    /* Decrypt the data */
    assert(0 == mbedtls_pk_decrypt(&server_ctx, encryptedData, RSA_SIZE, buffer, &olen, RSA_SIZE,
                                   mbedtls_ctr_drbg_random, &ctr_drbg));

    length = olen; /**< Set the length */

    /* Decrypt the data */
    assert(0 == mbedtls_pk_decrypt(&server_ctx, encryptedData + RSA_SIZE, RSA_SIZE, buffer + length,
                                   &olen, RSA_SIZE, mbedtls_ctr_drbg_random, &ctr_drbg));

    length += olen; /**< Set the length */

    /* Decrypt data*/
    assert(0 == mbedtls_pk_decrypt(&server_ctx, encryptedData + 2 * RSA_SIZE, RSA_SIZE, buffer + length,
                                   &olen, RSA_SIZE, mbedtls_ctr_drbg_random, &ctr_drbg));

    length += olen; /**< Set the length */

    /* Check the length */
    assert(length == (DER_SIZE + RSA_SIZE));

    /* Verify the data */
    mbedtls_pk_init(&clientPublicKeyContext);
    assert(0 == mbedtls_pk_parse_public_key(&clientPublicKeyContext, buffer, DER_SIZE));

    /* Check the public key type */
    assert(MBEDTLS_PK_RSA == mbedtls_pk_get_type(&clientPublicKeyContext));

    /* Verify the data */
    assert(0 == mbedtls_pk_verify(&clientPublicKeyContext, MBEDTLS_MD_SHA256, hmac_hash, HASH_SIZE, buffer + DER_SIZE, RSA_SIZE));

    strcpy((char *)buffer, "OKAY"); /**< Copy the buffer */

    /* Encrypt the data */
    assert(0 == mbedtls_pk_encrypt(&clientPublicKeyContext, buffer, strlen((const char *)buffer),
                                   encryptedData, &olen, RSA_SIZE, mbedtls_ctr_drbg_random, &ctr_drbg));

    length = RSA_SIZE; /**< Set the length */

    /* Write the data */
    assert(client_write(encryptedData, length));
}

static void establish_session(uint8_t *buffer)
{
    sessionId = 0;                      /**< Initial session ID */
    size_t olen, length;                /**< Output length */
    uint8_t encryptedData[RSA_SIZE]{0}; /**< Encrypted data */

    /* Decrypt the data */
    assert(0 == mbedtls_pk_decrypt(&server_ctx, buffer, RSA_SIZE, encryptedData,
                                   &olen, RSA_SIZE, mbedtls_ctr_drbg_random, &ctr_drbg));
    length = olen; /**< Set the length */

    /* Decrypt the data */
    assert(0 == mbedtls_pk_decrypt(&server_ctx, buffer + RSA_SIZE, RSA_SIZE,
                                   encryptedData + length, &olen, RSA_SIZE, mbedtls_ctr_drbg_random, &ctr_drbg));

    length += olen; /**< Set the length */

    /* Check the lenght of the Data*/
    assert(length == RSA_SIZE);

    /* Verify the data */
    assert(0 == mbedtls_pk_verify(&clientPublicKeyContext, MBEDTLS_MD_SHA256, hmac_hash, HASH_SIZE, encryptedData, RSA_SIZE));

    uint8_t *ptr{(uint8_t *)&sessionId}; /**< Pointer to the session */

    /* Check the pointer */
    if (ptr != nullptr)
    {
        /* Generate the session ID */
        for (size_t i = 0; i < sizeof(sessionId); i++)
        {
            ptr[i] = random(1, 0x100); /**< Set a ramdom session ID */
        }

        /* Encrypt the data with IV */
        for (size_t i = 0; i < sizeof(encryptionInitializationVector); i++)
        {
            encryptionInitializationVector[i] = random(0x100); /**< Set a random IV */
        }
    }
    else
    {
        /* Do Nothing */
    }
    /* Copy the IV */
    memcpy(decryptionInitializationVector, encryptionInitializationVector, sizeof(decryptionInitializationVector));

    /* Set a random  aes_key */
    for (size_t i = 0; i < sizeof(aes_key); i++)
    {
        aes_key[i] = random(0x100);
    }

    /* Set the key */
    assert(0 == mbedtls_aes_setkey_enc(&aesEncryptionContext, aes_key, sizeof(aes_key) * CHAR_BIT));

    memcpy(buffer, &sessionId, sizeof(sessionId));                                                   /**< Copy the session ID */
    length = sizeof(sessionId);                                                                      /**< Set the length */
    memcpy(buffer + length, encryptionInitializationVector, sizeof(encryptionInitializationVector)); /**< Copy the IV */
    length += sizeof(encryptionInitializationVector);                                                /**< Set the length */
    memcpy(buffer + length, aes_key, sizeof(aes_key));                                               /**< Copy the AES Key */
    length += sizeof(aes_key);                                                                       /**< Set the length */
    /* Encrypt the data */
    assert(0 == mbedtls_pk_encrypt(&clientPublicKeyContext, buffer, length,
                                   encryptedData, &olen, RSA_SIZE, mbedtls_ctr_drbg_random, &ctr_drbg));

    length = RSA_SIZE; /**< Set the length */

    /* Write the data */
    assert(client_write(encryptedData, length));
}

bool session_init(void)
{
    bool status = true; /**< Status Flag */

    if (communication_init())
    {
        uint8_t initial[AES_SIZE]{0}; /**< Initial Variable  */

        mbedtls_md_init(&hmacContext); /**< Initialize the HMAC Context */

        /* HMAC */
        if (0 == mbedtls_md_setup(&hmacContext, mbedtls_md_info_from_type(MBEDTLS_MD_SHA256), 1))
        {
            /* AES-256 */
            mbedtls_aes_init(&aesEncryptionContext); /**< Initialize the AES Context */
            mbedtls_entropy_init(&entropy);          /**< Initialize the Entropy */
            mbedtls_ctr_drbg_init(&ctr_drbg);        /**< Initialize the CTR DRBG */

            /* Set inital to a ramdom value  */
            for (size_t i = 0; i < sizeof(initial); i++)
            {
                initial[i] = random(0x100);
            }

            /* Seed the CTR DRBG */
            if (0 == mbedtls_ctr_drbg_seed(&ctr_drbg, mbedtls_entropy_func, &entropy, initial, sizeof(initial)))
            {
                /* RSA-2048 */
                mbedtls_pk_init(&server_ctx); /**< Initialize the Server Context */

                /* Setup the RSA */
                if (0 == mbedtls_pk_setup(&server_ctx, mbedtls_pk_info_from_type(MBEDTLS_PK_RSA)))
                {
                    /* Generate the RSA Key */
                    status = (0 == mbedtls_rsa_gen_key(mbedtls_pk_rsa(server_ctx), mbedtls_ctr_drbg_random, &ctr_drbg, RSA_SIZE * CHAR_BIT, EXPONENT));
                }
            }
        }
    }

    return status; /**< Return the status */
}

int session_request(void)
{
    int request = SESSION_OKAY; /**< Initial Request value */
    uint8_t response = STATUS_OKAY;
    uint8_t buffer[DER_SIZE + RSA_SIZE] = {0};           /**< Initialize Buffer */
    size_t length = client_read(buffer, sizeof(buffer)); /**< Read the data */

    /* Check the length */
    if (length == DER_SIZE)
    {
        exchange_public_keys(buffer); /**< Exchange the public keys */
    }
    /* Check the length */
    else if (length == 2 * RSA_SIZE)
    {
        establish_session(buffer); /**< Establish the session */
    }
    /* Check the length */
    else if (length == AES_BLOCK_SIZE)
    {
        /* Decrypt the data */
        if (sessionId != 0)
        {
            uint8_t encryptedData[AES_BLOCK_SIZE]{0}; /**< Initialize variable */

            /* Decrypt the data */
            if (0 == mbedtls_aes_crypt_cbc(&aesEncryptionContext, MBEDTLS_AES_DECRYPT, length, decryptionInitializationVector, buffer, encryptedData))
            {
                /* Check the data */
                if (encryptedData[AES_BLOCK_SIZE - 1] == 9)
                {
                    /* Check the session ID */
                    if (0 == memcmp(&sessionId, &encryptedData[1], sizeof(sessionId)))
                    {
                        /* Switch case that handles the request */
                        switch (encryptedData[0])
                        {
                        case SESSION_CLOSE:
                            sessionId = 0; /**< Set the session ID */
                            break;

                        case SESSION_TEMPERATURE:
                        case SESSION_TOGGLE_LED:
                            request = encryptedData[0]; /**< Set the request for LED Toggle */
                            break;

                        default:
                            request = SESSION_ERROR;       /**< Set the request */
                            response = STATUS_BAD_REQUEST; /**< Set the response */
                            break;
                        }
                    }
                    else
                    {
                        request = SESSION_ERROR;           /**< Set the request for Invalid Session  */
                        response = STATUS_INVALID_SESSION; /**< Set the response */
                    }
                }
                else
                {
                    request = SESSION_ERROR; /**< Set the request for Invalid Session  */
                    response = STATUS_ERROR; /**< Set the response */
                }
            }
            else
            {
                request = SESSION_ERROR; /**< Set the request for Invalid Session  */
                response = STATUS_ERROR; /**< Set the response */
            }
        }
        else
        {
            request = SESSION_ERROR;           /**< Set the request for Invalid Session  */
            response = STATUS_INVALID_SESSION; /**< Set the response */
        }
    }
    else
    {
        // sessionId = 0;
        request = SESSION_ERROR;      /**< Set the request for Invalid Session  */
        response = STATUS_HASH_ERROR; /**< Set the response */
    }

    if ((request == SESSION_ERROR) || (request == SESSION_CLOSE))
    {
        if (!session_response(&response, sizeof(response)))
        {
            request = SESSION_ERROR;
        }
    }

    return request; /**< Return the request */
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

    if (0 == mbedtls_aes_crypt_cbc(&aesEncryptionContext, MBEDTLS_AES_ENCRYPT, sizeof(response), encryptionInitializationVector, response, buffer))
    {
        status = client_write(buffer, AES_BLOCK_SIZE);
    }

    return status;
}