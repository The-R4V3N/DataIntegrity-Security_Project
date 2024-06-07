# Communication Module

This module handles the communication protocol with the client for the server-side application, developed using C++ and PlatformIO. It runs on the Olimex ESP32-EVB development board and is responsible for establishing and maintaining secure communication with the client.

## Key Components

The Communication module includes the following key functions:

1. **`communication_init`** - Initializes the serial communication with the specified baud rate.
2. **`communication_read`** - Reads data from the serial interface into a buffer.
3. **`communication_write`** - Writes data from a buffer to the serial interface.

## Functions

### `communication_init`

Initializes the serial communication at the defined baud rate.

```cpp
bool communication_init(void)
{
    Serial.begin(BAUDRATE); /**< Initialize the Serial Communication */
    return Serial;          /**< Return the Serial Communication */
}
```

### `communication_read`

Reads data from the serial interface into a buffer.

```cpp
size_t communication_read(uint8_t *buffer, size_t blen)
{
    while (0 == Serial.available()) 
    {
        ; // Wait for data to be available
    }

    return Serial.readBytes(buffer, blen); /**< Read the data from the Serial Communication */
}
```

### `communication_write`

Writes data from a buffer to the serial interface.

```cpp
size_t communication_write(const uint8_t *buffer, size_t blen)
{
    return Serial.write(buffer, blen); /**< Write the data to the Serial Communication */
}
```

## Features

- Initialization: Sets up the serial communication with a specified baud rate.
- Data Reading: Efficiently reads data from the serial interface.
- Data Writing: Securely writes data to the serial interface.

## Hardware

- Olimex ESP32-EVB: This development board is the core hardware for the communication module, featuring Wi-Fi and Bluetooth capabilities, along with various input/output interfaces.

## Dependencies

- PlatformIO: The PlatformIO IDE is used for developing the communication module, providing a powerful and flexible development environment.
- Arduino: The Arduino framework is used for developing the communication module, providing a simple and effective way to interact with the hardware.

## Installation and Setup

To set up the communication module:

- Install PlatformIO Core: Follow the installation guide at PlatformIO Installation Guide.
- Clone the Repository: Obtain the project repository on your machine.
- Navigate to the Communication Directory: Switch to the communication directory in the terminal.
- Install Dependencies: PlatformIO automatically handles the dependencies when you build the project.

## Usage

To use the communication module:

- Include the Header File: Add the communication header file to your project.
- Initialize the Communication: Call the `communication_init` function to set up the serial communication.
- Read Data: Use the `communication_read` function to read data from the serial interface.
- Write Data: Use the `communication_write` function to write data to the serial interface.
- Handle Data: Process the data as needed in your application.
- Close the Communication: Terminate the serial communication when done.
