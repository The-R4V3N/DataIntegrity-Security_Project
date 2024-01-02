# Autor: Oliver Joisten
# Desccription: This file contains the communictaion module which is used int the client.py file to send and receive messages from the server.

import serial
from client.lib.security.security import generate_hmac, verify_hmac, encrypt_message_aes256, decrypt_message_aes256, aes_key

import traceback


BAUD_RATE = 115200


class SerialCommunication:
    def __init__(self):
        self.serial_connection = None

    def establish_connection(self, port, BAUD_RATE=BAUD_RATE, timeout=1):
        try:
            self.serial_connection = serial.Serial(
                port, baudrate=BAUD_RATE, timeout=timeout)
            return True, f"Session Established on {port}"
        except serial.SerialException as e:
            return False, f"Failed to open port {port}: {e}"

    def is_connected(self):
        return self.serial_connection is not None and self.serial_connection.is_open

    def send_data(self, data, hmac_key, aes_key):
        if self.is_connected():  # Check if the serial connection is established
            try:
                # Generate HMAC for the data
                hmac = generate_hmac(data, hmac_key)

                # Combine data with HMAC
                full_message = f"{data},{hmac}"
                print(data, hmac)
                full_message_bytes = full_message.encode('utf-8')

                # Encrypt the message
                encrypted_message_hex = encrypt_message_aes256(
                    full_message_bytes, aes_key)
                print("ENCRYPTED MESSAGE BEFORE SEND: ", encrypted_message_hex)
                print("\n")

                # Send the encrypted message over the serial connection
                self.serial_connection.write(encrypted_message_hex.encode(
                    'utf-8'))  # Use serial_connection's write method
                print("Encrypted message sent ",
                      encrypted_message_hex.encode('utf-8'))
                print("\n")
                return True, "Data sent"
            except Exception as e:
                traceback.print_exc()
                return False, f"Failed to encrypt data: {e}"
        else:
            return False, "Serial connection not established"

    def serial_read(self, hmac_key):
        if self.serial_connection and self.serial_connection.in_waiting:
            encrypted_data = self.serial_connection.readline().decode('utf-8').strip()
            print("Encrypted Data Received:", encrypted_data)

            # Remove non-hexadecimal characters (temporary workaround)
            encrypted_data = ''.join(filter(str.isalnum, encrypted_data))

            # Decrypt the data
            try:
                decrypted_data = decrypt_message_aes256(
                    encrypted_data, hmac_key)
                message, received_hmac = decrypted_data.rsplit(',', 1)

                if verify_hmac(message, received_hmac, hmac_key):
                    return message
                else:
                    return "HMAC verification failed"
            except Exception as e:
                return f"Decryption error: {e}"
                print("Serial Read finished")
        return None

    def close_connection(self):
        if self.serial_connection and self.serial_connection.is_open:
            self.serial_connection.close()
            return True, "Session closed"
