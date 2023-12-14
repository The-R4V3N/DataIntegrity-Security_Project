# Project Requirements

## Client Side Requirements

### GUI Implementation using Tkinter (`gui.py`)

- **GUI Implementation:**
  - The GUI shall be implemented using Tkinter.

- **Serial Port Selector:**
  - The GUI shall include a dropdown menu to enumerate and select available serial ports.

- **Session Management:**
  - It shall include a button to establish and close sessions, with the label dynamically changing based on session status.

- **UI State Management:**
  - Temperature and LED control buttons shall be disabled when no session is established.

- **Temperature and LED Control:**
  - The GUI shall include a button to request temperature from the ESP32.
  - A button to toggle the LED on the ESP32 shall be included.

- **Logging and Display:**
  - A clear text button or a lable shall be included to clear logs.
  - A read-only textbox shall be included to display temperature, LED state, and other program states.

### Communication Handling (`communication.py`)

- **Serial Protocol Implementation:**
  - Communication shall be handled via a serial protocol.
  - The design shall allow for the future addition of more protocols.

- **Request-Response Mechanism:**
  - The client shall send requests to the server.
  - The server shall respond to the client's requests.
  - The client shall handle and process responses from the server.

### Security Management (`security.py`)

- **Encryption Protocols:**
  - The communication shall be secured using HMAC-SHA256, AES-256, and RSA-2048 protocols.
  - The design shall permit the addition of more protocols in the future.

- **HMAC-SHA256 Usage:**
  - All communications shall be hashed using the HMAC-SHA256 protocol.

- **RSA and AES Key Management:**
  - The AES-256 key shall be encrypted and decrypted using the RSA-2048 protocol.

- **Session Establishment:**
  - The server shall generate a random AES-256 key and a random IV(Initialization Vector) and sent it to the client.
  - The client shall encrypt the AES-256 key and the IV and send it back to the server.
  - The server shall decrypt the AES-256 key and the IV.
  - The client shall generate a random Session ID and send it to the server.
  - The server shall encrypt the Session ID and send it back to the client.
  - The client shall decrypt the Session ID.
  - The client shall send a request to the server to establish a session.
  - The server shall respond to the client's request to establish a session.
  - The client shall handle and process the server's response.
  - The server shall be able to detect a invalied Session ID and IV and send a new one to the client.

## Server Side Requirements

### Session Management

- **Single Session Handling:**
  - The server shall be capable of handling only one client session at a time.

- **Session Expiration:**
  - Sessions shall expire after 1 minute of inactivity.

### Response Handling

- **Process Client Requests:**
  - The server shall handle requests for temperature reading and LED control.

- **Security Protocol Compliance:**
  - The server shall ensure all communications are secured using the defined protocols.

### Hardware Interface

- **LED Control:**
  - The server shall interface with the LED based on client requests.

- **Temperature Sensor Integration:**
  - The server shall retrieve temperature data from the ESP32 core.

## General Requirements

### Installation and Setup

- **Installation:**
  - The project shall provide instructions for installing the project.

- **Dependencies:**
  - The project shall list all required Python packages, including pyserial and python-mbedtls.

- **Environment Setup:**
  - The project shall provide instructions for setting up the development and execution environment.

### Documentation and Testing

- **Comprehensive Documentation:**
  - The project shall include documentation for each module and its functionalities.
  - The documentation shall include UML Diagrams for the system architecture, sequence diagrams for the communication protocol.
  - Instructions for setup and usage shall be provided.
  - The documentation shall be comprehensive and easy to understand.

- **Unit Testing:**
  - The project shall include tests for each major functionality.
  - Guidelines for running tests shall be provided.
