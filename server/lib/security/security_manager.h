#ifndef SECURITYMANAGER_H
#define SECURITYMANAGER_H

#include <Arduino.h>
#include "mbedtls/md.h"
#include "mbedtls/aes.h"
#include <string.h>

extern const char *SECRET_KEY;

String bytesToHexString(const unsigned char *data, size_t length);

String computeHMAC_SHA256(const char *message);

bool verifyHMAC(const char *message, const char *receivedHmac);

void generate_random_bytes(unsigned char *buf, size_t len);

void add_pkcs7_padding(unsigned char *input, size_t &input_len, size_t block_size);

void encrypt_aes256(const unsigned char *input, size_t input_len,
                    unsigned char *output, size_t &output_len,
                    const unsigned char *key);

void remove_pkcs7_padding(unsigned char *output, size_t &output_len);

void decrypt_aes256(const unsigned char *input, size_t input_len,
                    unsigned char *output, size_t &output_len,
                    const unsigned char *key);

#endif // SECURITYMANAGER_H
