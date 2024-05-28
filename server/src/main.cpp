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

static void exchange_public_keys(uint8_t *buffer)
{
    sessionId = 0;

    size_t olen, length;

    mbedtls_pk_init(&clientPublicKeyContext);

    uint8_t encryptedData[3 * RSA_SIZE + HASH_SIZE] = {0};

    if (0 != mbedtls_pk_parse_public_key(&clientPublicKeyContext, buffer, DER_SIZE))
    {
        ;
    }

    if (MBEDTLS_PK_RSA != mbedtls_pk_get_type(&clientPublicKeyContext))
    {
        ;
    }

    if (DER_SIZE != mbedtls_pk_write_pubkey_der(&server_ctx, buffer, DER_SIZE))
    {
        ;
    }

    if (0 != mbedtls_pk_encrypt(&clientPublicKeyContext, buffer, DER_SIZE / 2, encryptedData,
                                &olen, RSA_SIZE, mbedtls_ctr_drbg_random, &ctr_drbg))
    {
        ;
    }

    if (0 != mbedtls_pk_encrypt(&clientPublicKeyContext, buffer + DER_SIZE / 2, DER_SIZE / 2,
                                encryptedData + RSA_SIZE, &olen, RSA_SIZE, mbedtls_ctr_drbg_random, &ctr_drbg))
    {
        ;
    }

    length = 2 * RSA_SIZE;

    if (!client_write(encryptedData, length))
    {
        ;
    }

    length = client_read(encryptedData, sizeof(encryptedData));

    if (!length == 3 * RSA_SIZE)
    {
        ;
    }

    if (0 != mbedtls_pk_decrypt(&server_ctx, encryptedData, RSA_SIZE, buffer, &olen, RSA_SIZE,
                                mbedtls_ctr_drbg_random, &ctr_drbg))
    {
        ;
    }

    length = olen;

    if (0 != mbedtls_pk_decrypt(&server_ctx, encryptedData + RSA_SIZE, RSA_SIZE, buffer + length,
                                &olen, RSA_SIZE, mbedtls_ctr_drbg_random, &ctr_drbg))
    {
        ;
    }

    length += olen;

    if (0 != mbedtls_pk_decrypt(&server_ctx, encryptedData + 2 * RSA_SIZE, RSA_SIZE, buffer + length,
                                &olen, RSA_SIZE, mbedtls_ctr_drbg_random, &ctr_drbg))
    {
        ;
    }

    length += olen;

    if (length != (DER_SIZE + RSA_SIZE))
    {
        ;
    }

    mbedtls_pk_init(&clientPublicKeyContext);

    if (0 != mbedtls_pk_parse_public_key(&clientPublicKeyContext, buffer, DER_SIZE))
    {
        ;
    }

    if (MBEDTLS_PK_RSA != mbedtls_pk_get_type(&clientPublicKeyContext))
    {
        ;
    }

    if (0 != mbedtls_pk_verify(&clientPublicKeyContext, MBEDTLS_MD_SHA256, hmac_hash, HASH_SIZE, buffer + DER_SIZE, RSA_SIZE))
    {
        ;
    }

    strcpy((char *)buffer, "OKAY");

    if (0 != mbedtls_pk_encrypt(&clientPublicKeyContext, buffer, strlen((const char *)buffer),
                                encryptedData, &olen, RSA_SIZE, mbedtls_ctr_drbg_random, &ctr_drbg))
    {
        ;
    }

    length = RSA_SIZE;

    if (!client_write(encryptedData, length))
    {
        ;
    }
}

static void establish_session(uint8_t *buffer)
{
    sessionId = 0;

    size_t olen, length;

    uint8_t encryptedData[RSA_SIZE]{0};

    if (0 != mbedtls_pk_decrypt(&server_ctx, buffer, RSA_SIZE, encryptedData, &olen,
                                RSA_SIZE, mbedtls_ctr_drbg_random, &ctr_drbg))
    {
        ;
    }

    length = olen;

    if (0 != mbedtls_pk_decrypt(&server_ctx, buffer + RSA_SIZE, RSA_SIZE, encryptedData + length,
                                &olen, RSA_SIZE, mbedtls_ctr_drbg_random, &ctr_drbg))
    {
        ;
    }

    length += olen;

    if (length != RSA_SIZE)
    {
        ;
    }

    if (0 != mbedtls_pk_verify(&clientPublicKeyContext, MBEDTLS_MD_SHA256, hmac_hash, HASH_SIZE, encryptedData, RSA_SIZE))
    {
        ;
    }

    uint8_t *ptr{(uint8_t *)&sessionId};

    if (ptr != nullptr)
    {
        for (size_t i = 0; i < sizeof(sessionId); i++)
        {
            ptr[i] = random(1, 0x100);
        }

        for (size_t i = 0; i < sizeof(encryptionInitializationVector); i++)
        {
            encryptionInitializationVector[i] = random(0x100);
        }
    }
    else
    {
        ;
    }

    memcpy(decryptionInitializationVector, encryptionInitializationVector, sizeof(decryptionInitializationVector));

    for (size_t i = 0; i < sizeof(aes_key); i++)
    {
        aes_key[i] = random(0x100);
    }

    if (0 != mbedtls_aes_setkey_enc(&aesEncryptionContext, aes_key, sizeof(aes_key) * CHAR_BIT))
    {
        ;
    }

    memcpy(buffer, &sessionId, sizeof(sessionId));

    length = sizeof(sessionId);

    memcpy(buffer + length, encryptionInitializationVector, sizeof(encryptionInitializationVector));

    length += sizeof(encryptionInitializationVector);

    memcpy(buffer + length, aes_key, sizeof(aes_key));

    length += sizeof(aes_key);

    if (0 != mbedtls_pk_encrypt(&clientPublicKeyContext, buffer, length, encryptedData, &olen,
                                RSA_SIZE, mbedtls_ctr_drbg_random, &ctr_drbg))
    {
        ;
    }

    length = RSA_SIZE;

    if (client_write(encryptedData, length) == 0)
    {
        ;
    }
}

void setup()
{
    Serial.begin(115200);

    // HMAC-SHA256
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

void loop()
{
    uint8_t buffer[DER_SIZE + RSA_SIZE] = {0};
    size_t length = client_read(buffer, sizeof(buffer));

    if (length == DER_SIZE)
    {
        exchange_public_keys(buffer);
    }
    else if (length == 2 * RSA_SIZE)
    {
        establish_session(buffer);
    }
    else if (length == AES_BLOCK_SIZE)
    {
        if (sessionId != 0)
        {
            uint8_t encryptedData[AES_BLOCK_SIZE]{0};
            if (0 == mbedtls_aes_crypt_cbc(&aesEncryptionContext, MBEDTLS_AES_DECRYPT, length, decryptionInitializationVector, buffer, encryptedData))
            {
                if (encryptedData[AES_BLOCK_SIZE - 1] == 9)
                {
                    if (encryptedData[0] == 0x54) // Get temperature
                    {
                        if (0 == memcmp(&sessionId, &encryptedData[1], sizeof(sessionId)))
                        {
                            buffer[0] = 0x10; // OKAY
                            sprintf((char *)&buffer[1], "%2.2f", temperatureRead());
                        }
                    }
                    else if (encryptedData[0] == 0x02) // Toggel led
                    {
                        if (0 == memcmp(&sessionId, &encryptedData[1], sizeof(sessionId)))
                        {
                            buffer[0] = 0x10; // OKAY
                            sprintf((char *)&buffer[1], "%2.2f", float(encryptedData[1]));
                        }
                    }
                    length = AES_BLOCK_SIZE;

                    if (0 == mbedtls_aes_crypt_cbc(&aesEncryptionContext, MBEDTLS_AES_ENCRYPT, length, encryptionInitializationVector, buffer, encryptedData))
                    {
                        if (!client_write(encryptedData, length))
                        {
                            ;
                        }
                    }
                    else
                    {
                        ;
                    }
                }
            }
        }
    }
    else
    {
        ;
    }
}