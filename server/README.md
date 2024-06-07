# Server-Side Application of the Project

Welcome to the server-side of my project, developed using C++ and PlatformIO. This application runs on the Olimex ESP32-EVB development board and incorporates robust security protocols for secure communication.

## Key Components

The server-side application is organized into the following C++ modules:

1. **Session Module** - Manages client sessions, particularly ensuring efficient handling of a single session at a time, as per project requirements. It also handles advanced security measures, including HMAC-SHA256, AES-256, and RSA-2048 encryption protocols, to secure data transmission.
2. **Communication Module** - Handles the communication protocol with the client, using secure methods as specified in the project requirements.
3. **Main Source** - The main source file that orchestrates the server-side application, including session management and communication handling.

## Features

The server-side offers functionalities including:

- **LED Control:** Manages the LED states based on client requests.
- **Temperature Reporting:** Retrieves and sends temperature data from the ESP32's sensors to the client.
- **Session Management:** Handles session creation, maintenance, and expiration, with a focus on single-session handling and automatic session expiration after a predefined period of inactivity.

## Hardware

- **Olimex ESP32-EVB:** This development board is the core of my server-side application, featuring Wi-Fi and Bluetooth capabilities, along with various input/output interfaces.

## Dependencies

- **PlatformIO:** An ecosystem for IoT development.
- **C++ Standard Libraries:** For core functionalities.
- **Arduino Core:** For ESP32 development.
- **ESP32-EVB Board Support:** Specific libraries and drivers for the Olimex ESP32-EVB board.
- **mbedTLS:** For implementing cryptographic functions like HMAC, AES, and RSA.

## Installation and Setup

To set up the server-side application:

1. **Install PlatformIO Core:** Follow the installation guide at [PlatformIO Installation Guide](https://docs.platformio.org/en/latest/core/installation.html).
2. **Clone the Repository:** Obtain the project repository on your machine.
3. **Navigate to the Server Directory:** Switch to the server directory in the terminal.
4. **Install Dependencies:** PlatformIO automatically handles the dependencies when you build the project.

## Building and Running the Server

To build and run the server-side application on the ESP32-EVB:

1. **Build the Project:** Execute `platformio run` in the server directory to compile the project.
2. **Upload to the ESP32-EVB:** Use `platformio run --target upload` to upload the compiled code to the Olimex ESP32-EVB board.
3. **Makefile:** You can also use the provided Makefile to build and upload the project. Run `make` to build and `make client` to upload the code to the board.

With this setup, you are ready to deploy and operate the server-side of my project on the Olimex ESP32-EVB development board!
