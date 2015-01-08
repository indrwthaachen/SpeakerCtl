SPEAKERCTL
========

SpeakerCtl can be used to control NuPro(R) Speakers by IR and optical feedback.
It allows to set the volume from within Matlab or via a tray applet.
Also it can detect when the speaker hits the maximum output volume limit during playback.


PCBs
-------------------------
The file nuproRemote.brd/sch contains the main PCB , which is in Eagle format.
lcdInterface.brd/sch is a separate circuit board that is needed for I2C communication with the LCD.
PartsList.txt contains a list of all components that are necessary in order to assemble the PCBs.

uC Code
-------------------------------
The uC code can be compiled and run on Teensy 3.1 microcontroller boards.


Enclosure 
------------
The enclosure is made up of three parts:
* top.stl
* middle.stl
* bottom.stl
which can be easily 3D printed on any consumer-grade 3D Printer.
After printing these parts must be stacked/glued together. 
 

TrayTask 
------------
A Python script that connects via Bluetooth with multiple SPEAKERCTL boards and allows to control them.
It provides a TCP/IP interface for external programs but also contains a simple GUI for direct control by the user. 


Matlab 
------------
Matlab code that communicates over TCP/IP with the TrayTask process.

