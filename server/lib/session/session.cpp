/**
 * @file session.cpp
 * @author Oliver Joisten (contact@oliver-joisten.se)
 * @brief 
 * @version 0.1
 * @date 2024-06-05
 * 
 * @copyright Copyright (c) 2024
 * 
 */

    /* Includes ------------------------------------------------------------------*/

#include "session.h"
#include <Arduino.h>
#include <mbedtls/md.h>
#include <mbedtls/pk.h>
#include <mbedtls/rsa.h>
#include <mbedtls/aes.h>
#include "communication.h"
#include <mbedtls/entropy.h>
#include <mbedtls/ctr_drbg.h>

    /* Private define ------------------------------------------------------------*/


    /* Private typedef -----------------------------------------------------------*/

    /**
     * @brief The status of the session
     *
     */
    enum
    {
        STATUS_OKAY,
        STATUS_ERROR,
        STATUS_EXPIRED,
        STATUS_HASH_ERROR,
        STATUS_BAD_REQUEST,
        STATUS_INVALID_SESSION,
    };

    /* Private macro -------------------------------------------------------------*/

#define SESSION_CLOSE 0xFF /**< Close the session */

    /* Private variables ---------------------------------------------------------*/

    constexpr int AES_SIZE{32};       /**< AES key size */
    constexpr int DER_SIZE{294};      /**< DER size */
    constexpr int RSA_SIZE{256};      /**< RSA size */
    constexpr int HASH_SIZE{32};      /**< Hash size */
    constexpr int EXPONENT{65537};    /**< Exponent */
    constexpr int AES_BLOCK_SIZE{16}; /**< AES block size */
    constexpr int KEEP_ALIVE{30000};  /**< Keep alive time */

    static mbedtls_aes_context aes_ctx;       /**< AES context */
    static mbedtls_md_context_t hmac_ctx;     /**< HMAC context */
    static mbedtls_pk_context client_ctx;     /**< Client context */
    static mbedtls_pk_context server_ctx;     /**< Server context */
    static mbedtls_entropy_context entropy;   /**< Entropy context */
    static mbedtls_ctr_drbg_context ctr_drbg; /**< CTR DRBG context */

    static uint64_t session_id{0};            /**< Session ID */
    static uint32_t accessed{0};              /**< Accessed Timer */
    static uint8_t aes_key[AES_SIZE]{0};      /**< AES key */
    static uint8_t enc_iv[AES_BLOCK_SIZE]{0}; /**< Encryption IV */
    static uint8_t dec_iv[AES_BLOCK_SIZE]{0}; /**< Decryption IV */

    /*Secret Key in Hexadecimal values */
    static const uint8_t secret_key[HASH_SIZE] = {0x29, 0x49, 0xde, 0xc2, 0x3e, 0x1e, 0x34, 0xb5, 0x2d, 0x22, 0xb5,
                                                  0xba, 0x4c, 0x34, 0x23, 0x3a, 0x9d, 0x3f, 0xe2, 0x97, 0x14, 0xbe,
                                                  0x24, 0x62, 0x81, 0x0c, 0x86, 0xb1, 0xf6, 0x92, 0x54, 0xd6};

    /* Static Assertions ---------------------------------------------------------*/


    /* Private function prototypes -----------------------------------------------*/

    bool session_response(const uint8_t *res, size_t size); /**< Respond to a session */

    /* Private user code ---------------------------------------------------------*/
    static size_t client_read(uint8_t *buf, size_t blen)
    {
        size_t length = communication_read(buf, blen); /**< Read the data from the communication module */

        /* Check if the length is greater than the hash size */
        if (length > HASH_SIZE)
        {
            length -= HASH_SIZE;                                        /**< Subtract the hash size from the length */
            uint8_t hmac[HASH_SIZE]{0};                                 /**< The HMAC buffer */
            mbedtls_md_hmac_starts(&hmac_ctx, secret_key, HASH_SIZE);   /**< Start the HMAC */
            mbedtls_md_hmac_update(&hmac_ctx, buf, length);             /**< Update the HMAC */
            mbedtls_md_hmac_finish(&hmac_ctx, hmac);                    /**< Finish the HMAC */

            /* Check if the HMAC is not equal to the buffer */
            if (0 != memcmp(hmac, buf + length, HASH_SIZE))
            {
                length = 0; /**< Set the length to 0 */
            }
        }
        else
        {
            length = 0; /**< Set the length to 0 */
        }

        return length; /**< Return the length */
}

static bool client_write(uint8_t *buf, size_t dlen)
{
    mbedtls_md_hmac_starts(&hmac_ctx, secret_key, HASH_SIZE); /**< Start the HMAC */
    mbedtls_md_hmac_update(&hmac_ctx, buf, dlen);            /**< Update the HMAC */
    mbedtls_md_hmac_finish(&hmac_ctx, buf + dlen);          /**< Finish the HMAC */
    dlen += HASH_SIZE;                                     /**< Add the hash size to the length */

    return communication_write(buf, dlen); /**< Write the data to the communication module */
}

/**
 * @brief Performs the exchange of public keys between the server and the client.
 * 
 * This function initializes the session ID to 0 and performs the exchange of public keys
 * between the server and the client. It encrypts and decrypts the data using RSA encryption
 * and verifies the signature of the client's public key.
 * 
 * @param buf Pointer to the buffer containing the public key.
 */
static void exchange_public_keys(uint8_t *buf)
{
    session_id = 0;         /**< Initialize the session ID to 0 */
    size_t olen, length;    /**< The output length and the length */

    mbedtls_pk_init(&client_ctx);                   /**< Initialize the client context */
    uint8_t cipher[3 * RSA_SIZE + HASH_SIZE] = {0}; /**< Initialize the cipher buffer */

    assert(0 == mbedtls_pk_parse_public_key(&client_ctx, buf, DER_SIZE)); /**< Parse the public key */
    assert(MBEDTLS_PK_RSA == mbedtls_pk_get_type(&client_ctx));           /**< Check the type */

    assert(DER_SIZE == mbedtls_pk_write_pubkey_der(&server_ctx, buf, DER_SIZE)); /**< Write the public key */

    /* Encrypt the buffer with halv DER_SIZE */
    assert(0 == mbedtls_pk_encrypt(&client_ctx, buf, DER_SIZE / 2, cipher,
                                   &olen, RSA_SIZE, mbedtls_ctr_drbg_random, &ctr_drbg));

    /* Encrypting the second half of the buffer using the client's public key and storing the result in the second half of the cipher buffer */
    assert(0 == mbedtls_pk_encrypt(&client_ctx, buf + DER_SIZE / 2, DER_SIZE / 2,
                                   cipher + RSA_SIZE, &olen, RSA_SIZE, mbedtls_ctr_drbg_random, &ctr_drbg));

    length = 2 * RSA_SIZE;                  /**< Set the length to 2 times the RSA size */
    assert(client_write(cipher, length));   /**< Write the cipher to the client */

    length = client_read(cipher, sizeof(cipher)); /**< Read the cipher from the client */
    assert(length == 3 * RSA_SIZE);              /**< Check the length */

    /* Decrypt the first part of the cipher buffer */
    assert(0 == mbedtls_pk_decrypt(&server_ctx, cipher, RSA_SIZE, buf, &olen, RSA_SIZE,
                                   mbedtls_ctr_drbg_random, &ctr_drbg));

    length = olen; /**< Set the length to the output length */

    /* Decrypt the second part of the cipher buffer */
    assert(0 == mbedtls_pk_decrypt(&server_ctx, cipher + RSA_SIZE, RSA_SIZE, buf + length,
                                   &olen, RSA_SIZE, mbedtls_ctr_drbg_random, &ctr_drbg));

    length += olen; /**< Add the output length to the length */

    /* Check if the length is equal to the DER size plus the RSA size */
    assert(0 == mbedtls_pk_decrypt(&server_ctx, cipher + 2 * RSA_SIZE, RSA_SIZE, buf + length,
                                   &olen, RSA_SIZE, mbedtls_ctr_drbg_random, &ctr_drbg));

    length += olen;                          /**< Add the output length to the length */
    assert(length == (DER_SIZE + RSA_SIZE)); /**< Check if the length is equal to the DER size plus the RSA size */

    mbedtls_pk_init(&client_ctx);                                         /**< Initialize the client context */
    assert(0 == mbedtls_pk_parse_public_key(&client_ctx, buf, DER_SIZE)); /**< Parse the public key */
    assert(MBEDTLS_PK_RSA == mbedtls_pk_get_type(&client_ctx));           /**< Check the type */

    /* Verify the signature */
    assert(0 == mbedtls_pk_verify(&client_ctx, MBEDTLS_MD_SHA256, secret_key, HASH_SIZE, buf + DER_SIZE, RSA_SIZE));

    strcpy((char *)buf, "OKAY"); /**< Copy the string to the buffer */

    /* Encrypting the buffer using the client's public key and storing the result in the cipher buffer*/
    assert(0 == mbedtls_pk_encrypt(&client_ctx, buf, strlen((const char *)buf),
                                   cipher, &olen, RSA_SIZE, mbedtls_ctr_drbg_random, &ctr_drbg));

    length = RSA_SIZE;                    /**< Set the length to the RSA size */
    assert(client_write(cipher, length)); /**< Write the cipher to the client */
}

/**
 * @brief Establishes a session by decrypting the buffer, generating a random session ID, encryption IV, and AES key,
 *        and encrypting the buffer using the client's public key.
 *
 * @param buf The buffer containing the encrypted data.
 */
static void session_establish(uint8_t *buf)
{
    session_id = 0;                 /**< Initialize the session ID to 0 */
    size_t olen, length;            /**< The output length and the length */
    uint8_t cipher[RSA_SIZE]{0};    /**< The cipher buffer */

    /* Decrypt the first part of the buffer */
    assert(0 == mbedtls_pk_decrypt(&server_ctx, buf, RSA_SIZE, cipher, &olen,
                                   RSA_SIZE, mbedtls_ctr_drbg_random, &ctr_drbg));

    length = olen; /**< Set the length to the output length */

    /* Decrypt the second part of the buffer */
    assert(0 == mbedtls_pk_decrypt(&server_ctx, buf + RSA_SIZE, RSA_SIZE, cipher + length,
                                   &olen, RSA_SIZE, mbedtls_ctr_drbg_random, &ctr_drbg));

    length += olen;                 /**< Add the output length to the length */
    assert(length == RSA_SIZE);     /**< Check if the length is equal to the RSA size */

    /* Decrypt the third part of the buffer */
    assert(0 == mbedtls_pk_verify(&client_ctx, MBEDTLS_MD_SHA256, secret_key, HASH_SIZE, cipher, RSA_SIZE));

    /* Generating a random session ID by assigning each byte of the session_id a random number between 1 and 0x100 */
    uint8_t *ptr{(uint8_t *)&session_id};
    for (size_t i = 0; i < sizeof(session_id); i++)
    {
        ptr[i] = random(1, 0x100); /**< Generate a random number for the SESSION_ID */
    }

    /* Generating a random initialization vector (IV) for encryption and copying it to the decryption IV */
    for (size_t i = 0; i < sizeof(enc_iv); i++)
    {
        enc_iv[i] = random(0x100);
    }
    memcpy(dec_iv, enc_iv, sizeof(dec_iv));

    /*Generating a random AES key by assigning each byte of the aes_key a random number between 0 and 0x100 */
    for (size_t i = 0; i < sizeof(aes_key); i++)
    {
        aes_key[i] = random(0x100);
    }

    /* Setting the AES key for encryption */
    assert(0 == mbedtls_aes_setkey_enc(&aes_ctx, aes_key, sizeof(aes_key) * CHAR_BIT));

    memcpy(buf, &session_id, sizeof(session_id)); /**< Copy the session ID to the buffer */
    length = sizeof(session_id);                  /**< Set the length to the size of the session ID */

    memcpy(buf + length, enc_iv, sizeof(enc_iv)); /**< Copy the encryption IV to the buffer */
    length += sizeof(enc_iv);                    /**< Add the size of the encryption IV to the length */

    memcpy(buf + length, aes_key, sizeof(aes_key)); /**< Copy the AES key to the buffer */
    length += sizeof(aes_key);                      /**< Add the size of the AES key to the length */

    /* Encrypting the buffer using the client's public key and storing the result in the cipher buffer */
    assert(0 == mbedtls_pk_encrypt(&client_ctx, buf, length, cipher, &olen,
                                   RSA_SIZE, mbedtls_ctr_drbg_random, &ctr_drbg));

    length = RSA_SIZE;                    /**< Set the length to the RSA size */
    assert(client_write(cipher, length)); /**< Write the cipher to the client */
}

/* Exported user code --------------------------------------------------------*/

bool session_init()
{
    bool status = false; /**< The status of the session */

    /* Initialize the communication module */
    if (communication_init())
    {
        mbedtls_md_init(&hmac_ctx); /**< Initialize the HMAC context */

        /* Setup the HMAC context */
        if (0 == mbedtls_md_setup(&hmac_ctx, mbedtls_md_info_from_type(MBEDTLS_MD_SHA256), 1))
        {
            mbedtls_aes_init(&aes_ctx); /**< Initialize the AES context */

            uint8_t initial[AES_SIZE]{0};       /**< The initial buffer */
            mbedtls_entropy_init(&entropy);     /**< Initialize the entropy */
            mbedtls_ctr_drbg_init(&ctr_drbg);   /**< Initialize the CTR DRBG */

            /* Generating a random initial array by assigning each byte a random number between 0 and 0x100*/
            for (size_t i = 0; i < sizeof(initial); i++)
            {
                initial[i] = random(0x100);
            }

            /* Seed the CTR DRBG */
            if (0 == mbedtls_ctr_drbg_seed(&ctr_drbg, mbedtls_entropy_func, &entropy, initial, sizeof(initial)))
            {
                /* RSA-2048 */
                mbedtls_pk_init(&server_ctx); /**< Initialize the server context */

                /* Setup the server context */
                if (0 == mbedtls_pk_setup(&server_ctx, mbedtls_pk_info_from_type(MBEDTLS_PK_RSA)))
                {
                    /* Generate the RSA key */
                    status = (0 == mbedtls_rsa_gen_key(mbedtls_pk_rsa(server_ctx), mbedtls_ctr_drbg_random, &ctr_drbg, RSA_SIZE * CHAR_BIT, EXPONENT));
                }
            }
        }
    }

    return status; /**< Return the status */
}

int session_request(void)
{
    uint8_t response = STATUS_OKAY;            /**< The response */
    int request = SESSION_OKAY;                /**< The request */
    uint8_t buffer[DER_SIZE + RSA_SIZE] = {0}; /**< The buffer */
    bool session_expired = false;              /**< The session expired */

    size_t length = client_read(buffer, sizeof(buffer)); /**< Read the buffer from the client */

    /* Checking the length of the received data and handling it accordingly:
     *- If it's equal to DER_SIZE, it's a public key exchange request
     *- If it's equal to twice the RSA_SIZE, it's a session establishment request
     *- If it's equal to AES_BLOCK_SIZE, it's a session request, which is decrypted and handled based on its content
     *- If it's none of the above, it's treated as an error
     */
    if (length == DER_SIZE)
    {
        exchange_public_keys(buffer); /**< Exchange the public keys */
    }
    else if (length == 2 * RSA_SIZE)
    {
        session_establish(buffer); /**< Establish the session */
    }
    else if (length == AES_BLOCK_SIZE)
    {
        if (session_id != 0)
        {
            uint32_t timer = millis(); /**< Timer in milli seconds */
            if (timer - accessed <= KEEP_ALIVE)
            {
                accessed = timer; /**< Set the accessed timer to the timer */

                uint8_t temp[AES_BLOCK_SIZE]{0}; /**< The temporary buffer */

                if (0 == mbedtls_aes_crypt_cbc(&aes_ctx, MBEDTLS_AES_DECRYPT, AES_BLOCK_SIZE, dec_iv, buffer, temp))
                {
                    if (temp[AES_BLOCK_SIZE - 1] == 9)
                    {
                        if (0 == memcmp(&session_id, &temp[1], sizeof(session_id)))
                        {
                            switch (temp[0])
                            {
                            case SESSION_CLOSE:
                            case SESSION_TOGGLE_LED:
                            case SESSION_TEMPERATURE:
                                request = (int)temp[0];
                                break;
                            default:
                                response = STATUS_BAD_REQUEST;
                                break;
                            }
                        }
                        else
                        {
                            response = STATUS_INVALID_SESSION;
                        }
                    }
                    else
                    {
                        response = STATUS_BAD_REQUEST;
                    }
                }
                else
                {
                    response = STATUS_ERROR;
                }
            }
            else
            {
                session_id = 0;
                response = STATUS_EXPIRED;
            }
        }
        else
        {
            response = STATUS_INVALID_SESSION;
        }
    }
    else
    {
        response = STATUS_HASH_ERROR;
    }

    /* Respond to the request */
    if ((request == SESSION_CLOSE) || (request == SESSION_ERROR))
    {
        assert(session_response(&response, sizeof(response)));
    }

    return request;
}

bool session_response(const uint8_t *res, size_t size)
{
    bool status = false;
    size_t length = 1;
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
    case STATUS_EXPIRED:
        response[0] = STATUS_EXPIRED;
        break;
    default:
        response[0] = res[0];
        break;
    }

    if (size > 1)
    {
        memcpy(response + length, res + 1, size - 1);
    }

    if (0 == mbedtls_aes_crypt_cbc(&aes_ctx, MBEDTLS_AES_ENCRYPT, sizeof(response), enc_iv, response, buffer))
    {
        status = client_write(buffer, AES_BLOCK_SIZE);
    }

    return status;
}
