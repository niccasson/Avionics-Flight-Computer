import datetime
from data_reader import SerialFunctions
import threading
import sys


mutex = threading.Lock()
buffer = [""]

def serialFunc(lock,serial_obj,buf):
    '''

    This function reads from the specified serial port.
    The bytes read are saved to the specified log file.
    This should be run as a thread, and will keep recording until
    iit acquires the mutex.

    Parameters:

        lock:           A mutex. The recording will stop when the mutex is acquired.
        baud_rate:      An integer that specifies the baud rate for serial communication.
        com_port:       A string containing the COM port to connect to. Should be of the form COMx where x is a number.
        log_file_name:  A string containing the file name of the output file.

    '''

    # This line is inserted as the first line in the log. Doesn't really matter what it is,
    # the formater C program expects to skip the first two lines. The second line to skip is
    # sent by the flight computer.
    first_line = datetime.date.today().strftime("%I:%M%p on %B %d, %Y")+"\n"

    # Convert the string to bytes, since our log file is open in binary mode.
    line_bytes = first_line.encode('ascii')

    serial_obj.file.write(line_bytes)

    # print("Now reading from {} at {} baud.\n Output will be in {}\n".format(com_port,baud_rate,log_file_name))

    while True:

        data = (serial_obj.read()).decode('UTF-8')
        #lock.acquire()
        buf[0] = buf[0] + data;
        #lock.release()
        #print("thread: {}".format(id(buf)))
        #print(str(serial_obj.read()))
        #print("thread buff: {}".format(buf[0]))
        if lock.acquire(False):
            break;

    serial_obj.file.close()
    print("Closed Log File!\n")

    serial_obj.s.close()
    print("Closed serial port!\n")


def run(num_args,com,baudRate,outName):

    if num_args != 4 and num_args != 2:
        print(
            "You entered the wrong number of arguments.\n Type serialReader.py help to see the instructions.\n Terminating Program!\n")
        sys.exit()

    if num_args == 2 and sys.argv[1] == "help":
        print('''
    This program reads from a serial port and saves the bytes to a log file. It is meant to be used with the 'formater' C program.

    To run this program enter:  python serialReader.py com_port baud_rate log_file

        com_port    String specifying the COM port to read from. Should be of the format:  "COMx"  where x is a number.
        baud_rate   Integer specifying the baud rate for serial communication. 
        log_file    String containing the name of the log file. Should be of the format: "logFileName.log"

            ''')
        sys.exit()

    S = SerialFunctions(baudRate, com, outName)

    serialThread = threading.Thread(group=None, target=serialFunc,
                                    kwargs={'lock': mutex, 'serial_obj': S,'buf':buffer}, name="serialThread")

    mutex.acquire()

    serialThread.start()

    while True:

        a = input("Press q to stop:")

        if a == 'q':
            mutex.release()

            serialThread.join()

            break;

def run(serialObj):

    serialThread = threading.Thread(group=None, target=serialFunc,
                                    kwargs={'lock': mutex, 'serial_obj': serialObj, 'buf': buffer}, name="serialThread")

    mutex.acquire()

    serialThread.start()

    # while True:
    #
    #     a = input("Press q to stop:")
    #
    #     if a == 'q':
    #         mutex.release()
    #
    #         serialThread.join()
    #
    #         break;

    return serialThread, mutex


if __name__ == "__main__":


    run(len(sys.argv),sys.argv[1],sys.argv[2],sys.argv[3])