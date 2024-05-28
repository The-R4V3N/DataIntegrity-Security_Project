# Author: Oliver Joisten
# Description: This file contains the security module which is used in the client.py file to encrypt and decrypt messages.

from mbedtls import pk, hmac, hashlib, cipher

RSA_SIZE = 256
EXPONENT = 65537
SECRET_KEY = b"Fj2-;wu3Ur=ARl2!Tqi6IuKM3nG]8z1+"

# HMAC setup
HMAC_KEY = hashlib.sha256()
HMAC_KEY.update(SECRET_KEY)
HMAC_KEY = HMAC_KEY.digest()
hmac_hash = hmac.new(HMAC_KEY, digestmod="SHA256")

# RSA setup
rsa = pk.RSA()
rsa.generate(RSA_SIZE * 8, EXPONENT)


def initialize_rsa():
    """Initializes the RSA object.

    Returns:
        pk.RSA: The RSA object.
    """
    rsa.generate(RSA_SIZE * 8, EXPONENT)
    return rsa


def get_hmac_key():
    """Gets the HMAC key.

    Returns:
        bytes: The HMAC key.
    """
    return HMAC_KEY


def create_aes(buffer):
    """Creates an AES object for encryption/decryption.

    Args:
        buffer (bytes): The buffer containing key and IV.

    Returns:
        cipher.AES: The AES object.
    """
    return cipher.AES.new(buffer[24:56], cipher.MODE_CBC, buffer[8:24])


def export_public_key(rsa):
    """Exports the RSA public key.

    Args:
        rsa (pk.RSA): The RSA object.

    Returns:
        bytes: The exported public key.
    """
    return rsa.export_public_key()


def decrypt_rsa(rsa, buffer):
    """Decrypts data using RSA.

    Args:
        rsa (pk.RSA): The RSA object.
        buffer (bytes): The encrypted data buffer.

    Returns:
        bytes: The decrypted data.
    """
    return rsa.decrypt(buffer)


def sign_rsa(rsa, HMAC_KEY):
    """Signs data using RSA.

    Args:
        rsa (pk.RSA): The RSA object.
        HMAC_KEY (bytes): The HMAC key to sign.

    Returns:
        bytes: The signed data.
    """
    return rsa.sign(HMAC_KEY, "SHA256")


def encrypt_rsa(rsa, buffer):
    """Encrypts data using RSA.

    Args:
        rsa (pk.RSA): The RSA object.
        buffer (bytes): The data to encrypt.

    Returns:
        bytes: The encrypted data.
    """
    return rsa.encrypt(buffer)


def send_data(self, buf: bytes) -> bool:
    """Sends data to the server.

    Args:
        buf (bytes): The data to send.

    Returns:
        bool: True if the data was sent successfully, False otherwise.
    """
    if self.ser and self.ser.is_open:
        hmac_hash.update(buf)
        buf += hmac_hash.digest()
        print(f"Sending data (length {len(buf)}): {buf.hex()}")
        written = self.ser.write(buf)
        if len(buf) != written:
            print(
                f"Connection Error: Only {written} bytes written out of {len(buf)}")
            self.close_connection()
            return False
        self.ser.flush()
        print("Data flushed.")
        return True
    else:
        if self.ser is None:
            raise ConnectionError("Serial port is not open")
        return False


def receive_data(self, size: int) -> bytes:
    """Receives data from the server.

    Args:
        size (int): The size of data to receive.

    Returns:
        bytes: The received data.
    """
    if self.ser and self.ser.is_open:
        buffer = self.ser.read(size + hmac_hash.digest_size)
        print(f"Received raw data (length {len(buffer)}): {buffer.hex()}")
        hmac_hash.update(buffer[0:size])
        buff = buffer[size:size + hmac_hash.digest_size]
        dig = hmac_hash.digest()
        print("b", buff.hex())
        print("d", dig.hex())
        if buff != dig:
            print("Hash Error")
            self.close_connection()
            return bytes()
        return buffer[0:size]
    else:
        if self.ser is None:
            raise ConnectionError("Serial port is not open")
        return bytes()
