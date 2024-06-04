# Autor: Oliver Joisten
# Desccription: This file contains a event handler and a Serial communication class which is used to send and receive messages from the server.

import serial

BAUDRATE = 115200


class Communication:
    ser = None

    def __init__(self, port, baudrate=BAUDRATE):
        self.ser = serial.Serial(port, baudrate)
        self.log = ("Connected to " + port + " at " + str(baudrate) + " baud")

    def communication_send(self, buffer: bytes):
        return self.ser.write(buffer)

    def communication_read(self, size: int) -> bytes:
        return self.ser.read(size)

    def close_connection(self):
        if self.ser:
            self.ser.close()
            self.ser = None
