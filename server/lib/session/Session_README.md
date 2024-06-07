# Session Module

This module manages client sessions for the server-side application, developed using C++ and PlatformIO. It runs on the Olimex ESP32-EVB development board and is responsible for handling session management and implementing robust security protocols.

## Overview

The Session module is a critical component of the server-side application, providing functionalities for session management and secure communication. It utilizes advanced cryptographic techniques to ensure data integrity and confidentiality during communication with the client.

## Key Components

1. **Session Management:** Handles the creation, maintenance, and expiration of client sessions.
2. **Security Protocols:** Implements advanced security measures, including HMAC-SHA256, AES-256, and RSA-2048 encryption protocols, to secure data transmission.
3. **Public Key Exchange:** Securely exchanges public keys between the server and the client.
4. **Session Establishment:** Establishes a secure session with the client, including session ID generation and encryption key setup.

## Features

- **Session Initialization:** Sets up cryptographic contexts and generates RSA keys.
- **Session Request Handling:** Manages incoming client requests and handles secure data communication.
- **Session Response:** Sends encrypted and authenticated responses to client requests.
- **Public Key Exchange:** Securely exchanges public keys with the client.
- **Session Establishment:** Establishes a secure session with the client.

## Hardware

- **Olimex ESP32-EVB:** This development board is the core hardware for the session module, featuring Wi-Fi and Bluetooth capabilities, along with various input/output interfaces.

## Dependencies

- **PlatformIO:** An ecosystem for IoT development.
- **Arduino Core for ESP32:** Provides the necessary functions for serial communication.
- **mbedTLS:** For implementing cryptographic functions like HMAC, AES, and RSA.

## Installation and Setup

To set up the session module:

1. **Install PlatformIO Core:** Follow the installation guide at [PlatformIO Installation Guide](https://docs.platformio.org/en/latest/core/installation.html).
2. **Clone the Repository:** Obtain the project repository on your machine.
3. **Navigate to the Session Directory:** Switch to the session directory in the terminal.
4. **Install Dependencies:** PlatformIO automatically handles the dependencies when you build the project.

## Building and Running the Session Module

To build and run the session module on the ESP32-EVB:

1. **Build the Project:** Execute `platformio run` in the client directory to compile the project.
2. **Upload to the ESP32-EVB:** Use `platformio run --target upload` to upload the compiled code to the Olimex ESP32-EVB board.
3. **Makefile:** You can also use the provided Makefile to build and upload the project. Run `make` to build and `make client` to upload the code to the board.

## Usage

Once the session module is set up and running, it will:

1. **Initialize Sessions:** Set up the necessary cryptographic contexts and generate RSA keys.
2. **Handle Requests:** Manage incoming client requests, including public key exchanges, session establishment, and secure data communication.
3. **Secure Communication:** Use HMAC-SHA256, AES-256, and RSA-2048 encryption protocols to ensure secure data transmission between the server and the client.
4. **Session Responses:** Send encrypted and authenticated responses to client requests.
