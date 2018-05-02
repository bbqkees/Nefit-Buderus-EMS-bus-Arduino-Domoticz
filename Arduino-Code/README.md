
This folder contains Arduino sketches that f.i. read the bus, decode the data and send selected values to Domoticz via HTTP GET requests if the value has changed.

### File info
#### Main File EMSbusReceiveExample1.ino
**Needs:** *Arduino (Mega 2560 for debugging), Wiznet Ethernet shield and EMS receive (and transmit) circuit.*<br>
The main file EMSbusReceiveExample1.ino contains everything needed to send and retrieve data from the EMS bus. Lots of comments help you understand how it all works. 
Only the main file is actively maintained at this moment.

#### Sub file EMSBusReceiveMinimalExample.ino
**Needs:** *Arduino (Mega 2560 for debugging), Wiznet Ethernet shield and EMS receive (and transmit) circuit.*<br>
The file EMSBusReceiveMinimalExample.ino is a subset of the main file and only contains the minimum of code to just read the bus and send the data to Domoticz. 

#### EMSBusSimpleRegisterReadRequest.ino
**Needs:** *Arduino Mega 2560 and EMS receive and transmit circuit.*<br>
EMSBusSimpleRegisterReadRequest.ino does only one thing: it requests a specific frame type from an EMS bus device.
The result is printed via the debug Serial port.<br> You can use this to test sending data to the bus.<br>
It is a partial rewrite of the main file in order to make it easier to understand and modify.<br>
You do not need an Ethernet shield to use this sketch.

#### EMSBusDumpToSerial.ino
**Needs:** *Arduino Mega 2560 and EMS receive circuit.*<br>
EMSBusDumpToSerial.ino will print all datagrams it sees on the EMS bus on the bus to the Serial port.
If enabled it can also print all polling messages, but for most cases these polling messages can be ignored.

I have a few more example sketches on the way that f.i. can set specific registers of EMS bus devices.<br>So have a look here once in a while.
<br>
The Arduino sketches use a modified Arduino Serial library which you need to use instead of the normal Arduino Serial library.
It is included.<br>
The library that was modified is a pretty old one. However, it will work for most purposes with this old library.<br>
Please see the [README file](https://github.com/bbqkees/Nefit-Buderus-EMS-bus-Arduino-Domoticz/blob/master/Arduino-Code/libraries/Nefitserial/README.md) of the library folder for all the details.

In the code of the main file ALL pieces are already there to send data to the EMS bus, I have tested it and it all works but in my situation I only need the read parts.
The parts I did not need to acquire the bus data, I commented out or just ignored.
What is not in there is fetching the HTTP GET requests from Domoticz (so you can send commands from Domoticz to the Arduino).

If you intend to send data to the thermostat, you also need to change the thermostat type in the writeRegister() function!
Furthermore if you have another thermostat than the RC20 you need to add the specific commands in the regNefitCoding array to read and write that thermostat.

There might also be some left over redundant stuff in the code. You can ignore it or remove it.

## Getting the data into Domoticz

You need to create a virtual hardware and virtual devices in Domoticz for the sketch to send to.
(Hardware-> new Dummy hardware -> Create virtual Sensors).
After you added the Dummy hardware you can create virtual sensors.
If you want to f.i. see the burner percentage create a percentage sensor in Domoticz.
Write down the IDX number so you can use it in the sketch (Devices -> search for device -> note IDX in second column).

Copy the sketch and library to your disk (best method: click on the file, choose 'raw' and save).<br>
Add the nefitSerial library in the correct Arduino folder on your computer (While the Arduino IDE is closed).<br>
Next open the sketch in the Arduino IDE and change the device IDX numbers and the IP address etc in the sketch to the values of your setup.<br>
Choose the correct board type and COM port in the board manager and then upload the sketch to your Arduino.<br>
And for reading out the regular parameters you are done. Devices are updated every 30 seconds (or when they change).
If you use Serial(0) for communication with the EMS bus make sure you remove EMS circuit from the Arduino because Serial(0) is also used when you program the board.

If you want to change the temperature you need the correct thermostat and some additional wrapping code that can write the correct EMS register.

### Arduino compiling errors
Please do not open an issue for simple compiling errors which include `...somethingsomething...was not declared in this scope`.
These are errors meaning the resource or variable is not there.<br><br>
If you get a message during compiling like `'nefitSerial' was not declared in this scope.` then you did not correctly install the nefitSerial library.<br><br>
If you get a message `nefitSerial1 was not declared in this scope` you are compiling the sketch for an Arduino that does not have Serial1. So you need to use Serial instead. Keep in mind that you cannot debug the sketch when you only have 1 Serial port.<br><br>
The error message `Error compiling for board ...someboardsomeboard` means you are compiling the sketch for a non-ATmega based Arduino board. As mentioned in the documentation, the nefitSerial library is only compatible with ATmega based Arduino boards.<br><br>
You can program f.i. an ESP8266 with the Arduino IDE but is only possible because the ESP8266 includes its own serial libraries into the Arduino IDE.
<br><br>
One less commen error is `HardwareSerial0.cpp.o (symbol from plugin): In function Serial: (.text+0x0): multiple definition of __vector_25`. This error means another library wants to use the original Serial library.
See [issue 6.](https://github.com/bbqkees/Nefit-Buderus-EMS-bus-Arduino-Domoticz/issues/6)

### Other errors
If you are encountering errors when compiling, first Google around for similar errors.<br>
I don't mind helping you out if you are stuck but most compilation errors are easily resolved by one Google search.<br>
If you can't solve it and need my help, see the closed issues first before opening a new issue.

## Methods to read and write to the bus
Basically there are two ways to approach this. One is that you simply only passively listen to the bus and the other method involves lots of interaction with the bus master.

### Passive listening/monitoring
You can just 'sniff' the bus traffic with the receive circuit. Because the UBA (=boiler) sends out datagrams 0x18 and 0x34 about every 10 seconds you can acquire most of the parameters that the UBA has to offer.<br>The main sketch as it is now will do that automatically. Just change the IP settings and device Idx numbers of your domoticz devices and it will send lots of useful data to your Domoticz setup.

### Interacting with the bus
If you want to read or write device settings you need to write to the bus. So you also need to implement the transmit part of the schematic.<br>
The way the bus works is explained pretty well on the [EMS wiki](https://emswiki.thefischer.net/doku.php?id=wiki:ems:ems-telegramme#polling). However this is all in German, so I will give a short introduction here.<br>
The EMS bus is controlled by the bus master, which is the UBA. Every bus device has its own bus address.<br>
The bus master continuously polls known bus addresses.<br>
If you want to read or write a certain register of a bus device (f.i. the UBA or the thermostat), you need to wait until you are polled and then you have a short window to send your request. The other device will then respond to your request.<br>
The device we emulate is the Buderus service key, which has address 0x0B.<br> So you need to wait with your request until the bus master polls device ID 0x0B.<br>
There are several functions in the sketch that help you read and write registers.<br>However, depending on what you intend to accomplish you need to use the functions to implement it. The sketch does not magically set the temperate on its own.

