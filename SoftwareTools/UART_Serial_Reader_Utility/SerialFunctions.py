#
#   File Description:
#       This file contains functions related to using the serial ports.
#
#   History
#   2018-05 by Joseph Howarth.
#       -   Created.

import serial
import serial.tools.list_ports as list_ports
import time


class SerialFunctions:

    def __init__(self, baudrate, port, filename):
        """
        This function initializes a serial port object.
        The port parameter should be a string. Ex. "COM12"
        """
    
        self.s = serial.Serial()
        self.s.baudrate = baudrate
        self.s.port = port
        self.s.timeout = 5
        self.file = open(filename,"wb")

        self.s.open()
        
        time.sleep(1)  # Wait for serial port to be open.

    def write(self, data):

        data = data.encode('utf-8')
        self.s.write(data)
        self.s.flush()

    def read(self):
        data = self.s.read()
        self.file.write(data)
        return data

    
    def close(self):
        self.s.close()


