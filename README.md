# Hey 👋, I'm The-R4V3N

## Connect with me  

[![github](https://img.shields.io/badge/github-%2324292e.svg?&style=flat&logo=github&logoColor=white)](https://github.com/The-R4V3N)
[![dev.to](https://img.shields.io/badge/dev.to-%2308090A.svg?&style=flat&logo=dev.to&logoColor=white)](https://dev.to/ther4v3n)
[![linkedin](https://img.shields.io/badge/linkedin-%231E77B5.svg?&style=flat&logo=linkedin&logoColor=white)](https://linkedin.com/in/oliver-joisten)
[![facebook](https://img.shields.io/badge/facebook-%232E87FB.svg?&style=flat&logo=facebook&logoColor=white)](https://www.facebook.com/oliver.joisten)

## Welcome! Glad to see you here  

## DataIntegrity-Security_Project

Examination Project of my Education as Advanced Software Developer Embedded Systems

### License and Tools used

[![License: GPL v2](https://img.shields.io/badge/License-GPL_v2-blue.svg?style=flat)](https://www.gnu.org/licenses/old-licenses/gpl-2.0.en.html)
![Visual Studio Code](https://img.shields.io/badge/Visual%20Studio%20Code-0078d7.svg?style=flat&logo=visual-studio-code&logoColor=white)
![C++](https://img.shields.io/badge/c++-%2300599C.svg?style=flat&logo=c%2B%2B&logoColor=white)
![Python](https://img.shields.io/badge/python-3670A0?style=flat&logo=python&logoColor=ffdd54)
![Client GUI made with Tkinter](https://img.shields.io/badge/Client%20GUI%20made%20with%20Tkinter-8A2BE2.svg?style=flat&logo=tkinter&logoColor=white)
![ESP32](https://img.shields.io/badge/Server%20Platform%20ESP32%20EVB-green.svg?style=flat&logo=ESP32&logoColor=white)

### Table of Contents

- [Media](#media)
- [Features](#features)
- [Prerequisites](#prerequisites)
- [Building](#building)
- [Execute](#execute)
- [Dependencies](#dependencies)
- [License](#license)

### Media

- Project Overview

![Project Overview](/media/overview.png)

- System Architecture UML Diagram

![System Achitecture](/media/system_uml.png)

- Sequence Security Diagram

![Sequence Security Diagram](/media/security_secuence_diagram.png)

- Sequence Diagram

![Sequence Diagram](/media/system_sequence_uml.png)

## Features

- This project showcast a secure communication between a client and a server. The client is a desktop application written in Python 3.10.12 and the server is a ESP32 microcontroller written in C++.
- The communication is secured using the following protocols:
  - HMAC-SHA256
  - AES-256
  - RSA-2048

- The client can send requests to the server and the server will respond to the client's requests.
- The Following requests are implemented:
  - Core Temperature reading
  - LED control

- The server can handle only one client session at a time.
- Sessions will expire after 1 minute of inactivity.

### Prerequisites

- **Python 3** with the following librarys: Tkinter, pyserial, python-mbedtls
- **PlatformIO** (for building and uploading firmware to the ESP32)
- **ESP32** Used in this Project is a Olimax ESP32 Development Board

### Building

- **Build the Server:**
Execute `platformio run` in the server directory to compile the project.

### Execute

-**Using Makefile:**
This project has a Makefile that can be used to build and run the project.
To build and run the server execute `make server` in the root of the project.
To run the client execute `make client` in the root of the project.
Additionally, you can run `make clean` to remove all compiled files and cache files.
You also have the option to use `make .PHONY` to run all the above commands in sequence. Starting with the server, then the client.

-**Run the Server:**
Execute `platformio run --target upload` to upload the compiled code to the Olimax ESP32-EVB board.

-**Run the Client:**
Execute `python3 -m client.src.client` in the root directory of the project to start the client application.

### Desktop Client and Server

- The Desktop Client  is a Python application with a basic GUI made with Tkinter.
- The servier is a ESP32 microcontroller that communicates with the client over serial communication.

### Dependencies

- **PlatformIO:** An ecosystem for IoT development.
- **C++ Standard Libraries:** For core functionalities.
- **ESP32-EVB Board Support:** Specific libraries and drivers for the Olimax ESP32-EVB board.
- **Python 3.10.12:** The core programming language used.
- **Tkinter:** A Python library for creating the graphical user interface.
- **Pyserial:** Essential for handling serial communication.
- **python-mbedtls:** Provides security features like encryption.

### License

- This project is licensed under the [GNU License](https://github.com/The-R4V3N/DataIntegrity-Security_Project/blob/master/LICENSE).

## Additional Information

- **Detailed Documentation:**
Find comprehensive documentation for each module in the `Respective README file` folder.
- **Client README:** [Client README](client/README.md)
- **Server README:** [Server README](server/README.md)

![The-R4V3N](https://github.com/The-R4V3N.png?size=50) More about me can be found on my [Website](https://www.oliver-joisten.se)
