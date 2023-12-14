# Autor: Oliver Joisten
# Desccription: This file contains the graphical user interface module which is used in the client.py file to display the GUI.

import tkinter as tk
from tkinter import scrolledtext
from tkinter import ttk
from client.lib.communication.communication import SerialCommunication
import serial

selected_port = "/dev/ttyUSB0"


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
        serial_label.grid(column=0, row=0, sticky='W')

        self.serial_port = ttk.Combobox(self.root, values=selected_port)
        self.serial_port.grid(column=1, row=0, sticky='WE')

        # Create the establish session button
        self.establish_button = tk.Button(
            self.root, text="Establish Session", command=self.establish_session)
        self.establish_button.grid(column=2, row=0, sticky='WE', padx=5)

        # Create the disconnect button
        self.disconnect_button = tk.Button(
            self.root, text="Disconnect Session", command=self.on_close)
        self.disconnect_button.grid(column=2, row=0, sticky='WE', padx=5)

        # Create the get temperature button
        get_temp_button = tk.Button(
            self.root, text="Get Temperature", command=self.get_temperature)
        get_temp_button.grid(column=3, row=0, sticky='WE', padx=5)

        # Create the toggle led button
        toggle_led_button = tk.Button(
            self.root, text="Toggle LED", command=self.toggle_led)
        toggle_led_button.grid(column=4, row=0, sticky='WE', padx=(5, 5))

        # Create the clear log label
        clear_label = tk.Label(self.root, text="Clear Log",
                               fg="blue", cursor="hand2")
        clear_label.grid(column=3, row=1, columnspan=2, sticky='E', padx=5)
        clear_label.bind("<Button-1>", lambda event: self.clear_log())

        # Create the log text area
        log_label = tk.Label(self.root, text="Log:")
        log_label.grid(column=0, row=1, sticky='W')

        self.log_text = scrolledtext.ScrolledText(
            self.root, height=30, bg="black", fg="white")
        self.log_text.grid(column=0, row=3, columnspan=5, sticky='WE')

        self.update_button_state()

    def update_button_state(self):
        if self.serial_comm.is_connected():
            self.disconnect_button.grid(column=2, row=0, sticky='WE', padx=5)
            self.establish_button.grid_remove()
        else:
            self.establish_button.grid(column=2, row=0, sticky='WE', padx=5)
            self.disconnect_button.grid_remove()

    def establish_session(self):
        port = self.serial_port.get()
        if port != "":
            success, message = self.serial_comm.establish_connection(port)
            self.log_text.insert(tk.END, message + "\n")
            self.update_button_state()

            ser = serial.Serial(selected_port, 115200)
            self.log_text.insert(
                tk.END, "Your data is:" + ser.readline().decode('utf-8').strip()+"\n")
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

    def on_close(self):
        self.serial_comm.close_connection()
        self.log_text.insert(tk.END, "Session closed\n")
        self.update_button_state()


# Create the main window and run the application
if __name__ == "__main__":
    root = tk.Tk()
    app = GUI(root)
    root.mainloop()
