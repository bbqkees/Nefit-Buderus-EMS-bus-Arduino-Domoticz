# EMS bus info page

## EMS bus interface schematic
If you only intend to read out the bus, you just need to build the top part of the schematic (the RX part).
When you also want to write to the bus build the entire schematic.
![EMS bus interface schematic](http://www.mikrocontroller.net/attachment/95287/EMS_Interface.png)

Depending on which Arduino you use connect the RX pin of the Arduino serial port to the RX pin in the schematic.
TX goes to TX. Do not connect the 12V pin of the service jack.
It does not matter which EMS bus pin you connect to which pin, the bridge rectifier will make sure both orientations will work.

## EMS bus interface locations
The EMS bus is usually available at two locations; at the front or inside the boiler.

### Front service jack
On most boilers there is a 3,5m service jack at the front of the boiler.

![EMS service jack pinout](https://github.com/bbqkees/Nefit-Buderus-EMS-bus-Arduino-Domoticz/blob/master/Documentation/EMS-bus-jack-pinout.JPG?raw=true)

### EMS Thermostat clamps
Aside from the front service jack, you can also connect the 2 EMS bus wires to the thermostat clamps on the inside interface of the boiler. You can connect these in parallel to a EMS thermostat if needed. The EMS bus is a shared bus that allows for multiple devices on the same bus.

![EMS thermostat clamps](https://github.com/bbqkees/Nefit-Buderus-EMS-bus-Arduino-Domoticz/blob/master/Documentation/ems-bus-on-boiler.JPG)


