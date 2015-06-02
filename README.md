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


### About the various (broken) usb-serial converters

- Arduino Uno with ATMega 8U2/16U2: their default firmware is quite broken,  
	but the default settings works around it at 115200 (slowly).

	If you're willing to test, I have an alternative firmware for it:
	https://github.com/urjaman/fast-usbserial
	with that firmware you should be able to use it as if it was an FTDI,
	meaning with make ftdi and at 2Mbaud. This means that for this case
	it would be valid to say `make ftdi; make flash-u2`. The flash targets
	dont check what kind of binary they're flashing.
	(Note: My benchmarks say it is still slower than the FTDI.)


- A "VISduino" named Uno R3 clone with with a CH340G:  
	It is a cheap usb-serial converter chip, and if it
	in general works on your computer (google had reports of it not
	working with all usb chipsets) you can treat it like an FTDI
	with a maximum baudrate of 115200. Claims 2Mbaud but apparently
	doesnt have big enough buffers for operation at that speed.
	Commands would be (if you dont touch the defaults in Makefile):  
	`DFLAGS=-DFTDI make clean all` and `SERIAL_DEV=/dev/ttyUSB0 make program`
