# Nefit-Buderus-EMS-bus-Arduino-Domoticz

Readout of Nefit/Buderus EMS (=Energy Management System) interface Protocol by Arduino and transfer of data via HTTP GET requests to Domoticz home automation software (or to other systems).

## Goal:
Readout the Nefit/Buderus EMS interface of a Nefit Trendline gas boiler (gas condensing central heating boiler).

Transfer of data via ethernet to Domoticz home automation software.

## What does it do?
This sketch reads the EMS interface data and depending on the format decodes the data and puts it in variables.
You can then send the values via HTTP GET requests to Domoticz or do whatever you want with it.
Usage is not limited to Domoticz, you can extract the decoding part for other purposes.

## Boiler support
Reading the EMS bus:
Should support all boilers using the EMS databus.
This includes most Bosch boiler brands like Nefit, Buderus, Worcester.
Datagrams are periodically sent out by the boiler with source ID 0x08.
You only need to listen in. No data requests are needed for most boilers.

Writing to the EMS bus:
To change the temperature and other settings you need to write to the thermostat on the bus.
Depending on the thermostat on your wall you need to send specific commands.
Most of these are already reverse-engineered. Some are not.
Below the list of thermostats which should work fine.

## Thermostat support
EMS bus thermostats:RC20 (source ID 0x17), likely also RC30 and RC35 (both ID 0x10).
Depending on under which brand name these thermostats are sold they might have a different type name.
If your thermostat does not work, and you really want to change the temperature you might want to buy a supported model.

## Hardware:
* EMS interface circuit
* Arduino Mega + Wiznet Ethernet Shield
* Raspberry Pi with Domoticz

The EMS bus interface can be converted to TTL level by means of a simple circuit.
The TTL-converted signal can then be connected to one of the the Arduino UARTs.
['See the Documentation folder'](https://github.com/bbqkees/Nefit-Buderus-EMS-bus-Arduino-Domoticz/tree/master/Documentation).


This sketch uses the Arduino Mega and the Wiznet 5100 ethernet shield.
You can also use another Arduino like the Uno but that one only has one hardware serial port.

Serial1 is used for the EMS module.
Serial is used to debug the output to PC. 
EMS serial works with 9700 Baudrate and 8N1.
For sending data to the EMS bus you need a modified Serial library. It's included in the project.

Arduino Mega:
* Serial  on pins  0 (RX)  and 1 (TX),
* Serial1 on pins 19 (RX) and 18 (TX),
* Serial2 on pins 17 (RX) and 16 (TX),
* Serial3 on pins 15 (RX) and 14 (TX). 

Arduino non-Mega:
You cannot use Serial1, so you need to use Serial, which does not allow for debugging via serial.

## Where did you find all the information about the bus?
http://www.mikrocontroller.net/topic/309075
http://www.mikrocontroller.net/topic/141831
https://emswiki.thefischer.net/doku.php?id=start

### How about MQTT instead of HTTP GET requests?
That is on my list.
For now it works exactly like my other Github project ['Vbus-Arduino-Domoticz'](https://github.com/bbqkees/vbus-arduino-domoticz).

### How about serial output instead of HTTP GET requests?
If you use a Mega, this is very easy to accomplish.
Just check f.i. ['this website'](https://wiki.sgripon.net/doku.php/add_temperature_and_humidity_sensor_dht11_in_domoticz).
It will not work easily with an Uno, because you would need to use the software serial for reading the bus, this would also need modification to its library. But a Mega is only a few Euro/Dollar on Ebay or Aliexpress anyway.

#### Additional credits
Sketch is based on the EMS sketches from 'Jvdmeer' from the Nodo forum.

#### Legal Notices
'Worcester, Bosch Group', 'Buderus' and 'Nefit' are brands of Bosch Thermotechnology.

All other trademarks are the property of their respective owners.
