# Board design V0.9
<img src="https://raw.githubusercontent.com/bbqkees/Nefit-Buderus-EMS-bus-Arduino-Domoticz/master/PCB-files/V0.9/nefit-ems-bus-interface-PCB-top.jpg" width="500">

## Board functionality
The board has the full send and receive circuit to interface between the EMS bus and a 3.3V or 5V interface logic microcontroller.<BR>The current and all previous versions have been succesfully running on many boilers for the past year.<BR>
In the end it is basically just a level shifter between the EMS and TTL designed to be used with the Arduino and the ESP8266 etc.<BR>
It can be plugged in directly to the EMS service jack on the boiler or parallel to the thermostat.<BR>
It is powered via 3.3V or 5V from the microcontroller board.<BR>
Some limited protection is provided by two polyfuses to protect the EMS bus from your experimental f*ck ups.<BR>

## Board design considerations
+ The board is double sided but all components are only on the top side.<BR>
+ Components are all SMD except for the connectors those are through-hole.<BR>
In theory the board could have been smaller and f.i. populated on both sides. However it was designed from a more practical point of view. It has larger SMD components and some additional room around them so the board is hand solderable for even not so skilled persons.<BR> 
Furthermore the board has all components on one side so I would only need one PCB solder paste stencil if I wanted to use one. Those things are relatively expensive compared to the price of the PCB's themselves. And you can't really do two-sided reflow soldering at home anyway.<BR>
+ All connectors are through hole, these are usually much stronger than SMD connectors which can easily be ripped off the board if f.i. you pull too hard on an attached wire.<br>
+ The bottom side of the board has a ground plane for reduced noise. <br>
+ The board has several pin holes you can use for mounting. They are not attached to anything.<BR>
They are in the normal 2.54mm grid spacing so f.i. you can use a few header pins to directly solder the EMS board onto an Arduino prototyping board so it will fit neatly and sturdy on top of an Arduino. 
+ Board size and connector positions were chosen such that it all fits on top of an Arduino prototyping board.

## Remarks
The last version is V0.9.<BR>
The only difference between V0.8 and V0.9 boards is the silkscreen. There were a few changes to the text on the board but the schematic and layout is identical. 

### PCB version history
V0.9: Only difference with V0.8 is the silkscreen.<BR>
V0.8: Changed the voltage selector to a resistor selector. R15 is no longer populated, changed resistor value R14 to 100 Ohm.<BR>
V0.7: Removed wire bridge from V0.6 and added a ground plane. Layout is more compact. Moved around some components. Added vias for mounting. Changed EMS 12V pin header. Added two polyfuses.<BR>
V0.6: First production version. This version has a wire bridge and no ground plane.<BR>

## Top side
![Top view](https://raw.githubusercontent.com/bbqkees/Nefit-Buderus-EMS-bus-Arduino-Domoticz/master/Documentation/nefit-ems-bus-interface-PCB.jpg)

## Bottom side
![Bottom view](https://raw.githubusercontent.com/bbqkees/Nefit-Buderus-EMS-bus-Arduino-Domoticz/master/PCB-files/V0.9/nefit-ems-bus-interface-PCB-bottom.jpg)

## Instructions for use
You can find the instruction for use as added to the shipment of the fully populated boards [here](https://github.com/bbqkees/Nefit-Buderus-EMS-bus-Arduino-Domoticz/blob/master/PCB-files/V0.9/EMS%20bus%20interface%20board%20manual%20V0.9.pdf).

## Components
I have almost all components in a nice [Reichelt shopping list](https://www.reichelt.de/my/1489718).
### Instructions before ordering:
- Double check the schematic and the component list below when you load the shopping cart.<br>
- Sometimes components sell out or have a long delivery time. You need to replace those components yourself with alternatives that are equal. Usually Reichelt has enough alternatives. Its possible to fit a 0603 on the 0805 footprint. For passives (resistors and capacitors) if equal components are not availble, you can try the next lower or higher value. For the capacitors on the EMS bus side you want ones that can handle 16V or higher.<BR>
- At this time the 10uF 0805 capacitor is not available at Reichelt at all. You could f.i. fit a slightly larger size on the board. <BR> 
- For the full board you need 8 header pins. In the shopping list there is a single row of 40 pins so if that one sells out just get a strip with at least 10 pins as usually you'll break off a few which you cannot use anymore.<BR>  
- What Reichelt does not have are the polyfuses. If you buy the bare PCB from me I'll add those two to the shipment.<br>
  I use the Multicomp MC36207. I got them from [Sinuss.nl](https://sinuss.nl/componenten/passieve-componenten/thermistors/pptc-resettable-fuse/1861187-mc36207-fuse-ptc-reset-30v-200ma-smd-multicomp) (high shipping costs!).<br>
  If you want to omit the polyfuses just solder a piece of wire across the footprint of F1 and F2 or solder a 0805 zero Ohm resistor.
- The screw terminal (J1) is a 5.08mm 2-pin screw terminal I got from Ebay [(Like these)](https://www.ebay.nl/itm/40x-2Pin-Screw-Terminal-Block-Connector-5-08mm-Pitch-Panel-PCB-Mount-Repalce-Kit/292743000390). Those are very cheap. If you buy a bare PCB from me I also add one.
- Do not replace the jack terminal with another type, as its footprint is very specific and others will either not fit or they have the wrong pinout.<br>
- L1 and L2 are for SMD inductors. If you want to use axial through-hole inductors instead you need to use L3 and L4. do not use both L1/L2 and L3/L4.

## Component list
Please see above for the Reichelt shopping list.

Index | Type | Size/package | Value | Number/Code | Remark
---|---|---|---|---|---
J1|Screw terminal| 5.08mm |||
J2|4 pin header|2.54mm|||
J3|3.5m jack plug stereo|TRS1|EBSF35|EBSF35|Use [this one](https://www.reichelt.de/klinkeneinbaubuchse-3-5-mm-stereo-ebsf-35-p153203.html) from Reichelt. (Note: I once found the same connectors on Aliexpress).
J4|3 pin header with one jumper|2.54mm|||
J5|2 pin header|2.54mm|||
J6 J7 J8 J9 J10 J11 J12|1 pin header||||Not populated. Use for mounting.
F1 F2|polyfuse|0805|200mA cont. 400mA trip. 30V|Multicomp MC36207|If not used replace with wire bridge or 0805 zero Ohm resistor.
L1 L2|inductor/choke|0805|4.7uH||Use pair L1/L2 for SMD *OR* pair L3/L4 for through hole.
L3 L4|inductor/choke|12.7mm axial|~4.7uH-1mH||Use L3 L4 for through hole *OR* L1 L2 for SMD. The inductor value is not so strict.
U2|IC|SOIC-8|LM393D|LM393D|You can also use the LM393.
Q1|transistor NPN|SOT-23|BC847B|BC847B|
D3 D4|schottky diode bridge|SOT-23|BAT54S|BAT54S|
D5|schottky diode|SOD-123|BAT46W|BAT46W|
D6 D7/Dx/BAT46|diode|SOD-80 (=mini-melf)|1N4148|1N4148|Some boards have the wrong silkscreen 'BAT46' here.
R2|resistor|0805|10K||
R3|resistor|0805|360E||
R4|resistor|0805|47K||
R5|resistor|0805|100K||
R6 R7 R8|resistor|0805|4K7||
R9 R10 R11 R12|resistor|0805|910E||
R13|resistor|0805|10K||
R14|resistor|0805|100E||
R15|resistor|0805|--|--|Not populated. See alternative setup below the table.
C1|capacitor|0805|68pF 16V||
C2|capacitor|0805|1.5nF 16V||
C3|capacitor|0805|10nF 16V||
C4|capacitor|0805|10uF 16V||
C5|capacitor|0805|1nF 16V||
C6|capacitor|0805|100nF 16V||

**Alternative setup** if you want to power the board with 5V but have a 3.3V compatible UART: Replace R14 with 20K and R15 with 10K.
And use the lower jumper setting.

## Schematic V0.9
![Schematic](https://raw.githubusercontent.com/bbqkees/Nefit-Buderus-EMS-bus-Arduino-Domoticz/master/PCB-files/V0.9/EMS-V09_schema.png)
(Click on the image above for a larger version)

## Gerber files
Not available.
 
