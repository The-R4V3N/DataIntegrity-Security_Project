# Autor: Oliver Joisten
# Desccription: This file contains the main function which starts the client.

from client.lib.gui.gui import GUI
import tkinter as tk


def main():
    """The main function which starts the client."""
    root = tk.Tk()
    app = GUI(root=root)
    app.mainloop()


if __name__ == "__main__":
    main()
