# Author: Oliver Joisten
# Description: This file contains the graphical user interface module which is used in the client.py file to display the GUI.

import serial as uart
from mbedtls import pk, hmac, hashlib, cipher
import serial.tools.list_ports
import tkinter as tk
from tkinter import scrolledtext
from tkinter import ttk

RSA_SIZE = 256
EXPONENT = 65537
SECRET_KEY = b"Fj2-;wu3Ur=ARl2!Tqi6IuKM3nG]8z1+"

HMAC_KEY = hashlib.sha256()
HMAC_KEY.update(SECRET_KEY)
HMAC_KEY = HMAC_KEY.digest()
hmac_hash = hmac.new(HMAC_KEY, digestmod="SHA256")


def get_serial_ports():
    """Gets the available serial ports.

    Returns:
        list: A list of available serial ports.
    """
    ports = serial.tools.list_ports.comports()
    return [port.device for port in ports]


SERIAL_PORT = None
SERIAL_BAUDRATE = 115200
comm = None


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
        self.root.resizable(False, False)
        self.serial_comm = None
        self.session_active = False
        self.create_widgets()

    def create_widgets(self):
        """Creates the widgets for the GUI."""
        self.root.columnconfigure(0, weight=1)
        self.root.columnconfigure(1, weight=1)
        self.root.columnconfigure(2, weight=1)
        self.root.columnconfigure(3, weight=1)
        self.root.columnconfigure(4, weight=1)

        self.baud_var = tk.StringVar(self)

        serial_label = tk.Label(self.root, text="Serial Port:")
        serial_label.grid(column=0, row=0, sticky='W', padx=(0, 5))

        self.available_ports = get_serial_ports()
        self.serial_port = ttk.Combobox(self.root, values=self.available_ports)
        self.serial_port.grid(column=1, row=0, sticky='W', padx=(0, 5))
        self.serial_port.bind("<<ComboboxSelected>>", self.select_serial_port)

        self.session_button = tk.Button(
            self.root, text="Establish Session", command=self.toggle_session)
        self.session_button.grid(column=2, row=0, sticky='W', padx=(0, 5))

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

    def select_serial_port(self, event):
        """Selects the serial port from the dropdown."""
        global SERIAL_PORT
        SERIAL_PORT = self.serial_port.get()

    def update_button_state(self):
        """Updates the state of the buttons based on the connection status."""
        if self.serial_comm and self.is_connected():
            self.session_button.config(text="Close Session")
            self.get_temp_button.config(state=tk.NORMAL)
            self.toggle_led_button.config(state=tk.NORMAL)
        else:
            self.session_button.config(text="Establish Session")
            self.get_temp_button.config(state=tk.DISABLED)
            self.toggle_led_button.config(state=tk.DISABLED)

    def toggle_session(self):
        """Toggles the session."""
        if not self.session_active:
            try:
                global comm
                if SERIAL_PORT is None:
                    self.log("No serial port selected!")
                    return

                self.log("Please wait until session is established...")
                self.root.update()  # Update the GUI to show the log message
                comm = uart.Serial(SERIAL_PORT, SERIAL_BAUDRATE)
                self.session_button.config(text="Close Session")
                self.session_active = True
                self.get_temp_button.config(state="normal")
                self.toggle_led_button.config(state="normal")
                self.establish_session()
            except uart.SerialException:
                self.log(
                    "Failed to establish session with {}!".format(SERIAL_PORT))
        else:
            self.close_session()
            self.update_button_state()

    def close_session(self):
        """Closes the session."""
        global comm
        comm.write(b"close")
        comm.close()
        self.session_button.config(text="Establish Session")
        self.session_active = False
        self.log("Session closed")
        self.get_temp_button.config(state="disabled")
        self.toggle_led_button.config(state="disabled")
        self.update_button_state()

    def establish_session(self):
        """Establishes a session with the server."""
        global rsa, hmac_hash

        rsa = pk.RSA()
        rsa.generate(RSA_SIZE * 8, EXPONENT)
        self.send_data(rsa.export_public_key())

        buffer = self.receive_data(2 * RSA_SIZE)
        public_key_server = rsa.decrypt(buffer[0:RSA_SIZE])
        public_key_server += rsa.decrypt(buffer[RSA_SIZE:2 * RSA_SIZE])
        self.server_rsa = pk.RSA().from_DER(public_key_server)

        rsa = pk.RSA()
        rsa.generate(RSA_SIZE * 8, EXPONENT)
        buffer = rsa.export_public_key() + rsa.sign(HMAC_KEY, "SHA256")
        buffer = self.server_rsa.encrypt(buffer[0:184]) + self.server_rsa.encrypt(
            buffer[184:368]) + self.server_rsa.encrypt(buffer[368:550])
        self.send_data(buffer)

        buffer = self.receive_data(RSA_SIZE)
        if b"OKAY" == rsa.decrypt(buffer):
            buffer = rsa.sign(HMAC_KEY, "SHA256")
            buffer = self.server_rsa.encrypt(
                buffer[0:RSA_SIZE//2]) + self.server_rsa.encrypt(buffer[RSA_SIZE//2:RSA_SIZE])
            self.send_data(buffer)

            buffer = self.receive_data(RSA_SIZE)
            buffer = rsa.decrypt(buffer)
            self.session_id = buffer[0:8]

            self.aes = cipher.AES.new(
                buffer[24:56], cipher.MODE_CBC, buffer[8:24])
            self.log(
                "Session Established on {} and Keys Successful Exchanged!" .format(SERIAL_PORT))

    def send_data(self, buf: bytes):
        """Sends data to the server.

        Args:
            buf (bytes): The data to send.
        """
        global hmac_hash
        hmac_hash.update(buf)
        buf += hmac_hash.digest()
        if len(buf) != comm.write(buf):
            self.log("Connection Error")
            self.close_session()
            exit(1)

    def receive_data(self, size: int) -> bytes:
        """Receives data from the server.

        Args:
            size (int): The size of data to receive.

        Returns:
            bytes: The received data. Returns an empty byte string if there is a hash error.
        """
        global hmac_hash
        buffer = comm.read(size + hmac_hash.digest_size)
        hmac_hash.update(buffer[0:size])
        buff = buffer[size:size + hmac_hash.digest_size]
        dig = hmac_hash.digest()
        if buff != dig:
            self.log("Hash Error")
            self.close_session()
            exit(1)
        return buffer[0:size]

    def get_temperature(self):
        """Gets the temperature from the server."""
        if self.session_active:
            self.send_command(0x01)
        else:
            self.log("No active session")

    def toggle_led(self):
        """Toggles the LED on the server."""
        if self.session_active:
            self.send_command(0x02)
        else:
            self.log("No active session")

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

    def log(self, message):
        """Logs a message to the log window.

        Args:
            message (str): The message to log.
        """
        self.log_text.config(state=tk.NORMAL, foreground="white")
        self.log_text.insert(tk.END, message)
        self.log_text.insert(tk.END, "\n")
        self.log_text.config(state=tk.DISABLED)

    def clear_log(self):
        """Clears the log."""
        self.log_text.config(state='normal')
        self.log_text.delete('1.0', tk.END)
        self.log_text.config(state='disabled')

    def on_close(self):
        """Closes the session when the window is closed."""
        self.disconnect()
