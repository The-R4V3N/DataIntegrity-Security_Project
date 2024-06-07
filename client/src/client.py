"""
    * @File: client.py
    * @Autor: Oliver Joisten    (contact@oliver-joisten.se)
    * @Desccription: This file contains the main function which starts the client.
    * @Version: 1.0
    * @Created: 2021-06-15
"""

from client.lib.gui.gui import GUI
import tkinter as tk


def main():
    """The main function which starts the client."""
    root = tk.Tk()
    app = GUI(root=root)
    app.mainloop()


if __name__ == "__main__":
    main()
