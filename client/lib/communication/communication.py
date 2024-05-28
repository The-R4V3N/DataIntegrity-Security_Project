# Autor: Oliver Joisten
# Desccription: This file contains a event handler and a Serial communication class which is used to send and receive messages from the server.

from abc import ABC, abstractmethod
import serial as uart

BAUDRATE = 115200


class evt_handler(ABC):
    @abstractmethod
    def send_data(self, data: bytes) -> None:
        """
        Abstract method to send data. Subclasses must implement this method.

        Args:
            data (bytes): The data to be sent.
        """
        raise NotImplementedError(
            "The send_data method must be implemented by subclasses")

    @abstractmethod
    def receive_data(self, size: int) -> bytes:
        """
        Abstract method to receive data. Subclasses must implement this method.

        Args:
            size (int): The size of data to receive.

        Returns:
            bytes: The received data.
        """
        raise NotImplementedError(
            "The receive_data method must be implemented by subclasses")

    @abstractmethod
    def close_connection(self) -> None:
        """
        Abstract method to close the connection. Subclasses must implement this method.
        """
        raise NotImplementedError(
            "The close_connection method must be implemented by subclasses")


class SerialCommunication(evt_handler):
    def __init__(self, port, baudrate=BAUDRATE):
        """
        Initializes the SerialCommunication object.

        Args:
            port (str): The port to connect to.
            baudrate (int, optional): The baudrate of the connection. Defaults to BAUDRATE.
        """
        self.ser = None
        self.port = port
        self.baudrate = baudrate

    def open_connection(self):
        """
        Opens the connection to the serial port.

        Raises:
            serial.SerialException: If the port cannot be opened.
        """
        if self.ser and self.ser.is_open:
            raise ConnectionError("Connection is already open.")
        self.ser = uart.Serial(self.port, self.baudrate, timeout=1)

    def send_data(self, data: bytes) -> None:
        """
        Sends data to the connected port.

        Args:
            data (bytes): The data to send.

        Raises:
            serial.SerialTimeoutException: If a timeout occurs during writing.
        """
        if self.ser and self.ser.is_open:
            self.ser.write(data)
            print(f"Sent data: {len(data)} bytes to {self.ser.port}")
        else:
            raise ConnectionError("Connection is not open.")

    def receive_data(self, size: int) -> bytes:
        """
        Receives data from the connected port.

        Args:
            size (int): The size of data to receive.

        Returns:
            bytes: The received data.

        Raises:
            serial.SerialTimeoutException: If a timeout occurs during reading.
        """
        if self.ser and self.ser.is_open:
            data = self.ser.read(size)
            print(f"Received data: {len(data)} bytes from {self.ser.port}")
            return data
        raise ConnectionError("Connection is not open.")

    def close_connection(self) -> None:
        """
        Closes the connection to the serial port.
        """
        if self.ser and self.ser.is_open:
            self.ser.close()
            print(f"Closed connection to {self.ser.port}")

    def is_connected(self) -> bool:
        """
        Checks if the connection is open.

        Returns:
            bool: True if the connection is open, False otherwise.
        """
        return self.ser is not None and self.ser.is_open
