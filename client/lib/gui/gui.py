# Autor: Oliver Joisten
# Desccription: This file contains the graphical user interface module which is used in the client.py file to display the GUI.

import serial
import serial.tools.list_ports
import threading
import tkinter as tk
from tkinter import scrolledtext
from tkinter import ttk
from client.lib.communication.communication import SerialCommunication
# from client.lib.security.security import Security


class GUI:
    def __init__(self, root):
        self.root = root
        self.root.geometry("700x600")
        self.serial_comm = SerialCommunication()
        self.create_widgets()

    def create_widgets(self):
        # Configure column weights
        self.root.columnconfigure(0, weight=1)
        self.root.columnconfigure(1, weight=1)
        self.root.columnconfigure(2, weight=1)
        self.root.columnconfigure(3, weight=1)
        self.root.columnconfigure(4, weight=1)

        # Create the dropdown for serial ports
        serial_label = tk.Label(self.root, text="Serial Port:")
        serial_label.grid(column=0, row=0, sticky='W', padx=(0, 5))

        # Get list of available serial ports
        self.available_ports = self.list_serial_ports()
        self.serial_port = ttk.Combobox(self.root, values=self.available_ports)
        self.serial_port.grid(column=1, row=0, sticky='W', padx=(0, 5))

        # Create the establish session button
        self.establish_button = tk.Button(
            self.root, text="Establish Session", command=self.establish_session)
        self.establish_button.grid(column=2, row=0, sticky='W', padx=(0, 5))

        # Create the disconnect button
        self.disconnect_button = tk.Button(
            self.root, text="Disconnect Session", command=self.on_close)
        self.disconnect_button.grid(column=2, row=0, sticky='W', padx=(0, 5))

        # Create the get temperature button
        self.get_temp_button = tk.Button(
            self.root, text="Get Temperature", command=self.get_temperature, state=tk.DISABLED)
        self.get_temp_button.grid(column=3, row=0, sticky='W', padx=(0, 5))

        # Create the toggle led button
        self.toggle_led_button = tk.Button(
            self.root, text="Toggle LED", command=self.toggle_led, state=tk.DISABLED)
        self.toggle_led_button.grid(column=4, row=0, sticky='W', padx=(0, 5))

        # Create the clear log label
        clear_label = tk.Label(self.root, text="Clear Log",
                               fg="blue", cursor="hand2")
        clear_label.grid(column=3, row=1, columnspan=2,
                         sticky='E', padx=(0, 15))
        clear_label.bind("<Button-1>", lambda event: self.clear_log())

        # Create the log text area
        log_label = tk.Label(self.root, text="Log:")
        log_label.grid(column=0, row=1, sticky='W')

        self.log_text = scrolledtext.ScrolledText(
            self.root, height=30, bg="black", fg="white")
        self.log_text.grid(column=0, row=3, columnspan=5, sticky='WE')

        # disable user editing
        self.log_text.config(state=tk.DISABLED)

        self.update_button_state()

    def system_log(self, message):
        """
        Inserts a message into the log text area. This method temporarily
        enables the text area to insert the message, then disables it.
        """
        self.log_text.config(state=tk.NORMAL)
        self.log_text.insert(tk.END, message + "\n")
        self.log_text.config(state=tk.DISABLED)

    def list_serial_ports(self):
        """Lists serial port names

        :returns:
            A list of the serial ports available on the system
        """
        ports = serial.tools.list_ports.comports()
        return [port.device for port in ports]

    def update_button_state(self):
        if self.serial_comm.is_connected():
            self.disconnect_button.grid(column=2, row=0, sticky='WE', padx=5)
            self.establish_button.grid_remove()
            self.get_temp_button['state'] = tk.NORMAL
            self.toggle_led_button['state'] = tk.NORMAL
        else:
            self.establish_button.grid(column=2, row=0, sticky='WE', padx=5)
            self.disconnect_button.grid_remove()
            self.get_temp_button['state'] = tk.DISABLED
            self.toggle_led_button['state'] = tk.DISABLED

    def establish_session(self):
        port = self.serial_port.get()
        if port != "":
            success, message = self.serial_comm.establish_connection(port)
            self.system_log(message)
            self.update_button_state()
            threading.Thread(target=self.read_serial_data, daemon=True).start()
        else:
            self.system_log(
                "No Serial Port Selected!\nPlease select a Serial Port")

    def read_serial_data(self):
        while self.serial_comm.is_connected():
            try:
                data = self.serial_comm.serial_read()
                if data:
                    self.root.after(0, self.system_log, data)
            except Exception as e:
                break

    def get_temperature(self):
        if self.serial_port.get() != "":
            self.system_log("Getting Temperature")
            success, message = self.serial_comm.send_data("read_temp")
            self.system_log(message)
        else:
            self.system_log(
                "No Serial Port Selected or Connection not open!\nPlease select a Serial Port")

    def toggle_led(self):
        if self.serial_comm.is_connected():
            self.system_log("Toggling LED")
            success, message = self.serial_comm.send_data("toggle_led_state")
            self.system_log(message)
        else:
            self.system_log(
                "No Serial Port Selected or Connection not open!\nPlease select and connect a Serial Port")

    def clear_log(self):
        self.log_text.config(state=tk.NORMAL)
        self.log_text.delete('1.0', tk.END)

        # Disable the widget again
        self.log_text.config(state=tk.DISABLED)

    def on_close(self):
        self.serial_comm.close_connection()
        self.system_log("Session closed")
        self.update_button_state()


# Create the main window and run the application
if __name__ == "__main__":
    root = tk.Tk()
    app = GUI(root)
    root.mainloop()
