# Autor: Oliver Joisten
# Desccription: This file contains the session class which is used to establish a connection with the server and send requests to it.

import serial.tools.list_ports
from client.lib.communication import communication
from mbedtls import pk, hmac, hashlib, cipher

BAUDRATE = 115200

RSA_SIZE = 256
EXPONENT = 65537
SECRET_KEY = b"Fj2-;wu3Ur=ARl2!Tqi6IuKM3nG]8z1+"

HMAC_KEY = hashlib.sha256()
HMAC_KEY.update(SECRET_KEY)
HMAC_KEY = HMAC_KEY.digest()
hmac_hash = hmac.new(HMAC_KEY, digestmod="SHA256")


class SessionManager:
    def __init__(self):
        self.serial_comm = None
        self.session_active = False
        self.communication = communication
        self.session_id = None
        self.aes = None

    def get_serial_ports(self):
        """Gets the available serial ports.

        Returns:
            list: A list of available serial ports.
        """
        ports = serial.tools.list_ports.comports()
        return [port.device for port in ports]

    def is_connected(self):
        """Checks if the serial communication is connected.

        Returns:
            bool: True if connected, False otherwise.
        """
        return self.serial_comm and self.serial_comm.is_open

    def close_session(self):
        """Closes the session."""
        if self.session_active:
            self.communication_manager.write_data(b"close")
            self.communication_manager.close_serial_port()
            self.session_active = False
            print("Session closed")

    def send_command(self, command: int):
        """Sends a command to the server.

        Args:
            command (int): The command to send.
        """
        try:
            request = bytes([command])
            buffer = request + self.session_id
            plen = cipher.AES.block_size - \
                (len(buffer) % cipher.AES.block_size)
            buffer = self.aes.encrypt(buffer + bytes([len(buffer)] * plen))
            self.send_data(buffer)

            buffer = self.receive_data(cipher.AES.block_size)
            buffer = self.aes.decrypt(buffer)
            if buffer[0] == 0x10:
                self.log("Response: " +
                         buffer[1:6].decode("UTF-8", errors="replace"))
            else:
                self.log("Command not found!")
        except Exception as e:
            self.log(f"Error: {e}")

    def establish_session(self, port):
        """Handles the establishment of a secure session."""
        global rsa, hmac_hash

        global rsa, hmac_hash
        try:
            self.serial_comm = self.communication_manager.open_serial_port(
                port, BAUDRATE)
            self.session_active = True

            rsa = pk.RSA()
            rsa.generate(RSA_SIZE * 8, EXPONENT)
            self.communication_manager.write_data(rsa.export_public_key())

            buffer = self.communication_manager.read_data(2 * RSA_SIZE)
            public_key_server = rsa.decrypt(buffer[0:RSA_SIZE])
            public_key_server += rsa.decrypt(buffer[RSA_SIZE:2 * RSA_SIZE])
            self.server_rsa = pk.RSA().from_DER(public_key_server)

            rsa = pk.RSA()
            rsa.generate(RSA_SIZE * 8, EXPONENT)
            buffer = rsa.export_public_key() + rsa.sign(HMAC_KEY, "SHA256")
            buffer = self.server_rsa.encrypt(buffer[0:184]) + self.server_rsa.encrypt(
                buffer[184:368]) + self.server_rsa.encrypt(buffer[368:550])
            self.communication_manager.write_data(buffer)

            buffer = self.communication_manager.read_data(RSA_SIZE)
            if b"OKAY" == rsa.decrypt(buffer):
                buffer = rsa.sign(HMAC_KEY, "SHA256")
                buffer = self.server_rsa.encrypt(
                    buffer[0:RSA_SIZE // 2]) + self.server_rsa.encrypt(buffer[RSA_SIZE // 2:RSA_SIZE])
                self.communication_manager.write_data(buffer)

                buffer = self.communication_manager.read_data(RSA_SIZE)
                buffer = rsa.decrypt(buffer)
                self.session_id = buffer[0:8]

                self.aes = cipher.AES.new(
                    buffer[24:56], cipher.MODE_CBC, buffer[8:24])
                print("Session Established and Keys Successfully Exchanged!")

        except serial.SerialException:
            print(f"Failed to establish session with {port}!")

    def _send_command(self, command: int):
        """Handles sending a command to the server."""
        try:
            request = bytes([command])
            buffer = request + self.session_id
            plen = cipher.AES.block_size - \
                (len(buffer) % cipher.AES.block_size)
            buffer = self.aes.encrypt(buffer + bytes([len(buffer)] * plen))
            self.communication_manager.write_data(buffer)

            buffer = self.communication_manager.read_data(
                cipher.AES.block_size)
            buffer = self.aes.decrypt(buffer)
            if buffer[0] == 0x10:
                print("Response: " +
                      buffer[1:6].decode("UTF-8", errors="replace"))
            else:
                print("Command not found!")
        except Exception as e:
            print(f"Error: {e}")
