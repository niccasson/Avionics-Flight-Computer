# Avionics
Flight Computer Software for the 2019 Rocket.

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
      


Information about UMSATS and our new rocketry division can be found at: http://www.umsats.ca/rocketry/
