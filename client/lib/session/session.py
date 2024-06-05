"""
    * @File: session.py
    * @Autor: Oliver Joisten (contact@oliver-joisten.se)
    * @Desccription: This file contains the session class which is used to establish a connection with the server and send requests to it.
    * @Version: 1.0
    * @Created: 2021-06-15
"""

from mbedtls import pk, hmac, hashlib, cipher
from client.lib.communication.communication import Communication

response_codes = {
    "00": "STATUS_OKAY",
    "01": "STATUS_ERROR",
    "02": "STATUS_EXPIRED",
    "03": "STATUS_HASH_ERROR",
    "04": "STATUS_BAD_REQUEST",
    "05": "STATUS_INVALID_SESSION",
}

class Session:
    RESPONSE = "SESSION_OKAY"
    RSA_SIZE = 256
    EXPONENT = 65537
    SECRET_KEY = b"Fj2-;wu3Ur=ARl2!Tqi6IuKM3nG]8z1+"
    CONNECTED = None

    def __init__(self, port):
        self.initialize = False
        self.PORT = port
        self.SESSION_ID = None
        self.HMAC_KEY = hashlib.sha256()
        self.HMAC_KEY.update(self.SECRET_KEY)
        self.HMAC_KEY = self.HMAC_KEY.digest()
        self.hmac_hash = hmac.new(self.HMAC_KEY, digestmod="SHA256")
        self.ser = Communication(port)
        self.client_public_rsa = pk.RSA()
        self.client_public_rsa.generate(
            self.RSA_SIZE * 8, self.EXPONENT)
        self.server_public_rsa = None
        self.aes_key = None
        Session.CONNECTED = port
        self.status = None

    def session_connected(self):
        Session.CONNECTED
        if Session.CONNECTED != None:
            Session.CONNECTED = False
        else:
            Session.CONNECTED = True
        return Session.CONNECTED

    def get_temperature(self):
        received = self.requests(int(0x03)).decode("utf-8")
        return received

    def toggle_led(self):
            received = self.requests(int(0x02)).decode("utf-8")
            if received == 0:
                return received

    def client_send(self, buffer: bytes):
        self.hmac_hash.update(buffer)
        buffer += self.hmac_hash.digest()
        sent_length = self.ser.communication_send(buffer)
        if len(buffer) != sent_length:
            self.ser.close_connection()

    def client_read(self, size: int) -> bytes:
        buffer = self.ser.communication_read(
            size + self.hmac_hash.digest_size)
        self.hmac_hash.update(buffer[0: size])
        buf = buffer[size: size + self.hmac_hash.digest_size]
        temp = self.hmac_hash.digest()

        if temp != buf:
            self.ser.close_connection()
        return buffer[0: size]
    
    def close_session(self):
        # self.ser.close_connection()
        pass

    def key_exchange(self) -> bool:

        try:
            status = False

            # Send client's public key to server
            self.client_send(
                self.client_public_rsa.export_public_key())

            # Receive server's public key
            buffer = self.client_read(2 * self.RSA_SIZE)
            status = True

            # Decrypt the server's public key
            SERVER_PUBLIC_KEY = self.client_public_rsa.decrypt(
                buffer[0 : self.RSA_SIZE])
            SERVER_PUBLIC_KEY += self.client_public_rsa.decrypt(
                buffer[self.RSA_SIZE: 2 * self.RSA_SIZE])
            self.server_public_rsa = pk.RSA().from_DER(SERVER_PUBLIC_KEY)
            status = True

            # Delete the client's public key and generate a new one
            del self.client_public_rsa
            self.client_public_rsa = pk.RSA()
            self.client_public_rsa.generate(self.RSA_SIZE * 8, self.EXPONENT)
            status = True

            # Export the client's public key and sign it with the secret key and send it to the server
            buffer = self.client_public_rsa.export_public_key(
            ) + self.client_public_rsa.sign(self.SECRET_KEY, "SHA256")
            buffer = self.server_public_rsa.encrypt(buffer[0:184]) + self.server_public_rsa.encrypt(
                buffer[184:368]) + self.server_public_rsa.encrypt(buffer[368:550])
            self.client_send(buffer)
            status = True

            # Receive the server's response
            buffer = self.client_read(self.RSA_SIZE)

            # Decrypt the server's response if okay continue with authentication else throw an exception and close the connection
            if self.client_public_rsa.decrypt(buffer) == self.RESPONSE:
                self.authenticate()

            status = True

            return status

        except Exception as e:
            print(e)
            self.close_session()

    def authenticate(self) -> bool:
        try:
            connected = False

            buffer = self.client_public_rsa.sign(self.SECRET_KEY, "SHA256")
            buffer = self.server_public_rsa.encrypt(
                buffer[0: self.RSA_SIZE // 2]) + self.server_public_rsa.encrypt(buffer[self.RSA_SIZE // 2: self.RSA_SIZE])
            self.client_send(buffer)

            buffer = self.client_read(self.RSA_SIZE)
            buffer = self.client_public_rsa.decrypt(buffer)
            self.SESSION_ID = buffer[0:8]

            self.aes_key = cipher.AES.new(
                buffer[24: 56], cipher.MODE_CBC, buffer[8: 24])
            connected = True

            return connected

        except Exception as e:
            print(e)
            self.close_session()

    def requests(self, invalue) -> str:
        request = bytes([invalue])
        buffer = request + self.SESSION_ID
        padding_length = cipher.AES.block_size - \
            (len(buffer) % cipher.AES.block_size)
        buffer = self.aes_key.encrypt(
            buffer + bytes([len(buffer)] * padding_length))

        self.client_send(buffer)


        buffer = self.client_read(cipher.AES.block_size)
        buffer = self.aes_key.decrypt(buffer)

        if buffer[0] == 0x00:

            result = str(buffer[1:6], "utf-8").replace('\x00', '').strip()

            led_states = ["ON", "OFF"]

            if result in led_states:
                return "Led =>: " + result

            try:
                temp = float(result)
                return "Temperature =>: " + str(temp) + " C"
            except ValueError:
                return "Unexpected result =>: " + result


        else:
            error_code = str(buffer[:1].hex())
            if error_code in response_codes:
                return f"Error code =>: {response_codes[error_code]}"
            else:
                return f"Error code =>: Unknown error ({error_code})"
