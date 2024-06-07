"""
    * @File: communication.py 
    * @Autor: Oliver Joisten    (contact@oliver-joisten.se)
    * @Desccription: This file contains a event handler and a Serial communication class which is used to send and receive messages from the server.
    * @Version: 1.0
    * @Created: 2021-06-15
"""

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

    def communication_open(self) -> bool:
        if not self.ser.is_open:
            self.ser.open()
        return self.ser.is_open
    
    def communication_close(self):
        if self.ser:
            self.ser.close()

