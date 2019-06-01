# UART Seria Reader Utility

This program reads from a serial port and logs the raw bytes to a log file.

This program is needed because PuTTY and other serial monitors have a hard time logging non-printable byte values. For example PuTTY will replace the value 0x00 with a space character(0x20).

This repository contains 2 files:

-SerialFunctions.py -> This has wrapper functions for the Pyserial functions.
-serialReader.py    -> This contains the main application.

## Setup

This program is confirmed to work on Windows 10.

In order to run this program you must have the following installed:
- Python. (Confirmed to work with Python 3.7.2)
- PySerial. (Confirmed to work with PySerial 3.4)

## Usage

To run the program, open powershell in the folder with the program files.
The program takes 3 arguments: COM port, baudrate and logfile.

For example type:
	python ./serialReader.py "COM3" 115200 "UMSATS_ROCKET.log"
This will record data from COM port 3 at 115200 and save the data to the file UMSATS_ROCKET.log.

When the program is ran, it while take a second to connect to the serial port.
The program will print to the screen when it has started recording data.

To quit the program, 'q' can be pressed.

