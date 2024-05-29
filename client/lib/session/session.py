# Autor: Oliver Joisten
# Desccription: This file contains the session class which is used to establish a connection with the server and send requests to it.

from client.lib.security.security import pk, hmac_hash, RSA_SIZE, EXPONENT
from client.lib.communication.communication import SerialCommunication


class Session:
    def __init__(self):
        """Initializes the session object."""
        self.connect_state = False
        self.comm = SerialCommunication(self)

    def session(self):
        """Checks the connection state with the server.

        Returns:
            bool: The current connection state.
        """
        return self.connect_state

    def toggle_led(self):
        """Sends a request to toggle the LED on the server.

        Returns:
            bool: True if the request was sent successfully, False otherwise.
        """
        if self.connect_state:
            return self.send_request(b'0x49')
        return False

    def get_temp(self):
        """Sends a request to get the temperature from the server.

        Returns:
            bool: True if the request was sent successfully, False otherwise.
        """
        if self.connect_state:
            return self.send_request(b'0x54')
        return False

    def close_session(self):
        """Closes the session with the server."""
        if self.connect_state:
            self.send_request(b'0x10')
            self.comm.close()
            self.connect_state = False

    def send_request(self, data: bytes) -> bool:
        """Sends a request to the server.

        Args:
            data (bytes): The data to send.

        Returns:
            bool: True if the request was sent successfully, False otherwise.
        """
        hmac_hash.update(data)
        data += hmac_hash.digest()
        written = self.comm.write(data)
        if len(data) != written:
            print(
                f"Connection Error: Only {written} bytes written out of {len(data)}")
            self.close_session()
            return False
        return True

    def receive_data(self, size: int) -> bytes:
        """Receives data from the server.

        Args:
            size (int): The size of data to receive.

        Returns:
            bytes: The received data. Returns an empty byte string if there is a hash error.
        """
        buffer = self.comm.read(size + hmac_hash.digest_size)
        hmac_hash.update(buffer[0:size])
        buff = buffer[size:size + hmac_hash.digest_size]
        dig = hmac_hash.digest()
        if buff != dig:
            print("Hash Error")
            self.close_session()
            return b''
        return buffer[0:size]


# Initialize session
session = Session()

# Generate RSA key pair for client
client_rsa = pk.RSA()
client_rsa.generate(RSA_SIZE * EXPONENT)

# Send the public key to the server
session.send_request(client_rsa.export_public_key())  # len249

# Receive the server's public key
buffer = session.receive_data(2 * RSA_SIZE)  # STOP

if buffer:
    SERVER_PUBLIC_KEY = client_rsa.decrypt(buffer[0:RSA_SIZE])
    SERVER_PUBLIC_KEY += client_rsa.decrypt(buffer[RSA_SIZE:2 * RSA_SIZE])
    server_rsa = pk.RSA().from_DER(SERVER_PUBLIC_KEY)

# Clean up client RSA key
del client_rsa
