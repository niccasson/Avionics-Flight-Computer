# Avionics
Flight Computer Software for the 2019 Rocket.

## Versions
#### v2.0.0 - 2020 Spaceport America Cup Flight Software

#### v1.0.0 - 2019 Spaceport America Cup Flight Software
- includes custom flight computer software
- includes PC Tools interface for configuring, and downloading data from flight computer

## How To Run Flight Computer Software

The 'Avionics Software - Atollic' folder contains the main flight software project, created in Atollic TrueSTUDIO.
The project runs on the STM32F401RE Nucleo development board.

To run the project:

  1. Open Attolic.
  
  2. Import the project by doing the following:
      - Click File -> Import .
      - Then click 'Existing Projects into Workspace' under general.
      - Click 'Browse' and navigate to the 'Avionics Software - Atollic' then click 'OK'.
      - Make sure the 'Avionics Software' project is selected then click 'Finish'.

  3. Build the project by right clicking on the project in the Project Explorer on the left and clicking 'Build Project'.    
      
  4. Setup the debug configuration:
      -Right-click on the project in the Project explorer on the left.
      - Go to Debug As -> Debug Configurations...
      - Double-click 'Embedded C/C++  Application'
      - In the 'main' tab on the right section of the Debug Configurations window, click 'search project' and then select 'Avionics Software.elf'.
      
  5. Debug the project by clicking 'Debug'.      
      
# Operations

The software in the AvionicsSoftware-AtollicProject folder is meant to be run on the UMSTAS student designed flight computer.
Instructions for uploading the software to the flight computer can be found on google drive: 

UMSATS/Rocket/2-Avionics/Avionics Setup and Use Guide


UMSATS Flight Computer Prototype:

<img src="https://i.imgur.com/smPBZTm.jpg" width="500">


When powered on the flight computer will start a 1 hour countdown.  
The user LED will toggle every 2 seconds until the halfway point, when it will start to toggle every second. When 3/4 of the time has passed, te LED will toggle every 0.5 seconds.

This time can be changed in the configuration.h file.

**After the time has elapsed, the flash memory will be erased** and the flight computer will start recording data at a rate of 10/20 Hz (BMP/IMU). 
The data rate can be changed in the configuration file.
The flight computer will record data until the flash memory is full or power is removed.

To recover data from the flight computer, power it on while pressing the S2 button. This will start recovery mode.
In recovery mode, an inteface will be provided over UART, allowing the data to be read.

---
Information about UMSATS and our new rocketry division can be found at: http://www.umsats.ca/rocketry/
