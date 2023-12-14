# Autor: Oliver Joisten
# Desccription: This file contains the client class which is used to connect to the server and send messages to it.
# It also contains the main function which is used to start the client.


import tkinter as tk
from client.lib.gui.gui import GUI
# from client.lib.comunication.communication import Communication
# from client.lib.security.security import Security


def main():
    root = tk.Tk()
    root.title("Client")
    app = GUI(root)
    root.mainloop()


if __name__ == '__main__':
    main()
