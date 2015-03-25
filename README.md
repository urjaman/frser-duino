frser-duino
===========

Alternative code to serprog-duino.
We (urjaman & GNUtoo) are in process of melding all the good bits of
serprog-duino into frser-duino and deprecating serprog-duino.

Available targets (same as with serprog-duino):  
ftdi, flash-ftdi:  
	For the Arduinos with an FTDI  
	compatible flashrom arguments: flashrom -p serprog:dev=/dev/ttyUSB0:2000000  
	Other boards using an hardware USB<->Serial converter might work too.  
u2, flash-u2:  
	For the Arduino with a 8u2 or a 16u2  
	compatible flashrom arguments: flashrom -p serprog:dev=/dev/ttyACM0:115200  

Traditional targets:  
make - builds  
make program - "flashes"  

These are effectively synonyms for u2 and flash-u2,
except that "make" doesnt clean out the previous build if it sees no changes.

