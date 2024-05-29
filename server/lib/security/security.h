#ifndef SECURITY_H
#define SECURITY_H

#include <mbedtls/md.h>
#include <mbedtls/pk.h>
#include <mbedtls/rsa.h>
#include <mbedtls/aes.h>
#include <mbedtls/entropy.h>
#include <mbedtls/ctr_drbg.h>

constexpr int AES_SIZE{32};       /**< AES_SIZE*/
constexpr int DER_SIZE{294};      /**< DER_SIZE*/
constexpr int RSA_SIZE{256};      /**< RSA_SIZE*/
constexpr int HASH_SIZE{32};      /**< HASH_SIZE*/
constexpr int EXPONENT{65537};    /**< EXPONENT*/
constexpr int AES_BLOCK_SIZE{16}; /**< AES_BLOCK_SIZE*/

static mbedtls_aes_context aesEncryptionContext;  /**< aesEncryptionContext*/
static mbedtls_md_context_t hmacContext;          /**< hmacContext*/
static mbedtls_pk_context clientPublicKeyContext; /**< clientPublicKeyContext*/
static mbedtls_pk_context server_ctx;             /**< server_ctx*/
static mbedtls_entropy_context entropy;           /**< entropy*/
static mbedtls_ctr_drbg_context ctr_drbg;

static uint64_t sessionId{0};                                     /**< sessionId*/
static uint8_t aes_key[AES_SIZE]{0};                              /**< aes_key*/
static uint8_t encryptionInitializationVector[AES_BLOCK_SIZE]{0}; /**< encryptionInitializationVector*/
static uint8_t decryptionInitializationVector[AES_BLOCK_SIZE]{0}; /**< decryptionInitializationVector*/

/**
 * @brief  The HMAC hash
 *
 */
static const uint8_t hmac_hash[HASH_SIZE] = {
    0x29, 0x49, 0xde, 0xc2, 0x3e, 0x1e, 0x34, 0xb5, 0x2d, 0x22, 0xb5,
    0xba, 0x4c, 0x34, 0x23, 0x3a, 0x9d, 0x3f, 0xe2, 0x97, 0x14, 0xbe,
    0x24, 0x62, 0x81, 0x0c, 0x86, 0xb1, 0xf6, 0x92, 0x54, 0xd6};

/**
 * @brief  This function initializes the security context. And seeds the RNG for Key generation.
 *
 */
void init_security();

/**
 * @brief  This function generates a RSA key pair and assings it to the clientPublicKeyContext and server_ctx.
 *
 */
void generate_rsa_key_pair();

/**
 * @brief  This function loads the client public key.
 *
 * @param key The key to load
 * @param key_len The length of the key
 */
void load_client_public_key(const uint8_t *key, size_t key_len);

/**
 * @brief  This function loads the server public key.
 *
 * @param key The key to load
 * @param key_len The length of the key
 */
void load_server_public_key(const uint8_t *key, size_t key_len);

/**
 * @brief  This function encrypts the input data using AES in CTR mode.
 *
 * @param input The input data
 * @param output The output data
 * @param length The length of the input data
 */
void aes_encrypt(const uint8_t *input, uint8_t *output, size_t length);

/**
 * @brief  This function decrypts the input data using AES in CTR mode.
 *
 * @param input The input data
 * @param output The output data
 * @param length The length of the input data
 */
void aes_decrypt(const uint8_t *input, uint8_t *output, size_t length);

/**
 * @brief  This function calculates the HMAC of the input data for a given input.
 *
 * @param input The input data
 * @param length The length of the input data
 * @param output The output data
 */
void calculate_hmac(const uint8_t *input, size_t length, uint8_t *output);

/**
 * @brief  This function reads the data from the client.
 *
 * @param buffer The buffer to read the data into
 * @param blen The length of the buffer
 * @return size_t The size of the data read
 */
void set_aes_key(const uint8_t *key);

/**
 * @brief  This function sets the encryption IV.
 *
 * @param iv The IV to set
 */
void set_encryption_iv(const uint8_t *iv);

/**
 * @brief  This function sets the decryption IV.
 *
 * @param iv The IV to set
 */
void set_decryption_iv(const uint8_t *iv);

#endif // SECURITY_H
