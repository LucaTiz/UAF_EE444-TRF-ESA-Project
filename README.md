# EE444 Project: ToF Analysis and RF Transmitter System for Space-Bound ESAs

This repository contains the firmware and documentation for a project focused on acquiring sensor data using a Time-to-Digital Converter (TDC), processing it on an MSP430 microcontroller, and transmitting it wirelessly. The system is designed to calculate electron energy binning and the angle of approach based on TDC inputs.

## Hardware Components

The project utilizes the following hardware components:

*   **TDC Board:** TI TDC7201-ZAX-EVM evaluation board
*   **MCU/RF Board:** TI EM430F6137RF900 development board (from the CC430 family)
*   **Wireless Module:** CC1101 wireless RF module
*   **Interface:** PC terminal interface

## Software and Tools

The firmware is developed using the following software stack:

*   **IDE:** CrossStudio for MSP430
*   **Language:** Embedded C
*   **Terminal:** Tera Term

## Initial Project Plan and Tasks

The following is a list of tasks for the project, broken down by functionality.

### Core Configuration
- [ ] Configure internal TDC SPI as a slave and send the clock. (Owner: Ray)
- [ ] Configure MSP430 general SPI connections and UART communication. (Owner: Wenceslao)

### Data Processing
- [ ] Calculate the angle of electron approach from TDC output. (Owner: Luca)
- [ ] Structure the processed data into a packet `char` array. (Owner: Adrian)

### Communication and Power Management
- [ ] Send a data packet to the console via UART USB or wireless UART. (Owner: Adrian)
- [ ] Configure the CC430 for sub-GHz wireless communication and power optimization.
- [ ] Implement the CC430 RF data interface.
- [ ] Simulate sensor acquisition, storage, and relay to optimize power management.


# ---- Current Working To-Do list (created: 4/14) ----

### TDC code
 - [ ] Impliment constant 8MHz clock for TDC operation.
 - [ ] Clean up file structure - Put SPI Read/Write functions in separate file.
 - [ ] Write main measurement loop commands 
        - [ ] use TDC external interrupt to trigger read3 commands.
 - [ ] Use TDC datasheet to impliment calculation of TOFs (TDC combined mode).
 - [ ] Compare time of flights to determine position (degrees).

 ### UART Communication   

