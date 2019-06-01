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
        self.fileopen = True
        self.s.open()
        
        time.sleep(1)  # Wait for serial port to be open.

    def write(self, data):

        data = data.encode('utf-8')
        self.s.write(data)
        self.s.flush()

    def read(self):
        data = self.s.read()
        if self.fileopen:
            self.file.write(data)
        return data

    
    def close(self):
        self.s.close()

    def close_log(self):
        self.fileopen = False
        self.file.close()

    def open_log(self,filename):
        self.file = open(filename,'a+b')
        self.fileopen = True

    def flush(self):

        self.s.flush()

def list_COM_ports():
    """
    This function lists all available com ports and returns a list of the names.
    return: a list containing the name of each port as a string.
    """
    COMports = []
    portInfoLists = list_ports.comports()

    for port in portInfoLists:
        COMports.append(port.device)


    return COMports