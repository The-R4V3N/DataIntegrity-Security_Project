"""
    * @File: gui.py
    * @Author: Oliver Joisten   (contact@oliver-joisten.se)
    * @Description: This file contains the graphical user interface module which is used in the client.py file to display the GUI.
    * @Version: 1.0
    * @Created: 2021-06-15
"""

import tkinter as tk
from tkinter import scrolledtext
from tkinter import ttk
import serial.tools.list_ports
from client.lib.session.session import Session

ports = None
__GET_TEMP__ = 0x03
__Toggle_LED__ = 0x02


def get_serial_ports():
    global ports
    ports = serial.tools.list_ports.comports()
    return [port.device for port in ports]


class GUI(tk.Frame):
    def __init__(self, root=None):
        super().__init__(root)
        self.root = root
        self.root.title("Client")
        self.root.geometry("700x600")
        self.root.resizable(False, False)
        self.session = None
        self.session_active = False
        self.create_widgets()
        self.serial_port = None
        self.log_text_exists = lambda: hasattr(self, 'log_text')

    def create_widgets(self):
        self.root.columnconfigure(0, weight=1)
        self.root.columnconfigure(1, weight=1)
        self.root.columnconfigure(2, weight=1)
        self.root.columnconfigure(3, weight=1)
        self.root.columnconfigure(4, weight=1)

        serial_label = tk.Label(self.root, text="Serial Port:")
        serial_label.grid(column=0, row=0, sticky='W', padx=(0, 5))

        self.available_ports = get_serial_ports()
        self.serial_port_combobox = ttk.Combobox(
            self.root, values=self.available_ports)
        self.serial_port_combobox.grid(
            column=1, row=0, sticky='W', padx=(0, 5))
        self.serial_port_combobox.bind(
            "<<ComboboxSelected>>", self.on_port_selected)

        self.session_button = tk.Button(
            self.root, text="Establish Session", command=self.establish_session)
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

        self.update_button_state(False)


    def on_port_selected(self, event=None):
        selected_port = self.serial_port_combobox.get()
        if self.session and self.session.port == selected_port:
            self.log("Key exchange already done for this port.")
            self.root.update()
        else:
            self.session = Session(selected_port)
            status = self.session.key_exchange()

            if status == True:
                self.log("Session Keys are successfully exchanged.")
            else:
                self.log("Error exchanging security keys. Please try again.")
                self.session = None

    def initialize(self):

        return True

    def update_button_state(self, enable=False):
        state = tk.NORMAL if enable else tk.DISABLED
        self.get_temp_button.config(state=state)
        self.toggle_led_button.config(state=state)

    def establish_session(self):
        if self.session and not self.session_active:
                self.session.authenticate()
                self.session_active = True
                self.update_button_state(True)
                self.log("Session Established and Active.")
                self.session_button.config(text="Close Session")
        elif self.session_active:
                self.close_session()
                self.session_button.config(text="Establish Session")
        elif not self.session:
                self.log(
                    "Authentication failed. Please check the session and try again.")
        else:
            self.log(
                "Please select a serial port and exchange keys before establishing the session.")

    def close_session(self):
        self.session.close_session()
        self.session_active = False
        self.update_button_state(False)
        self.log("Session is closed")

    def get_temperature(self):
        if self.session_active:
            response = self.session.requests(__GET_TEMP__)
            self.log(response) 


    def toggle_led(self):
        if self.session_active:
            response = self.session.requests(__Toggle_LED__)
            self.log(response)

    def error_handling(self, message):
        
        self.log(message)


    def log(self, message):
        self.log_text.config(state=tk.NORMAL)
        self.log_text.insert(tk.END, message + "\n")
        self.log_text.config(state=tk.DISABLED)


    def clear_log(self):
        self.log_text.config(state=tk.NORMAL)
        self.log_text.delete('1.0', tk.END)
        self.log_text.config(state=tk.DISABLED)

    def on_close(self):
        if self.session_active:
            self.close_session()
        self.root.destroy()
