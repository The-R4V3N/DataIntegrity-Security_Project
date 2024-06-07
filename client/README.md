# Client-Side Application of the Project

Welcome to the client-side portion of my project! This directory houses the key components necessary for the client-side functionality. Here's a quick guide to help you understand and navigate through this section of the project.

## Key Modules

my client-side application comprises three essential modules:

1. **`gui.py`** - This is the Graphical User Interface (GUI) module. It's responsible for presenting a user-friendly interface to interact with the application. Through this GUI, users can send requests and receive responses from the server.

2. **`communication.py`** - This module manages all communication between the client and the server. It's crucial for sending requests and receiving data from the server, ensuring smooth data exchange.

3. **`security.py`** - Security is paramount, and this module handles all the security aspects of the client-side operations. It includes implementing encryption and other security protocols to safeguard data integrity and privacy.

## Features

The client-side application offers several interactive features:

- **Temperature Inquiry:** Request the current temperature data from the server.
- **LED State Management:** Query the current state of the LED on the server and send commands to turn the LED on or off.

## Dependencies

To ensure the client-side application runs smoothly, it relies on the following dependencies:

- **Python 3.10.12:** The core programming language used.
- **Tkinter:** A Python library for creating the graphical user interface.
- **Pyserial:** Essential for handling serial communication.
- **python-mbedtls:** Provides security features like encryption.

## Installation Guide

To set up the client-side application, follow these steps:

1. Navigate to the root of the project directory in your terminal.
2. Change to the client directory with the command: `cd client`.
3. Install all required dependencies by executing: `pip3 install -r requirements.txt`.
4. Navigate back to the root of the project directory with: `cd ..`.

## How to Use

Once installation is complete, you can start using the client-side application by running the following command in your terminal:

```bash
python3 -m client.src.client
```

Before you run the client-side application, ensure that the server-side application is flashed to the micro- controller and running before you try to establish a connection.
