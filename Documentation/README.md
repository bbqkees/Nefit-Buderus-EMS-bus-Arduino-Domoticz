# EMS bus info page

## EMS bus interface schematic
If you only intend to read out the bus, you just need to build the top part of the schematic (the RX part), and the 100nF capacitor from the bottom part.
When you also want to write to the bus build the entire schematic.
![EMS bus interface schematic](http://www.mikrocontroller.net/attachment/95287/EMS_Interface.png)

(The schematic is originally from the website http://wiki.neo-soft.org and it is the reverse engineered schematic of a Buderus service key).

Depending on which Arduino you use connect the RX pin of the Arduino serial port to the RX_OUT pin in the schematic.
TX goes to TX_IN. Do not connect the 12V pin of the service jack.
It does not matter which EMS bus pin you connect to which pin, the bridge rectifier will make sure both orientations will work.

The 100nF (=0,1uF) capacitor is a bypass capacitor to ensure a stable voltage for the LM393 (See the LM393 datasheet). 

The 2 'N/A' components on the left are 2 poly fuses. Include them or not, your choice.
The 4 parallel resistors in the transmitter part can be replaced by a single 1W ~100 Ohm resistor. 

There are more components that are not that strict, I used f.i. a LM339 instead of the LM393.

Furthermore the 4,7mH inductors are pretty huge, likely 4,7uH is the correct value.

One improvement to the circuit would be using optocouplers on the right side where you interface them with your logic.
Without those in theory a voltage burst on the bus could destroy your Arduino or vice versa.

The line 'U_REF' is an internal voltage for the comparators, just connect every 'U_REF' line together and you are done.

Here is the Rx part of the schematic on a small breadboard (working example, no layout available):

![EMS schematic on a breadboard](https://github.com/bbqkees/Nefit-Buderus-EMS-bus-Arduino-Domoticz/blob/master/Documentation/ems-breadboard.JPG?raw=true)

### Using this circuit for interfacing with a Raspberry Pi
You can use this circuit also to directly interface with the Raspberry Pi UART. However, you need to make a small modification.<br>
The Arduino has 5V compatible UARTS, the Raspberry Pi has 3,3V compatible UARTS.<br>
Replace the 4k7 resistor on the right (next to RX_OUT) by a voltage divider consisting of one 20k resistor and one 10k resistor in series. Put the 20k resistor where the 4k7 resistor is, and in series to that one connect the 10k resistor to ground. Now connect the RX_OUT to the point between the 20k and the 10k resistor. Keep in mind the circuit still needs a 5V power supply.

### Powering your circuit from the bus itself
I got a few questions whether you could power the circuit and even the whole Arduino from the EMS bus or the 12V pin.
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

## EMS bus protocol
A typical EMS bus datagram looks like this:<br>
 | Byte 1 |  Byte 2  |  Byte 3   | Byte 4 | Byte 5 .. n-2 | Byte n-1 | Byte n |<br>
 | Sender | Receiver | Frametype | Offset |  Data bytes   |   CRC    | BREAK  |<br>
<br>
The sequence starts with the sender ID (byte 1) and the intended receiver ID (byte 2). Byte 3 contains the frametype. The frametype is the identification of the type of message that is transmitted. A frametype basically represents a table. Byte 4 contains the offset in bytes in the particular frametype. So it is the index of the item in the table. Byte 5 and following contain the data. At the end of the message follows a CRC byte.<br>
A datagram is terminated by a BREAK.<br>
What the code does is listen on the bus for databytes until this BREAK appears. After the break it knows it has received a complete datagram. With all the datagram bytes in its buffer it will check if the first 3 bytes of the datagram match a known entry in the decoding list. If so, it will decode the datagram so you can use the value to send it to Domoticz.


## EMS bus datagram details
The bus datagram details and a large list of all frametypes can be found at het German website https://emswiki.thefischer.net/doku.php.
For your reference there are two PDF's generated from that website with all the datagram details contained in this folder.
[HERE](https://github.com/bbqkees/Nefit-Buderus-EMS-bus-Arduino-Domoticz/blob/master/Documentation/telegrammaufbau.pdf) and [HERE](https://github.com/bbqkees/Nefit-Buderus-EMS-bus-Arduino-Domoticz/blob/master/Documentation/telegramme.pdf).


