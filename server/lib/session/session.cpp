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
#include "communication.h"
#include <Arduino.h>
#include <mbedtls/md.h>
#include <mbedtls/pk.h>
#include <mbedtls/rsa.h>
#include <mbedtls/aes.h>
#include <mbedtls/entropy.h>
#include <mbedtls/ctr_drbg.h>

/**
 * @brief Session request types
 *
 */
enum
{
    TOGGLE_LED,
    CLOSE_SESSION,
    GET_TEMPERATURE
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
    /* Wait for data */
    while (0 == Serial.available())
    {
        ;
    }

    /* Read the data */
    size_t length = Serial.readBytes(buffer, blen);
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
{

    bool status{false};                                         /**< Status */
    mbedtls_md_hmac_starts(&hmacContext, hmac_hash, HASH_SIZE); /**< Start the HMAC */
    mbedtls_md_hmac_update(&hmacContext, buffer, dlen);         /**< Update the HMAC */
    mbedtls_md_hmac_finish(&hmacContext, buffer + dlen);        /**< Finish the HMAC */
    dlen += HASH_SIZE;

    /* Write the data */
    if (dlen == Serial.write(buffer, dlen))
    {
        Serial.flush(); /**< Flush the buffer */
        status = true;  /**< Set the status */
    }
    return status; /**< Return the status */
}

static void exchange_public_keys(uint8_t *buffer)
{
    sessionId = 0;                                         /**< Initial session ID */
    size_t olen, length;                                   /**< Output length */
    mbedtls_pk_init(&clientPublicKeyContext);              /**< Initialize the client public key context */
    uint8_t encryptedData[3 * RSA_SIZE + HASH_SIZE] = {0}; /**< Encrypted data */

    /* Parse the public key */
    if (0 != mbedtls_pk_parse_public_key(&clientPublicKeyContext, buffer, DER_SIZE))
    {
        /* Do Nothing */
    }

    /* Check the public key type */
    if (MBEDTLS_PK_RSA != mbedtls_pk_get_type(&clientPublicKeyContext))
    {
        /* Do Nothing */
    }

    /* Write the public key */
    if (DER_SIZE != mbedtls_pk_write_pubkey_der(&server_ctx, buffer, DER_SIZE))
    {
        /* Do Nothing */
    }

    /* Encrypt the data */
    if (0 != mbedtls_pk_encrypt(&clientPublicKeyContext, buffer, DER_SIZE / 2, encryptedData,
                                &olen, RSA_SIZE, mbedtls_ctr_drbg_random, &ctr_drbg))
    {
        /* Do Nothing */
    }

    /* Encrypt the data */
    if (0 != mbedtls_pk_encrypt(&clientPublicKeyContext, buffer + DER_SIZE / 2, DER_SIZE / 2,
                                encryptedData + RSA_SIZE, &olen, RSA_SIZE, mbedtls_ctr_drbg_random, &ctr_drbg))
    {
        /* Do Nothing */
    }
    length = 2 * RSA_SIZE; /**< Set the length */

    /* Write the encrypted data */
    if (!client_write(encryptedData, length))
    {
        /* Do Nothing */
    }
    length = client_read(encryptedData, sizeof(encryptedData)); /**< Read the data */

    /* Check length */
    if (!length == 3 * RSA_SIZE)
    {
        /* Do Nothing */
    }

    /* Decrypt the data */
    if (0 != mbedtls_pk_decrypt(&server_ctx, encryptedData, RSA_SIZE, buffer, &olen, RSA_SIZE,
                                mbedtls_ctr_drbg_random, &ctr_drbg))
    {
        /* Do Nothing */
    }
    length = olen; /**< Set the length */

    /* Decrypt the data */
    if (0 != mbedtls_pk_decrypt(&server_ctx, encryptedData + RSA_SIZE, RSA_SIZE, buffer + length,
                                &olen, RSA_SIZE, mbedtls_ctr_drbg_random, &ctr_drbg))
    {
        /* Do Nothing */
    }
    length += olen; /**< Set the length */

    /* Decrypt data*/
    if (0 != mbedtls_pk_decrypt(&server_ctx, encryptedData + 2 * RSA_SIZE, RSA_SIZE, buffer + length,
                                &olen, RSA_SIZE, mbedtls_ctr_drbg_random, &ctr_drbg))
    {
        /* Do Nothing */
    }
    length += olen; /**< Set the length */

    /* Check the length */
    if (length != (DER_SIZE + RSA_SIZE))
    {
        /* Do Nothing */
    }

    /* Verify the data */
    mbedtls_pk_init(&clientPublicKeyContext);
    if (0 != mbedtls_pk_parse_public_key(&clientPublicKeyContext, buffer, DER_SIZE))
    {
        /* Do Nothing */
    }

    /* Check the public key type */
    if (MBEDTLS_PK_RSA != mbedtls_pk_get_type(&clientPublicKeyContext))
    {
        /* Do Nothing */
    }

    /* Verify the data */
    if (0 != mbedtls_pk_verify(&clientPublicKeyContext, MBEDTLS_MD_SHA256, hmac_hash, HASH_SIZE, buffer + DER_SIZE, RSA_SIZE))
    {
        /* Do nothing */
    }
    strcpy((char *)buffer, "OKAY"); /**< Copy the buffer */

    /* Encrypt the data */
    if (0 != mbedtls_pk_encrypt(&clientPublicKeyContext, buffer, strlen((const char *)buffer),
                                encryptedData, &olen, RSA_SIZE, mbedtls_ctr_drbg_random, &ctr_drbg))
    {
        /* Do Nothing */
    }
    length = RSA_SIZE; /**< Set the length */

    /* Write the data */
    if (!client_write(encryptedData, length))
    {
        /* Do Nothing */
    }
}
static void establish_session(uint8_t *buffer)
{
    sessionId = 0;                      /**< Initial session ID */
    size_t olen, length;                /**< Output length */
    uint8_t encryptedData[RSA_SIZE]{0}; /**< Encrypted data */

    /* Decrypt the data */
    if (0 != mbedtls_pk_decrypt(&server_ctx, buffer, RSA_SIZE, encryptedData, &olen,
                                RSA_SIZE, mbedtls_ctr_drbg_random, &ctr_drbg))
    {
        /* Do Nothing */
    }
    length = olen; /**< Set the length */

    /* Decrypt the data */
    if (0 != mbedtls_pk_decrypt(&server_ctx, buffer + RSA_SIZE, RSA_SIZE, encryptedData + length,
                                &olen, RSA_SIZE, mbedtls_ctr_drbg_random, &ctr_drbg))
    {
        /* Do Nothing */
    }
    length += olen; /**< Set the length */

    /* Check the lenght of the Data*/
    if (length != RSA_SIZE)
    {
        /* Do Nothing */
    }

    /* Verify the data */
    if (0 != mbedtls_pk_verify(&clientPublicKeyContext, MBEDTLS_MD_SHA256, hmac_hash, HASH_SIZE, encryptedData, RSA_SIZE))
    {
        /* Do Nothing */
    }

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
    if (0 != mbedtls_aes_setkey_enc(&aesEncryptionContext, aes_key, sizeof(aes_key) * CHAR_BIT))
    {
        /* Do Nothing */
    }

    memcpy(buffer, &sessionId, sizeof(sessionId));                                                   /**< Copy the session ID */
    length = sizeof(sessionId);                                                                      /**< Set the length */
    memcpy(buffer + length, encryptionInitializationVector, sizeof(encryptionInitializationVector)); /**< Copy the IV */
    length += sizeof(encryptionInitializationVector);                                                /**< Set the length */
    memcpy(buffer + length, aes_key, sizeof(aes_key));                                               /**< Copy the AES Key */
    length += sizeof(aes_key);                                                                       /**< Set the length */
    /* Encrypt the data */
    if (0 != mbedtls_pk_encrypt(&clientPublicKeyContext, buffer, length, encryptedData, &olen,
                                RSA_SIZE, mbedtls_ctr_drbg_random, &ctr_drbg))
    {
        /* Do Nothing */
    }
    length = RSA_SIZE; /**< Set the length */

    /* Write the data */
    if (client_write(encryptedData, length) == 0)
    {
        /* Do Nothing */
    }
}

bool session_init(void)
{
    bool status = true;           /**< Status Flag */
    uint8_t initial[AES_SIZE]{0}; /**< Initial Variable  */

    mbedtls_md_init(&hmacContext); /**< Initialize the HMAC Context */

    /* HMAC */
    if (0 != mbedtls_md_setup(&hmacContext, mbedtls_md_info_from_type(MBEDTLS_MD_SHA256), 1))
    {
        status = false; /**< Set the status */
        goto end_init;  /**< End the initialization */
    }

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
    if (0 != mbedtls_ctr_drbg_seed(&ctr_drbg, mbedtls_entropy_func, &entropy, initial, sizeof(initial)))
    {
        status = false; /**< Set the status */
        goto end_init;  /**< End the initialization */
    }

    /* RSA-2048 */
    mbedtls_pk_init(&server_ctx); /**< Initialize the Server Context */

    /* Setup the RSA */
    if (0 != mbedtls_pk_setup(&server_ctx, mbedtls_pk_info_from_type(MBEDTLS_PK_RSA)))
    {
        status = false; /**< Set the status */
        goto end_init;  /**< End the initialization */
    }

    /* Generate the RSA Key */
    if (0 != mbedtls_rsa_gen_key(mbedtls_pk_rsa(server_ctx), mbedtls_ctr_drbg_random,
                                 &ctr_drbg, RSA_SIZE * CHAR_BIT, EXPONENT))
    {
        status = false; /**< Set the status */
        goto end_init;  /**< End the initialization */
    }

end_init:
    return status; /**< Return the status */
}

int session_request(void)
{
    int request = SESSION_REQ_OKAY;                      /**< Initial Request value */
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
                        case GET_TEMPERATURE:
                            request = SESSION_REQ_TEMPERATURE; /**< Set the request for Temperature */
                            break;
                        case TOGGLE_LED:
                            request = SESSION_REQ_TOGGLE_LED; /**< Set the request for LED Toggle */
                            break;
                        case CLOSE_SESSION:
                            sessionId = 0;                       /**< Set the session ID */
                            request = SESSION_REQ_OKAY;          /**< Set the request */
                            encryptedData[0] = SESSION_RES_OKAY; /**< Set the response */
                            session_response(encryptedData, 1);  /**< Respond to the request */

                            break;
                        default:
                            request = SESSION_REQ_ERROR;                /**< Set the request */
                            encryptedData[0] = SESSION_RES_BAD_REQUEST; /**< Set the response */
                            session_response(encryptedData, 1);         /**< Respond to the request */

                            break;
                        }
                    }
                    else
                    {
                        request = SESSION_REQ_ERROR;            /**< Set the request for Invalid Session  */
                        encryptedData[0] = SESSION_RES_INVALID; /**< Set the response */
                        session_response(encryptedData, 1);     /**< Respond to the request */
                    }
                }
            }
        }
    }
    else
    {
        request = SESSION_RES_HASH_ERROR; /**< Set the request for Hash Error */
    }

    return request; /**< Return the request */
}

bool session_response(const uint8_t *res, size_t size)
{
    bool status = false;                              /**< Status */
    uint8_t buffer[AES_BLOCK_SIZE + HASH_SIZE] = {0}; /**< Buffer */

    /* Encrypt the data */
    if (0 == mbedtls_aes_crypt_cbc(&aesEncryptionContext, MBEDTLS_AES_ENCRYPT, size,
                                   encryptionInitializationVector, res, buffer))
    {
        /* Set the data */
        if (client_write(buffer, AES_BLOCK_SIZE))
        {
            status = true; /**< Set the status */
        }
    }

    return status; /**< Return the status */
}