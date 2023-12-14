# Autor: Oliver Joisten
# Desccription: This file contains the communictaion module which is used int the client.py file to send and receive messages from the server.

import serial

selected_port = "/dev/ttyUSB0"


class SerialCommunication:
    def __init__(self):
        self.serial_connection = None

    def establish_connection(self, port, baudrate=115200):
        try:
            self.serial_connection = serial.Serial(
                port, baudrate=baudrate, timeout=1)
            return True, f"Session Established on {port}"
        except serial.SerialException as e:
            return False, f"Failed to open port {port}: {e}"

    def serial_read(self):
        # Open the serial port
        ser = serial.Serial(selected_port, 115200)

        try:
            while True:
                # Read data from the serial port
                data = ser.readline().decode('utf-8').strip()

            # Print the received data
            # print("Received:", data)

        except KeyboardInterrupt:
            # Close the serial port when the program is interrupted (e.g., by pressing Ctrl+C)
            ser.close()
            # print("Serial port closed.")

        return data

    def is_connected(self):
        return self.serial_connection is not None and self.serial_connection.is_open

    def close_connection(self):
        if self.serial_connection and self.serial_connection.is_open:
            self.serial_connection.close()
            return True, f"Session closed"
