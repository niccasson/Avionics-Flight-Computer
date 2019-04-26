# Data Format Description

The UMSATS flight computer currently uses a variable length data structure to store sensor data.
The motivation for using the following format was to minimize the waste of flash memory.

Currently the flight computer records data from the IMU at twice the rate as from the BMP388 sensor. Using a fixed length packet would result in only 85% of the flash memory being filled with useful data.

The flight computer current records measurements at a certain sampling rate. Each measurement will contain a timestamp and data from the IMU. Approximately every second measurement will also have data from the BMP388 sensor.

## Packet Format

Each packet holds the data from one measurement.
Packets can be either 14 bytes or 20 bytes.
Each packet has a 16 bit header, with the timestamp and the data type.
The header has the least significant byte first!
The type bits are defined in the data_logging.h header file.

Diagrams below show the meaning of the bytes in each packet.



The data is stored in memory starting at address 0. The packets are stored sequentially, and the length of each packet can be found from the data type bits.



