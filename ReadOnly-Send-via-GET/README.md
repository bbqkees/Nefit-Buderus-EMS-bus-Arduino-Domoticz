
This folder contains two Arduino sketches that read the bus, decode the data and send selected values to Domoticz via HTTP GET requests if the value has changed.

The main file EMSbusReceiveExample1.ino contains lots of comments to help you understand how it works. The file EMSBusReceiveMinimalExample.ino only contains the minimum of code to just read the bus. Only the main file is actively maintained at this moment.

The Arduino sketch uses a modified Arduino Serial library which you need to use instead of the normal Arduino Serial library.
It is included.<br>
The library that was modified is a pretty old one. However, it will work for most purposes with this old library.<br>
Please see the [README file](https://github.com/bbqkees/Nefit-Buderus-EMS-bus-Arduino-Domoticz/blob/master/ReadOnly-Send-via-GET/libraries/Nefitserial/README.md) of the library folder for all the details.

In the code of the main file ALL pieces are already there to send data to the EMS bus, I have not tested this.
The parts I did not need to acquire the bus data, I commented out or just ignored.
What is not in there is fetching the HTTP GET requests from Domoticz (so you can send commands from Domoticz to the Arduino).

If you intend to send data, you also need to change the thermostat type in the writeRegister() function!
Furthermore if you have another thermostat than the RC20 you need to add the specific commands in the regNefitCoding array to read and write that thermostat.

There might also be some left over redundant stuff in the code. You can ignore it or remove it.

You need to create a virtual hardware and virtual devices in Domoticz for the sketch to send to.
So if you want to see the burner percentage create a percentage sensor in Domoticz.
Write down the ID so you can use it in the sketch.

## Methods to read and write to the bus

Basically there are two ways to approach this. One is that you simply only passively listen to the bus and the other method involves lots of interaction with the bus master.

### Passive listening/monitoring
You can just 'snif' the bus traffic with the receive circuit. Because the UBA (=boiler) sends out datagrams 0x18 and 0x34 about every 10 seconds you can acquire most of the parameters that the UBA has to offer.<br>The sketch as it is now will do that automatically. Just change the IP settings and device ID's of your domoticz devices and it will send lots of useful data to your Domoticz setup.

### Interacting with the bus
The way the bus works is explained pretty well on the [EMS wiki](https://emswiki.thefischer.net/doku.php?id=wiki:ems:ems-telegramme#polling). However this is all in German, so I will give a short introduction here.<br>
The EMS bus is controlled by the bus master, which is the UBA. Every bus device has its own bus address.<br>
The bus master continuously polls known bus addresses.<br>
If you want to read or write a certain register of a bus device (f.i. the UBA or the thermostat), you need to wait until you are polled and then you have a short window to send your request. The other device will then respond to your request.<br>
There are several functions in the sketch that help you read and write registers.<br>However, depending on what you intend to accomplish you need to use the functions to implement it. The sketch does not magically set the temperate on its own.

