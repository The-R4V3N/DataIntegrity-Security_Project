# Author: Oliver Joisten
# Description: This file contains the graphical user interface module which is used in the client.py file to display the GUI.

import serial
import serial.tools.list_ports
import tkinter as tk
from tkinter import scrolledtext
from tkinter import ttk
from client.lib.communication.communication import SerialCommunication

BAUDRATE = 115200   # Baud rate for serial communication
RESPONSE = 32       # Response size for the server


class GUI(tk.Frame):
    def __init__(self, root=None):
        """Initializes the GUI object.

        Args:
            root (tk.Tk, optional): The root window. Defaults to None.
        """
        super().__init__(root)
        self.root = root
        self.root.title("Client")
        self.root.geometry("700x600")
        self.serial_comm = None
        self.create_widgets()

    def create_widgets(self):
        """Creates the widgets for the GUI."""
        self.root.columnconfigure(0, weight=1)
        self.root.columnconfigure(1, weight=1)
        self.root.columnconfigure(2, weight=1)
        self.root.columnconfigure(3, weight=1)
        self.root.columnconfigure(4, weight=1)

        self.baud_var = tk.StringVar(self)
        self.baud_var.set(BAUDRATE)

        serial_label = tk.Label(self.root, text="Serial Port:")
        serial_label.grid(column=0, row=0, sticky='W', padx=(0, 5))

        self.available_ports = self.get_serial_ports()
        self.serial_port = ttk.Combobox(self.root, values=self.available_ports)
        self.serial_port.grid(column=1, row=0, sticky='W', padx=(0, 5))

        self.connect_button = tk.Button(
            self.root, text="Establish Session", command=self.toggle_connection)
        self.connect_button.grid(column=2, row=0, sticky='W', padx=(0, 5))

        self.get_temp_button = tk.Button(
            self.root, text="Get Temperature", command=self.get_temperature, state=tk.DISABLED)
        self.get_temp_button.grid(column=3, row=0, sticky='W', padx=(0, 5))

        self.toggle_led_button = tk.Button(
            self.root, text="Toggle LED", command=self.toggle_led, state=tk.DISABLED)
        self.toggle_led_button.grid(column=4, row=0, sticky='W', padx=(0, 5))

        clear_label = tk.Label(self.root, text="Clear Log",
                               fg="blue", cursor="hand2")
        clear_label.grid(column=3, row=1, columnspan=2,
                         sticky='E', padx=(0, 15))
        clear_label.bind("<Button-1>", lambda event: self.clear_log())

        log_label = tk.Label(self.root, text="Log:")
        log_label.grid(column=0, row=1, sticky='W')

        self.log_text = scrolledtext.ScrolledText(
            self.root, height=30, bg="black", fg="white")
        self.log_text.grid(column=0, row=3, columnspan=5, sticky='WE')

        self.log_text.config(state=tk.DISABLED)

        self.update_button_state()

    def get_serial_ports(self):
        """Gets the available serial ports.

        Returns:
            list: A list of available serial ports.
        """
        ports = serial.tools.list_ports.comports()
        return [port.device for port in ports]

    def toggle_connection(self):
        """Toggles the connection to the serial port."""
        if self.serial_comm and self.serial_comm.is_connected():
            self.disconnect()
        else:
            self.establish_session()

    def establish_session(self):
        """Establishes a session with the serial port."""
        port = self.serial_port.get()
        baudrate = self.baud_var.get()
        if port and baudrate and port != "Select a port":
            self.serial_comm = SerialCommunication(
                port=port, baudrate=int(baudrate))
            self.serial_comm.open_connection()
            self.log(f"Connected to {port} at {baudrate} baudrate")
            self.get_temp_button.config(state=tk.NORMAL)
            self.toggle_led_button.config(state=tk.NORMAL)
            self.connect_button.config(text="Disconnect Session")
        else:
            self.log("Invalid Port. Please select a Serial port.")
        self.update_button_state()

    def disconnect(self):
        """Disconnects the session with the serial port."""
        if self.serial_comm:
            self.serial_comm.close_connection()
            self.serial_comm = None
            self.log("Disconnected")
            self.get_temp_button.config(state=tk.DISABLED)
            self.toggle_led_button.config(state=tk.DISABLED)
            self.connect_button.config(text="Establish Session")
        self.update_button_state()

    def update_button_state(self):
        """Updates the state of the buttons based on the connection status."""
        if self.serial_comm and self.serial_comm.is_connected():
            self.connect_button.config(text="Close Session")
            self.get_temp_button.config(state=tk.NORMAL)
            self.toggle_led_button.config(state=tk.NORMAL)
        else:
            self.connect_button.config(text="Establish Session")
            self.get_temp_button.config(state=tk.DISABLED)
            self.toggle_led_button.config(state=tk.DISABLED)

    def log(self, message):
        """Logs a message to the GUI.

        Args:
            message (str): The message to log.
        """
        self.log_text.config(state='normal')
        self.log_text.insert('end', message + '\n')
        self.log_text.config(state='disabled')

    def toggle_led(self):
        """Sends a request to toggle the LED on the server."""
        if self.serial_comm and self.serial_comm.is_connected():
            self.serial_comm.send_data(b"0x49")
            size = RESPONSE
            response = self.serial_comm.receive_data(size)
            self.log("Sent LED toggle request")
            self.log(f"Received response: {response}")

    def clear_log(self):
        """Clears the log."""
        self.log_text.config(state='normal')
        self.log_text.delete('1.0', tk.END)
        self.log_text.config(state='disabled')

    def get_temperature(self):
        """Sends a request to get the temperature from the server."""
        if self.serial_comm and self.serial_comm.is_connected():
            self.serial_comm.send_data(b"0x54")
            size = RESPONSE
            response = self.serial_comm.receive_data(size)
            self.log("Sent temperature request")
            self.log(f"Received temperature data: {response}")

    def on_close(self):
        """Closes the session when the window is closed."""
        self.disconnect()
