# Autor: Oliver Joisten
# Desccription: This file contains a event handler and a Serial communication class which is used to send and receive messages from the server.

import serial as uart


SERIAL_PORT = None
SERIAL_BAUDRATE = 115200
comm = None


def set_serial_port(port):
    """Sets the serial port."""
    global SERIAL_PORT
    SERIAL_PORT = port


def get_serial_port():
    """Gets the serial port."""
    return SERIAL_PORT


def close_serial_port(self):
    """Closes the serial port."""
    if self.comm:
        self.comm.close()


def write_data(self, data: bytes):
    """Writes data to the serial port.
    Args:
        data (bytes): The data to write.
    """
    if self.comm:
        self.comm.write(data)


def read_data(self, size: int) -> bytes:
    """Reads data from the serial port.
    Args:
        size (int): The number of bytes to read.
    Returns:
        bytes: The data read from the serial port.
    """
    if self.comm:
        return self.comm.read(size)
    return b""


# class CommunicationManager:

#     def __init__(self):
#         self.comm = None

#     def open_serial_port(self, port, baudrate):
#         """Opens a serial port."""
#         self.comm = serial.Serial(port, baudrate)
#         return self.comm

#     def close_serial_port(self):
#         """Closes the serial port."""
#         if self.comm:
#             self.comm.close()

#     def write_data(self, data: bytes):
#         """Writes data to the serial port.

#         Args:
#             data (bytes): The data to write.
#         """
#         if self.comm:
#             self.comm.write(data)

#     def read_data(self, size: int) -> bytes:
#         """Reads data from the serial port.

#         Args:
#             size (int): The number of bytes to read.

#         Returns:
#             bytes: The data read from the serial port.
#         """
#         if self.comm:
#             return self.comm.read(size)
#         return b""
