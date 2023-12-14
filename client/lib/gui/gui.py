# Autor: Oliver Joisten
# Desccription: This file contains the graphical user interface module which is used in the client.py file to display the GUI.

import re
import tkinter as tk
from tkinter import scrolledtext
from tkinter import ttk

selected_port = "/dev/ttyUSB0"


class GUI:
    def __init__(self, root):
        self.root = root
        self.root.geometry("700x600")
        self.create_widgets()

    def create_widgets(self):
        # Create the dropdown for serial ports
        serial_label = tk.Label(self.root, text="Serial Port:")
        serial_label.grid(column=0, row=0, sticky='W')

        self.serial_port = ttk.Combobox(self.root, values=selected_port)
        self.serial_port.grid(column=1, row=0, sticky='WE')

        # Create the wstablish session button
        establish_button = tk.Button(
            self.root, text="Establish Session", command=self.establish_session)
        establish_button.grid(column=2, row=0, sticky='WE')

        # Create the get temperature button
        get_temp_button = tk.Button(
            self.root, text="Get Temperature", command=self.get_temperature)
        get_temp_button.grid(column=3, row=0, sticky='WE')

        # Create the toggle led button
        toggle_led_button = tk.Button(
            self.root, text="Toggle LED", command=self.toggle_led)
        toggle_led_button.grid(column=4, row=0, sticky='WE')

        # Create the clear log label
        clear_label = tk.Label(self.root, text="Clear Log",
                               fg="blue", cursor="hand2")
        clear_label.grid(column=5, row=0, sticky='WE')
        clear_label.bind("<Button-1>", lambda event: self.clear_log())

        # Create the log text area
        log_label = tk.Label(self.root, text="Log:")
        log_label.grid(column=0, row=1, sticky='W')

        self.log_text = scrolledtext.ScrolledText(
            self.root, height=30, bg="black", fg="white")
        self.log_text.grid(column=0, row=2, columnspan=6, sticky='WE')

    def establish_session(self):
        if self.serial_port.get() != "":
            self.log_text.insert(tk.END, "Session Established\n")
        else:
            self.log_text.insert(
                tk.END, "No Serial Port Selected!\nPlease select a Serial Port\n")

    def get_temperature(self):
        if self.serial_port.get() != "":
            self.log_text.insert(
                tk.END, "Getting Temperature\n")
        else:
            self.log_text.insert(
                tk.END, "No Serial Port Selected!\nPlease select a Serial Port\n")

    def toggle_led(self):
        if self.serial_port.get() != "":
            self.log_text.insert(tk.END, "Toggling LED\n")
        else:
            self.log_text.insert(
                tk.END, "No Serial Port Selected!\nPlease select a Serial Port\n")

    def clear_log(self):
        self.log_text.delete('1.0', tk.END)
