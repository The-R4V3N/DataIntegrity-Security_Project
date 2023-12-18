# Autor: Oliver Joisten
# Desccription: This file contains the communictaion module which is used int the client.py file to send and receive messages from the server.

import serial

BAUD_RATE = 115200


class SerialCommunication:
    def __init__(self):
        self.serial_connection = None

    def establish_connection(self, port, BAUD_RATE=BAUD_RATE, timeout=1):
        try:
            self.serial_connection = serial.Serial(
                port, baudrate=BAUD_RATE, timeout=1)
            return True, f"Session Established on {port}"
        except serial.SerialException as e:
            return False, f"Failed to open port {port}: {e}"

    def send_data(self, data):
        if self.is_connected():
            try:
                self.serial_connection.write(data.encode())
                return True, f"Data sent: {data}"
            except Exception as e:
                return False, f"Failed to send data: {e}"
        else:
            return False, f"Session not established"

    def toggle_led(self):
        if self.is_connected():
            try:
                return self.serial_connection.readline().decode('utf-8').strip()
            except Exception as e:
                return f"Error reading from serial: {e}"
            else:
                return "Not connected to any serial port"

    def is_connected(self):
        return self.serial_connection is not None and self.serial_connection.is_open

    def serial_read(self):
        if self.serial_connection and self.serial_connection.in_waiting:
            return self.serial_connection.readline().decode('utf-8').strip()
        return None

    def close_connection(self):
        if self.serial_connection and self.serial_connection.is_open:
            self.serial_connection.close()
            return True, f"Session closed"
