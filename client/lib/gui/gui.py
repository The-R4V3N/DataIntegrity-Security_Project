# Autor: Oliver Joisten
# Desccription: This file contains the graphical user interface module which is used in the client.py file to display the GUI.

import re
import tkinter as tk
from tkinter import scrolledtext
from tkinter import ttk


class GUI:
    def __init__(self, root):
        self.root = root
        self.create_widgets()

    def sort_ports(self, ports):
        '''
        Custom sort function to handle sorting of port names.
        It extracts the base name and the number from the port name.
        '''
        match = re.match(r"(/dev/ttyS?)(\d+)", ports.strip())
        if match:
            return (match.group(1), int(match.group(2)))
        return (ports,)

    def create_widgets(self):
        # Create the dropdown for serial ports
        serial_label = tk.Label(self.root, text="Serial Port:")
        serial_label.grid(column=0, row=0, sticky='W')

        # Get the serial ports and sort them
        ports = ["/dev/tty  ", "/dev/tty23", "/dev/tty39", "/dev/tty54", "/dev/ttyS10   ", "/dev/ttyS26",
                 "/dev/tty0 ", "/dev/tty24", "/dev/tty4 ", "/dev/tty55", "/dev/ttyS11   ", "/dev/ttyS27",
                 "/dev/tty1 ", "/dev/tty25", "/dev/tty40", "/dev/tty56", "/dev/ttyS12   ", "/dev/ttyS28",
                               "/dev/tty11", "/dev/tty27", "/dev/tty42", "/dev/tty58    ", "/dev/ttyS14", "/dev/ttyS3",
                               "/dev/tty10", "/dev/tty26", "/dev/tty41", "/dev/tty57    ", "/dev/ttyS13", "/dev/ttyS29",
                               "/dev/tty13", "/dev/tty29", "/dev/tty44", "/dev/tty6     ", "/dev/ttyS16", "/dev/ttyS31",
                               "/dev/tty12", "/dev/tty28", "/dev/tty43", "/dev/tty59    ", "/dev/ttyS15", "/dev/ttyS30",
                               "/dev/tty15", "/dev/tty30", "/dev/tty46", "/dev/tty61    ", "/dev/ttyS18", "/dev/ttyS5",
                               "/dev/tty14", "/dev/tty3 ", "/dev/tty45", "/dev/tty60    ", "/dev/ttyS17", "/dev/ttyS4",
                               "/dev/tty17", "/dev/tty32", "/dev/tty48", "/dev/tty63    ", "/dev/ttyS2 ", "/dev/ttyS7",
                               "/dev/tty16", "/dev/tty31", "/dev/tty47", "/dev/tty62    ", "/dev/ttyS19", "/dev/ttyS6",
                               "/dev/tty19", "/dev/tty34", "/dev/tty5 ", "/dev/tty8     ", "/dev/ttyS21", "/dev/ttyS9",
                               "/dev/tty18", "/dev/tty33", "/dev/tty49", "/dev/tty7     ", "/dev/ttyS20", "/dev/ttyS8",
                               "/dev/tty2 ", "/dev/tty35", "/dev/tty50", "/dev/tty9     ", "/dev/ttyS22",
                               "/dev/tty20", "/dev/tty36", "/dev/tty51", "/dev/ttyprintk", "/dev/ttyS23",
                               "/dev/tty21", "/dev/tty37", "/dev/tty52", "/dev/ttyS0    ", "/dev/ttyS24",
                               "/dev/tty22", "/dev/tty38", "/dev/tty53", "/dev/ttyS1    ", "/dev/ttyS25",
                 ]
        sorted_ports = sorted(ports, key=self.sort_ports)

        self.serial_port = ttk.Combobox(self.root, values=sorted_ports)
        self.serial_port.grid(column=1, row=0, sticky='WE')

        # Create the buttons
        establish_button = tk.Button(
            self.root, text="Establish Session", command=self.establish_session)
        establish_button.grid(column=2, row=0, sticky='WE')

        get_temp_button = tk.Button(
            self.root, text="Get Temperature", command=self.get_temperature)
        get_temp_button.grid(column=3, row=0, sticky='WE')

        toggle_led_button = tk.Button(
            self.root, text="Toggle LED", command=self.toggle_led)
        toggle_led_button.grid(column=4, row=0, sticky='WE')

        clear_button = tk.Button(
            self.root, text="Clear", command=self.clear_log)
        clear_button.grid(column=5, row=0, sticky='WE')

        # Create the log text area
        log_label = tk.Label(self.root, text="Log:")
        log_label.grid(column=0, row=1, sticky='W')

        self.log_text = scrolledtext.ScrolledText(self.root, height=30)
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
