frser-duino
===========

Alternative code to serprog-duino.  
We're in process of melding all the good bits of
serprog-duino into frser-duino and deprecating serprog-duino.

Available targets (same as with serprog-duino):

* ftdi, flash-ftdi:  
	For the Arduinos with an FTDI  
	compatible flashrom arguments: flashrom -p serprog:dev=/dev/ttyUSB0:2000000  
	Other boards using an hardware USB<->Serial converter might work too.

* u2, flash-u2:  
	For the Arduino with a 8u2 or a 16u2  
	compatible flashrom arguments: flashrom -p serprog:dev=/dev/ttyACM0:115200  

Traditional targets:  
make - builds  
make program - "flashes"  

These are by default effectively synonyms for u2 and flash-u2,
except that "make" doesnt clean out the previous build if it sees no changes.
You can change what the traditional targets do by either
1. changing the defines at the beginning of the makefile, or
2. using environment variables, for example: `BLBAUD=115200 SERIAL_DEV=/dev/ttyUSB1 make program`


Note: the repository has a submodule, clone with --recursive.
