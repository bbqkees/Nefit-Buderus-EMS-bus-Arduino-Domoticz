# EMS bus info page

## EMS bus interface schematic
If you only intend to read out the bus, you just need to build the top part of the schematic (the RX part), and the capacitor from the bottom part.
When you also want to write to the bus build the entire schematic.
![EMS bus interface schematic](http://www.mikrocontroller.net/attachment/95287/EMS_Interface.png)

Depending on which Arduino you use connect the RX pin of the Arduino serial port to the RX_out pin in the schematic.
TX goes to TX_in. Do not connect the 12V pin of the service jack.
It does not matter which EMS bus pin you connect to which pin, the bridge rectifier will make sure both orientations will work.

The 2 'N/A' components on the left are 2 poly fuses. Include them or not, your choice.
The 4 parallel resistors in the transmitter part can be replaced by a single 1W ~100 Ohm resistor. 
One improvement to the circuit would be using optocouplers on the right side where you interface them with your logic.
Without those in theory a voltage burst on the bus could destroy your Arduino.

(This schematic is from the website http://wiki.neo-soft.org/index.php/Heizungsschnittstelle/ServiceKey, you can also find additional info on the EMS bus there).

### Powering your circuit from the bus itself
I got a few questions if you could power the circuit and even the whole Arduino from the EMS bus or the 12V pin.
I have not tested this, but it should be possible to some extent.
Nefit sells a WiFi service adapter that only plugs into the front service jack, so likely you can also power some stuff from this jack.
I have no idea how much current you can draw from it, test it yourself if you need it.

## EMS bus interface locations
The EMS bus is usually available at two locations; at the front and/or inside the boiler.

### Front service jack
On most boilers there is a 3,5mm service jack at the front of the boiler.

![EMS service jack pinout](https://github.com/bbqkees/Nefit-Buderus-EMS-bus-Arduino-Domoticz/blob/master/Documentation/EMS-bus-jack-pinout.JPG?raw=true)

### EMS Thermostat clamps
Aside from the front service jack, you can also connect the 2 EMS bus wires to the thermostat clamps on the inside interface of the boiler. You can connect these in parallel to a EMS thermostat if needed. The EMS bus is a shared bus that allows for multiple devices on the same bus. If needed, you can also put it in parallel where the thermostat is mounted on the wall. 

![EMS thermostat clamps](https://github.com/bbqkees/Nefit-Buderus-EMS-bus-Arduino-Domoticz/blob/master/Documentation/ems-bus-on-boiler.JPG)

## EMS bus datagram details
The bus datagram details can be found at het German website https://emswiki.thefischer.net/doku.php.
For your reference there are two PDF's generated from that website with all the datagram details contained in this folder.
[HERE](https://github.com/bbqkees/Nefit-Buderus-EMS-bus-Arduino-Domoticz/blob/master/Documentation/telegrammaufbau.pdf) and [HERE](https://github.com/bbqkees/Nefit-Buderus-EMS-bus-Arduino-Domoticz/blob/master/Documentation/telegramme.pdf).


