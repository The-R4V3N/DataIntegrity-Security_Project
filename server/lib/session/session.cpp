/**
 * @file session.cpp
 * @brief This file contains the implementation of the session module.
 *        The session module handles the establishment and management of sessions between the server and the client.
 * @author Oliver Joisten (contact@oliver-joisten.se)
 * @version 0.1
 * @date 2024-05-29
 *
 * @details The session module uses various cryptographic algorithms and protocols to ensure secure communication between the server and the client.
 *          It establishes a session by exchanging public keys and encrypting the session ID, initialization vector, and AES key.
 *          The module also provides functions for reading and writing encrypted data during the session.
 *          The session can be closed using the session_close() function.
 *          The session_request() function is used to receive requests from the client and return the corresponding response.
 *          The session_init() function initializes the session module and sets up the necessary cryptographic contexts.
 *          The session_establish() function establishes a session with the client.
 */

/* Includes ------------------------------------------------------------------*/

#include "communication.h"
#include "session.h"
#include <Arduino.h>
#include <mbedtls/md.h>
#include <mbedtls/pk.h>
#include <mbedtls/rsa.h>
#include <mbedtls/aes.h>
#include <mbedtls/entropy.h>
#include <mbedtls/ctr_drbg.h>

/* Private define ------------------------------------------------------------*/

/* Private typedef -----------------------------------------------------------*/
/**
 * @brief The status codes for the session module.
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

constexpr int AES_SIZE{32};         /**< AES Key Size */
constexpr int DER_SIZE{294};        /**< DER Size */
constexpr int RSA_SIZE{256};        /**< RSA Size */
constexpr int HASH_SIZE{32};        /**< Hash Size */
constexpr int EXPONENT{65537};      /**< Exponent */
constexpr int KEEP_ALIVE{60000};    /**< Keep Alive Timer */
constexpr int AES_BLOCK_SIZE{16};   /**< AES Block Size */

/* Private variables ---------------------------------------------------------*/

static mbedtls_aes_context aes_ctx;         /**< AES Context */
static mbedtls_md_context_t hmac_ctx;       /**< HMAC Context */
static mbedtls_pk_context client_ctx;       /**< Client Public Key Context */
static mbedtls_pk_context server_ctx;       /**< Server Public Key Context */
static mbedtls_entropy_context entropy;     /**< Entropy Context */
static mbedtls_ctr_drbg_context ctr_drbg;   /**< CTR DRBG Context */

static uint32_t accessed{0};                        /**< The last time the session was accessed */
static uint64_t session_id{0};                      /**< The session ID */
static uint8_t aes_key[AES_SIZE]{0};                /**< The AES Key */
static uint8_t enc_iv[AES_BLOCK_SIZE]{0};           /**< The Encryption IV */
static uint8_t dec_iv[AES_BLOCK_SIZE]{0};           /**< The Decryption IV */
static uint8_t buffer[DER_SIZE + RSA_SIZE] = {0};   /**< The Buffer */

/* Security Key */
static const uint8_t secret_key[HASH_SIZE] = {0x29, 0x49, 0xde, 0xc2, 0x3e, 0x1e, 0x34, 0xb5, 0x2d, 0x22, 0xb5,
                                              0xba, 0x4c, 0x34, 0x23, 0x3a, 0x9d, 0x3f, 0xe2, 0x97, 0x14, 0xbe,
                                              0x24, 0x62, 0x81, 0x0c, 0x86, 0xb1, 0xf6, 0x92, 0x54, 0xd6};

/* Static Assertions ---------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/

/* Private user code ---------------------------------------------------------*/


/**
 * @brief Reads data from the communication channel and verifies its integrity using HMAC.
 * 
 * This function reads data from the communication channel into the provided buffer and verifies its integrity
 * using HMAC (Hash-based Message Authentication Code). The function calculates the HMAC of the data using a secret key,
 * and compares it with the HMAC appended to the data. If the HMACs match, the function returns the length of the data
 * without the HMAC. Otherwise, it returns 0 to indicate that the data is not valid.
 * 
 * @param buf Pointer to the buffer where the data will be stored.
 * @param blen The maximum length of the buffer.
 * @return The length of the data without the HMAC if the data is valid, 0 otherwise.
 */
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

/**
 * @brief Writes data to the client with HMAC integrity check.
 * 
 * This function calculates the HMAC of the data using the secret key and appends it to the data buffer.
 * The total length of the data buffer is increased by the size of the HMAC.
 * The resulting data is then written to the client using the communication_write function.
 * 
 * @param buf Pointer to the data buffer.
 * @param dlen Length of the data in the buffer.
 * @return True if the write operation was successful, false otherwise.
 */
static bool client_write(uint8_t *buf, size_t dlen)
{
    mbedtls_md_hmac_starts(&hmac_ctx, secret_key, HASH_SIZE);
    mbedtls_md_hmac_update(&hmac_ctx, buf, dlen);
    mbedtls_md_hmac_finish(&hmac_ctx, buf + dlen);
    dlen += HASH_SIZE;

    return communication_write(buf, dlen);
}

/**
 * @brief Performs the exchange of public keys between the server and the client.
 * 
 * This function initializes the session ID, initializes the client context, and performs the exchange
 * of public keys using RSA encryption. It encrypts the client's public key and sends it to the server,
 * and then decrypts the server's public key received from the client. Finally, it verifies the decrypted
 * server's public key with the secret key and encrypts the response message "OKAY" using the client's
 * public key and sends it back to the client.
 * 
 * @note This function assumes that the necessary cryptographic contexts and buffers have been properly
 * initialized before calling this function.
 */
static void exchange_public_keys(void)
{
    session_id = 0;
    size_t olen, length;

    mbedtls_pk_init(&client_ctx);
    uint8_t cipher[3 * RSA_SIZE + HASH_SIZE] = {0};

    assert(0 == mbedtls_pk_parse_public_key(&client_ctx, buffer, DER_SIZE));
    assert(MBEDTLS_PK_RSA == mbedtls_pk_get_type(&client_ctx));

    assert(DER_SIZE == mbedtls_pk_write_pubkey_der(&server_ctx, buffer, DER_SIZE));

    assert(0 == mbedtls_pk_encrypt(&client_ctx, buffer, DER_SIZE / 2, cipher,
                                   &olen, RSA_SIZE, mbedtls_ctr_drbg_random, &ctr_drbg));

    assert(0 == mbedtls_pk_encrypt(&client_ctx, buffer + DER_SIZE / 2, DER_SIZE / 2,
                                   cipher + RSA_SIZE, &olen, RSA_SIZE, mbedtls_ctr_drbg_random, &ctr_drbg));

    length = 2 * RSA_SIZE;
    assert(client_write(cipher, length));

    length = client_read(cipher, sizeof(cipher));
    assert(length == 3 * RSA_SIZE);

    assert(0 == mbedtls_pk_decrypt(&server_ctx, cipher, RSA_SIZE, buffer, &olen, RSA_SIZE,
                                   mbedtls_ctr_drbg_random, &ctr_drbg));

    length = olen;
    assert(0 == mbedtls_pk_decrypt(&server_ctx, cipher + RSA_SIZE, RSA_SIZE, buffer + length,
                                   &olen, RSA_SIZE, mbedtls_ctr_drbg_random, &ctr_drbg));

    length += olen;
    assert(0 == mbedtls_pk_decrypt(&server_ctx, cipher + 2 * RSA_SIZE, RSA_SIZE, buffer + length,
                                   &olen, RSA_SIZE, mbedtls_ctr_drbg_random, &ctr_drbg));

    length += olen;
    assert(length == (DER_SIZE + RSA_SIZE));

    mbedtls_pk_init(&client_ctx);
    assert(0 == mbedtls_pk_parse_public_key(&client_ctx, buffer, DER_SIZE));
    assert(MBEDTLS_PK_RSA == mbedtls_pk_get_type(&client_ctx));

    assert(0 == mbedtls_pk_verify(&client_ctx, MBEDTLS_MD_SHA256, secret_key, HASH_SIZE, buffer + DER_SIZE, RSA_SIZE));

    strcpy((char *)buffer, "OKAY");
    assert(0 == mbedtls_pk_encrypt(&client_ctx, buffer, strlen((const char *)buffer),
                                   cipher, &olen, RSA_SIZE, mbedtls_ctr_drbg_random, &ctr_drbg));

    assert(client_write(cipher, RSA_SIZE));
}

/* Exported user code --------------------------------------------------------*/

/**
 * @brief Writes the given data to the session using AES encryption.
 * 
 * @param res Pointer to the data to be written.
 * @param size Size of the data to be written.
 * @return True if the data was successfully written, false otherwise.
 */
static bool session_write(const uint8_t *res, size_t size)
{
    bool status = false;
    uint8_t response[AES_BLOCK_SIZE] = {0};
    uint8_t cipher[AES_BLOCK_SIZE + HASH_SIZE] = {0};

    memcpy(response, res, size);

    if (0 == mbedtls_aes_crypt_cbc(&aes_ctx, MBEDTLS_AES_ENCRYPT, sizeof(response), enc_iv, response, cipher))
    {
        status = client_write(cipher, AES_BLOCK_SIZE);
    }

    return status;
}

bool session_init(void)
{
    bool status = false;

    if (communication_init())
    {

        mbedtls_md_init(&hmac_ctx);

        if (0 == mbedtls_md_setup(&hmac_ctx, mbedtls_md_info_from_type(MBEDTLS_MD_SHA256), 1))
        {

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

bool session_establish(void)
{
    session_id = 0;
    bool status = false;
    size_t olen, length;
    uint8_t cipher[2 * RSA_SIZE]{0};

    if (0 == mbedtls_pk_decrypt(&server_ctx, buffer, RSA_SIZE, cipher, &olen, RSA_SIZE, mbedtls_ctr_drbg_random, &ctr_drbg))
    {
        length = olen;

        if (0 == mbedtls_pk_decrypt(&server_ctx, buffer + RSA_SIZE, RSA_SIZE, cipher + length, &olen, RSA_SIZE, mbedtls_ctr_drbg_random, &ctr_drbg))
        {
            length += olen;

            if (length == RSA_SIZE)
            {
                if (0 == mbedtls_pk_verify(&client_ctx, MBEDTLS_MD_SHA256, secret_key, HASH_SIZE, cipher, RSA_SIZE))
                {
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

                    if (0 == mbedtls_aes_setkey_enc(&aes_ctx, aes_key, sizeof(aes_key) * CHAR_BIT))
                    {
                        memcpy(buffer, &session_id, sizeof(session_id));
                        length = sizeof(session_id);

                        memcpy(buffer + length, enc_iv, sizeof(enc_iv));
                        length += sizeof(enc_iv);

                        memcpy(buffer + length, aes_key, sizeof(aes_key));
                        length += sizeof(aes_key);

                        status = true;
                    }
                    else
                    {
                        session_id = 0;
                    }
                }
            }
        }
    }

    if (!status)
    {
        memset(buffer, 0, sizeof(buffer));
        length = sizeof(session_id) + sizeof(enc_iv) + sizeof(aes_key);
    }

    if (0 == mbedtls_pk_encrypt(&client_ctx, buffer, length, cipher, &olen, RSA_SIZE, mbedtls_ctr_drbg_random, &ctr_drbg))
    {
        if (!client_write(cipher, RSA_SIZE))
        {
            status = false;
        }
    }

    if (status)
    {
        accessed = millis();
    }
    else
    {
        session_id = 0;
    }

    return status;
}

void session_close(void)
{
    session_id = 0;
}

request_t session_request(void)
{
    uint8_t response = STATUS_OKAY;
    request_t request = SESSION_ERROR;

    size_t length = client_read(buffer, sizeof(buffer));

    if (length == DER_SIZE)
    {
        exchange_public_keys();

        length = client_read(buffer, sizeof(buffer));
    }

    if (length == 2 * RSA_SIZE)
    {
        request = SESSION_ESTABLISH;
    }
    else if (length == AES_BLOCK_SIZE)
    {
        if (session_id != 0)
        {
            uint32_t now = millis();

            if (now - accessed <= KEEP_ALIVE)
            {
                accessed = now;

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
                            case SESSION_GET_TEMP:
                            case SESSION_TOGGLE_LED:
                                request = (request_t)temp[0];
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

    if (request == SESSION_ERROR)
    {
        assert(session_write(&response, sizeof(response)));
    }

    return request;
}

bool session_response(bool success, const uint8_t *res, size_t rlen)
{
    size_t len = 1;
    uint8_t response[AES_BLOCK_SIZE] = {0};

    response[0] = success ? STATUS_OKAY : STATUS_ERROR;

    if ((res != nullptr) && (rlen > 0))
    {
        memcpy(response + len, res, rlen);
        len += rlen;
    }

    return session_write(response, len);
}
