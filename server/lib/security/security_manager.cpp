#include "security_manager.h"

const char *SECRET_KEY = "Fj2-;wu3Ur=ARl2!Tqi6IuKM3nG]8z1+";

// Function to convert binary data to a hexadecimal string
String bytesToHexString(const unsigned char *data, size_t length)
{
    String result;
    Serial.println("ESP32 RESULT:" + result + "\n");
    for (size_t i = 0; i < length; ++i)
    {
        char hex[3];
        sprintf(hex, "%02x", data[i]);
        result += hex;
    }
    Serial.println("ESP32 RESULT HEX:" + result + "\n");
    return result;
}

// Function to compute HMAC-SHA256
String computeHMAC_SHA256(const char *message)
{
    mbedtls_md_context_t ctx;
    mbedtls_md_type_t md_type = MBEDTLS_MD_SHA256;
    unsigned char hmacResult[32]; // SHA256 outputs 32 bytes

    mbedtls_md_init(&ctx);
    mbedtls_md_setup(&ctx, mbedtls_md_info_from_type(md_type), 1);
    mbedtls_md_hmac_starts(&ctx, (const unsigned char *)SECRET_KEY, strlen(SECRET_KEY));
    mbedtls_md_hmac_update(&ctx, (const unsigned char *)message, strlen(message));
    mbedtls_md_hmac_finish(&ctx, hmacResult);
    mbedtls_md_free(&ctx);

    String result = bytesToHexString(hmacResult, 32);

    // Debug print
    Serial.print("ESP32 HMAC RESULT: " + hmacResult[32]) + "\n";
    Serial.print("ESP32 HMAC RESULT (HEX): " + result + "\n");
    Serial.print("Computed HMAC (hex):  \n");
    Serial.println(result) + "\n";

    return result;
}

// Function to verify HMAC-SHA256
bool verifyHMAC(const char *message, const char *receivedHmac)
{
    String ourHmac = computeHMAC_SHA256(message);

    // Debug prints
    Serial.print("Our computed HMAC: " + ourHmac + "\n");
    Serial.print("Received HMAC: ") + "\n";
    Serial.println(receivedHmac);

    return ourHmac.equals(receivedHmac);
}

void generate_random_bytes(unsigned char *buf, size_t len)
{
    for (size_t i = 0; i < len; ++i)
    {
        buf[i] = random(0, 256);
    }
}

void add_pkcs7_padding(unsigned char *input, size_t &input_len, size_t block_size)
{
    size_t padding_len = block_size - (input_len % block_size);
    for (size_t i = 0; i < padding_len; i++)
    {
        input[input_len + i] = (unsigned char)padding_len;
    }
    input_len += padding_len;
}

void encrypt_aes256(const unsigned char *input, size_t input_len,
                    unsigned char *output, size_t &output_len,
                    const unsigned char *key)
{
    mbedtls_aes_context aes;
    unsigned char iv[16];

    // Generate a random IV
    generate_random_bytes(iv, 16); // Assuming generate_random_bytes correctly populates iv with random values
    Serial.print("ESP32 IV (hex): ") + "\n";

    mbedtls_aes_init(&aes);
    Serial.print("ESP32 AES (hex): ") + "\n";
    mbedtls_aes_setkey_enc(&aes, key, 256 * 8); // Key length in bits
    Serial.print("ESP32 AES (hex): ") + "\n";

    // Apply PKCS7 padding
    unsigned char padded_input[input_len + 16]; // Max padding is 16 bytes
    memcpy(padded_input, input, input_len);
    add_pkcs7_padding(padded_input, input_len, 16);

    mbedtls_aes_crypt_cbc(&aes, MBEDTLS_AES_ENCRYPT, input_len, iv, padded_input, output);
    output_len = input_len;

    output_len = input_len; // This needs to be adjusted if padding is applied
    mbedtls_aes_free(&aes);

    // Debug prints
    Serial.print("ESP32 IV (hex): ") + "\n";
    Serial.println(bytesToHexString(iv, 16)) + "\n";
    Serial.print("ESP32 AES KEY (hex): ") + "\n";
    Serial.println(bytesToHexString(key, 32)) + "\n";

    // Include IV with the output
    memmove(output + 16, output, output_len); // Move the ciphertext to make space for the IV
    memcpy(output, iv, 16);                   // Copy the IV to the beginning of the output
    output_len += 16;                         // Adjust the output length to include the IV
}

void remove_pkcs7_padding(unsigned char *output, size_t &output_len)
{
    size_t padding_len = output[output_len - 1];
    if (padding_len > 0 && padding_len <= 16)
    { // Block size is 16 bytes
        output_len -= padding_len;
    }
}

void decrypt_aes256(const unsigned char *input, size_t input_len, unsigned char *output, size_t &output_len, const unsigned char *key)
{
    if (input_len < 16)
    {
        Serial.println("Invalid input length for decryption");
        return; // Invalid input length
    }

    mbedtls_aes_context aes;
    unsigned char iv[16];
    memcpy(iv, input, 16); // Assuming the first 16 bytes are the IV
    mbedtls_aes_init(&aes);
    mbedtls_aes_setkey_dec(&aes, key, 256 * 8);                                               // Key length in bits
    mbedtls_aes_crypt_cbc(&aes, MBEDTLS_AES_DECRYPT, input_len - 16, iv, input + 16, output); // Adjust input pointer and length

    output_len = input_len - 16;

    // Remove PKCS7 padding
    remove_pkcs7_padding(output, output_len);

    mbedtls_aes_free(&aes);

    // Debug prints
    Serial.print("ESP32 IV (hex):  \n");
    Serial.println(bytesToHexString(iv, 16)) + "\n";
    Serial.print("ESP32 AES KEY (hex):  \n");
    Serial.print("ESP32 Decrypted Data (hex):  \n");
    Serial.println(bytesToHexString(output, output_len));
}
