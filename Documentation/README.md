# EMS bus info page

## Complete interface board
I created a complete interface board with 5V and 3.3V compatible UART interface. For the design see the [PCB-files](https://github.com/bbqkees/Nefit-Buderus-EMS-bus-Arduino-Domoticz/tree/master/PCB-files/V0.9) folder.
You can order a complete and tested board or Gateway [HERE](https://bbqkees-electronics.nl/).<br>
![EMS bus PCB](https://github.com/bbqkees/Nefit-Buderus-EMS-bus-Arduino-Domoticz/blob/master/Documentation/nefit-ems-bus-interface-PCB.jpg)

## EMS bus interface locations
The EMS bus is usually available at two locations; at the front and/or inside the boiler.

### Front service jack
On most boilers there is a 3,5mm service jack at the front of the boiler.

![EMS service jack pinout](https://github.com/bbqkees/Nefit-Buderus-EMS-bus-Arduino-Domoticz/blob/master/Documentation/EMS-bus-jack-pinout.JPG?raw=true)

### EMS Thermostat clamps
Aside from the front service jack, you can also connect the 2 EMS bus wires to the thermostat clamps on the inside interface of the boiler. You can connect these in parallel to a EMS thermostat on the same terminal if needed. The EMS bus is a shared bus that allows for multiple devices on the same bus. If needed, you can also put it in parallel where the thermostat is mounted on the wall. 

![EMS thermostat clamps](https://github.com/bbqkees/Nefit-Buderus-EMS-bus-Arduino-Domoticz/blob/master/Documentation/ems-bus-on-boiler.JPG)

You can also use the on/off terminal for an on/off thermostat while simultaneously using the EMS bus terminal for the interface board. So if you have an on/off thermostat you can still communicate with the EMS bus. In fact I use this very setup at home.


## Powering your circuit from the jack and the bus itself

### Powering from the 12V service jack
I got a few questions whether you could power the circuit and even the whole Arduino from the EMS bus or the 12V pin.<br> This pin is often not really 12V but in my case f.i. around 8V.<br>
I have successfully drawn a nice 400mA at about 7,5V DC with a resistive test load between the EMS- and 12V pin on my own boiler (with no other devices connected to the bus).<br>
Nefit sells a WiFi service adapter that only plugs into the front service jack, so aside from a thermostat you can also power some stuff from this service jack.
I have no idea how much current you may safely draw from it, test it yourself if you need it.
Likely also the amount of EMS devices on the bus will affect the available power.<br>
The combination of the level shifter circuit, an Arduino Mega 2560 and the ethernet shield draws about 210mA at 5V.<br>
So in theory you could power the entire combination from the front service jack. However, your mileage may vary and of course any testing you do is at your own risk.<br>
If you want to do this anyway, first connect the entire combination to the EMS- and EMS+ pins. Do not connect an external power supply or USB cable to the Arduino.<br>
Then measure the voltage between the 12V pin and GND of the Arduino.<br> If this voltage is positive, above 7V and does not exceed 12V it should be safe for powering the Arduino.<br> The maximum input voltage for most Arduino's is 20V, but at this level the voltage converter on the Arduino will get too hot very quickly.<br>
Now connect a diode that will take at least 250mA and ideally a 300mA fuse in series with this 12V pin and connect this to Vin of the Arduino.<br>Vin is located on the power header of the Arduino, next to GND.
Internally, Vin of the Arduino is directly connected to the input of both the 5V and 3,3V voltage regulator on the board. The diode you need to insert is for reverse voltage protection. The fuse is to protect the EMS bus from current overdraw.<br>
Likely you will see a voltage drop on Vin. If the voltage gets below 7V, you cannot reliably power the Arduino from the bus.<br>
If your thermostat turns off or it does not work well anymore, there is a possibility that too much current is drawn. Also in this case you cannot power the Arduino from the bus.<br> If you do not get any real problems at this point, at least make sure the 5V voltage regulator of the Arduino does not overheat.<BR>
---DO NOT connect an external power supply to the Arduino if you have connected the 12V pin to Vin!---<br>
A USB cable is safe because the Arduino has a internal switchover to Vin if both Vin and USB are connected.<br>
Furthermore keep in mind that when you are sending data to the bus, you are pulling the bus lower through the 4 parallel 910 Ohm resistors. This can cause an additional current draw up to 70mA.
  
### Powering from the EMS bus itself
The EMS bus has a very limited power supply but aside from the thermostat you can power something additional. But how much depends on what is already on the bus. You will also get problems on transmitting to the bus when you draw the incorrect amount of current. Not recommended. 

#### Isolate your power supply circuit from the bus 
What is important to note is that both data and power uses the same bus lines. So you need to separate your power supply circuit from the bus lines so it cannot interfere with the transmission of data.
There are several ways to do this. Use f.i. a suitable diode in series between EMS+ and your power supply circuit.

## EMS bus protocol
Although there are some variations, a typical EMS bus datagram looks like this:

Byte 1 | Byte 2 | Byte 3 | Byte 4 | Byte 5 .. n-2 | Byte n-1 | Byte n
--- | --- | --- | --- | --- | --- | ---
Sender | Receiver | Frametype | Offset | Data bytes | CRC | BREAK

<br>
The sequence starts with the sender ID (byte 1) and the intended receiver ID (byte 2). Byte 3 contains the frametype. The frametype is the identification of the type of message that is transmitted. A frametype basically represents a table. Byte 4 contains the offset in bytes in the particular frametype. So it is the index of the item in the table. Most of the time you receive the entire table anyway so this byte might not have the same meaning in every datagram. Byte 5 and following contain the data. At the end of the message follows a CRC byte.<br> 
A datagram is terminated by a BREAK.<br>
What the code does is listen on the bus for databytes until this BREAK appears. After the break it knows it has received a complete datagram. With all the datagram bytes in its buffer it will check if the first 3 bytes of the datagram match a known entry in the decoding list. If so, it will decode the datagram so you can use the value to send it to Domoticz.


## EMS bus datagram details
The bus datagram details and a large list of all frametypes can be found at het German website https://emswiki.thefischer.net/doku.php.
For your reference there are two PDF's generated from that website with all the datagram details contained in this folder.
[HERE](https://github.com/bbqkees/Nefit-Buderus-EMS-bus-Arduino-Domoticz/blob/master/Documentation/telegrammaufbau.pdf) and [HERE](https://github.com/bbqkees/Nefit-Buderus-EMS-bus-Arduino-Domoticz/blob/master/Documentation/telegramme.pdf).

### Datagrams that are available without writing to the bus
The boiler (UBA) will periodically send out datagrams 0x18 UBAMonitorFast and 0x34 UBAMonitorWWMessage.<br>
0x18 concerns status updates of the central heating part, and 0x34 updates of the tap water part.

## Be careful what parameters you write
A number of the registers of the boiler are also writeable. However, this also means that you can send a wrong value to it. <br>For instance what if you would change the boiler flow temperature (Vorlauftemperatur/aanvoertemperatuur) from 45 to 90? Now if water of this temperature would reach your floor heating, you'll shurely damage it.<br>

If you mess up completely and the boiler locks up, you need to do a factory reset of the boiler.<br>Usually this is achieved by pressing a certain combination of buttons on the boiler. Check the installation manual of your boiler.<br> It is a good idea to make a note of all boiler settings before you start sending commands to the bus, so you'll know the right default settings just in case. 

# Further documentation on this Github
Every folder on this Github has its own README file with lots of additional information.<br>
[Infopage on the Arduino sketches](https://github.com/bbqkees/Nefit-Buderus-EMS-bus-Arduino-Domoticz/tree/master/Arduino-Code)<br>
[Infopage on the nefitSerial Library](https://github.com/bbqkees/Nefit-Buderus-EMS-bus-Arduino-Domoticz/tree/master/Arduino-Code/libraries/Nefitserial)<br>
[Example log files in Domoticz](https://github.com/bbqkees/Nefit-Buderus-EMS-bus-Arduino-Domoticz/tree/master/Documentation/examples)<br>
[Specific info on EMS thermostats](https://github.com/bbqkees/Nefit-Buderus-EMS-bus-Arduino-Domoticz/blob/master/Documentation/Thermostats)<br>


