/**
 * @file security.cpp
 * @author Oliver Joisten (contact@oliver-joisten.se)
 * @brief
 * @version 0.1
 * @date 2024-05-29
 *
 * @copyright Copyright (c) 2024
 *
 */

#include "security.h"
#include <Arduino.h>

void init_security()
{
    mbedtls_entropy_init(&entropy);                                                                              /**< Initialize the entropy */
    mbedtls_ctr_drbg_init(&ctr_drbg);                                                                            /**< Initialize the DRBG */
    const char *pers = "aes_generate_key";                                                                       /**< The personalization string */
    mbedtls_ctr_drbg_seed(&ctr_drbg, mbedtls_entropy_func, &entropy, (const unsigned char *)pers, strlen(pers)); /**< Seed the DRBG */

    mbedtls_aes_init(&aesEncryptionContext);                                         /**< Initialize the AES context */
    mbedtls_md_init(&hmacContext);                                                   /**< Initialize the HMAC context */
    mbedtls_md_setup(&hmacContext, mbedtls_md_info_from_type(MBEDTLS_MD_SHA256), 1); /**< Setup the HMAC context */

    mbedtls_pk_init(&clientPublicKeyContext); /**< Initialize the client public Key */
    mbedtls_pk_init(&server_ctx);             /**< Initialize the server Key */
}

void generate_rsa_key_pair()
{
    int ret;
    mbedtls_pk_context key;
    mbedtls_pk_init(&key);

    if ((ret = mbedtls_pk_setup(&key, mbedtls_pk_info_from_type(MBEDTLS_PK_RSA))) != 0)
    {
        /* Do nothing */
    }

    mbedtls_rsa_context *rsa = mbedtls_pk_rsa(key);
    if ((ret = mbedtls_rsa_gen_key(rsa, mbedtls_ctr_drbg_random, &ctr_drbg, RSA_SIZE * 8, EXPONENT)) != 0)
    {
        /* Do nothing */
    }

    mbedtls_pk_free(&clientPublicKeyContext); /**< Free the old client public Key */
    mbedtls_pk_free(&server_ctx);             /**< Free the old server Key */

    clientPublicKeyContext = key; /**< Assign the new client public Key */
    server_ctx = key;             /**< Assign the new server Key */
}

void load_client_public_key(const uint8_t *key, size_t key_len)
{
    int ret = mbedtls_pk_parse_public_key(&clientPublicKeyContext, key, key_len);
    if (ret != 0)
    {
        /* Do nothing */
    }
}

void load_server_public_key(const uint8_t *key, size_t key_len)
{
    int ret = mbedtls_pk_parse_public_key(&server_ctx, key, key_len);
    if (ret != 0)
    {
        /* Do nothing */
    }
}

void aes_encrypt(const uint8_t *input, uint8_t *output, size_t length)
{
    size_t nc_off = 0;
    unsigned char stream_block[AES_BLOCK_SIZE];
    mbedtls_aes_crypt_ctr(&aesEncryptionContext, length, &nc_off, encryptionInitializationVector, stream_block, input, output);
}

void aes_decrypt(const uint8_t *input, uint8_t *output, size_t length)
{
    size_t nc_off = 0;
    unsigned char stream_block[AES_BLOCK_SIZE];
    mbedtls_aes_crypt_ctr(&aesEncryptionContext, length, &nc_off, decryptionInitializationVector, stream_block, input, output);
}

void calculate_hmac(const uint8_t *input, size_t length, uint8_t *output)
{
    mbedtls_md_hmac_starts(&hmacContext, hmac_hash, HASH_SIZE);
    mbedtls_md_hmac_update(&hmacContext, input, length);
    mbedtls_md_hmac_finish(&hmacContext, output);
}

void set_aes_key(const uint8_t *key)
{
    memcpy(aes_key, key, AES_SIZE);
    mbedtls_aes_setkey_enc(&aesEncryptionContext, aes_key, AES_SIZE * 8);
}

void set_encryption_iv(const uint8_t *iv)
{
    memcpy(encryptionInitializationVector, iv, AES_BLOCK_SIZE);
}

void set_decryption_iv(const uint8_t *iv)
{
    memcpy(decryptionInitializationVector, iv, AES_BLOCK_SIZE);
}
