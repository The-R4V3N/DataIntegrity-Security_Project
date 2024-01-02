# Autor: Oliver Joisten
# Desccription: This file contains the security module which is used in the client.py file to encrypt and decrypt messages.

from mbedtls import hmac, cipher
import os

hmac_key = "Fj2-;wu3Ur=ARl2!Tqi6IuKM3nG]8z1+"

# Generating a 256-bit AES key
aes_key = os.urandom(32)
print("AES KEY:", aes_key)
print("\n")


def generate_hmac(message, hmac_key):
    hmac_key_bytes = hmac_key.encode()
    h = hmac.new(hmac_key_bytes, digestmod='sha256')
    h.update(message.encode())
    print("HMAC:", h.hexdigest())
    print("\n")
    return h.hexdigest()


def verify_hmac(message, received_hmac, hmac_key):
    our_hmac = generate_hmac(message, hmac_key)
    print(our_hmac + "\n" + received_hmac + "\n")

    return our_hmac == received_hmac


def pad(data, block_size):
    padding_length = block_size - len(data) % block_size
    padding = chr(padding_length) * padding_length
    return data + padding.encode()


def unpad(data):
    padding_length = data[-1]
    if padding_length > 16:
        raise ValueError("Invalid padding length")
    return data[:-padding_length]


def encrypt_message_aes256(message, aes_key):
    print("\n Encryption process started")
    print("Type of aes_key:", type(aes_key))
    print("\n")

    # Check if the key and message are bytes
    if not isinstance(aes_key, bytes):
        raise TypeError("Key must be a bytes-like object")
    if not isinstance(message, bytes):
        raise TypeError("Message must be a bytes-like object for encryption")

    # Generate a random IV
    iv = os.urandom(16)

    # Create a new AES cipher in CBC mode with PKCS7 padding
    cipher_aes = cipher.AES.new(aes_key, cipher.MODE_CBC, iv)
    print("IV:", iv)
    print("\n")

    # Encrypt the padded message
    encrypted_message = cipher_aes.encrypt(pad(message, cipher_aes.block_size))
    print("ENCRYPTED MESSAGE:", encrypted_message)
    print("\n")

    # Convert the encrypted message to hexadecimal
    encrypted_message_hex = encrypted_message.hex()
    print("HEX ENCRYPTED MESSAGE:", encrypted_message_hex)
    print("\n")

    # Convert the IV to hexadecimal
    iv_hex = iv.hex()
    print("HEX ENCRYPTED IV: ", iv_hex)
    print("\n")

    # Prepend IV to the encrypted message
    final_message_hex = iv_hex + encrypted_message_hex
    print("FINAL MESSAGE:", final_message_hex)
    print("\n")
    print("Encryption process finished")
    return final_message_hex


def decrypt_message_aes256(encrypted_message_hex, aes_key):
    print("Decryption process started")
    if not isinstance(aes_key, bytes):
        raise TypeError("Key must be a bytes-like object")
    if not isinstance(encrypted_message_hex, str):
        raise TypeError("Encrypted message must be a hexadecimal string")

    # Correctly extract the IV and the encrypted message
    iv_hex = encrypted_message_hex[:32]
    encrypted_message_hex = encrypted_message_hex[32:]

    print("Extracted IV:", iv_hex)
    print("\n")

    # Convert hex to bytes
    iv = bytes.fromhex(iv_hex)
    encrypted_message = bytes.fromhex(encrypted_message_hex)

    print("Converted IV:", iv)
    print("\n")

    # Decrypt the message
    cipher_aes = cipher.AES.new(aes_key, cipher.MODE_CBC, iv)
    decrypted_padded_message = cipher_aes.decrypt(encrypted_message)

    # Unpad the message
    decrypted_message = unpad(decrypted_padded_message)
    print("DECRYPTED MESSAGE:", decrypted_message)
    print("\n")
    print("Decryption process finished")
    print("\n")
    return decrypted_message
