# Board design V0.9

## Remarks
The last version is V0.9.<BR>
The only difference between V0.8 and V0.9 boards is the silkscreen. There were a few changes to the text on the board but the schematic and layout is identical. 

### Version history
V0.9: Only difference with V0.8 is the silkscreen.<BR>
V0.8: R15 is no longer populated, changed resistor value R14 to 100 Ohm.<BR>
V0.7: Removed wire bridge from V0.6 and added a ground plane. Layout is more compact. Moved around some components. Added vias for mounting. Changed EMS 12V pin header.<BR>
V0.6: First production version. This version has an wire bridge.<BR>

## Top side
![Top view](https://raw.githubusercontent.com/bbqkees/Nefit-Buderus-EMS-bus-Arduino-Domoticz/master/Documentation/nefit-ems-bus-interface-PCB.jpg)

## Bottom side
![Bottom view](https://raw.githubusercontent.com/bbqkees/Nefit-Buderus-EMS-bus-Arduino-Domoticz/master/PCB-files/V0.9/nefit-ems-bus-interface-PCB-bottom.jpg)

## Instrunctions for use
[Instructions for use](https://github.com/bbqkees/Nefit-Buderus-EMS-bus-Arduino-Domoticz/blob/master/PCB-files/V0.9/EMS%20bus%20interface%20board%20manual%20V0.9.pdf) as added to the shipment of the fully populated boards.

## Schematic
The schematic will be here shortly.

## Board design
The board is double sided but all components are only on the top side.
The bottom side of the board has a ground shield. The earliest version of the board did not have it. There is now a huge reduction in noise.<br>
The board has 5 unnamed holes you can use for mounting. They are not attached to anything.<BR>

## Components
I have almost all components in a nice [Reichelt shopping list](https://www.reichelt.de/my/1489718).
### Instructions:
- Sometimes components sell out or have a long delivery time. You need to replace those components yourself with alternatives that are equal. Usually Reichelt has enough alternatives. Its possible to fit a 0603 on the 0805 footprint. For passives (resistors and capacitors) if equal components are not availble, you can try the next lower or higher value. For the capacitors on the EMS bus side you want ones that can handle 16V or higher.<BR>
- At this time the 10uF 0805 capacitor is not available at Reichelt at all. You can fit a slightly larger size on the board. <BR> 
- For the full board you need 8 header pins. In the shopping list there is a single row of 40 pins so if that one sells out just get a strip with at least 10 pins as usually you'll break off a few.<BR>  
- What Reichelt does not have are the polyfuses. If you buy the bare PCB from me I'll add those two to the shipment.<br>
  I use the Multicomp MC36207. I got them from [Sinuss.nl](https://sinuss.nl/componenten/passieve-componenten/thermistors/pptc-resettable-fuse/1861187-mc36207-fuse-ptc-reset-30v-200ma-smd-multicomp) (high shipping costs!).<br>
  If you want to omit the polyfuses just solder a piece of wire across the footprint of F1 and F2 or solder a 0805 zero Ohm resistor.
- The screw terminal (J1) is a 5.08mm 2-pin screw terminal I got from Ebay [https://www.ebay.nl/itm/40x-2Pin-Screw-Terminal-Block-Connector-5-08mm-Pitch-Panel-PCB-Mount-Repalce-Kit/292743000390). Those are very cheap. If you buy a bare PCB from me I also add one.
- Do not replace the jack terminal with another type, as its footprint is very specific and others will either not fit or they have the wrong pinout.<br>
- L1 and L2 are for SMD inductors. If you want to use through-hole inductors instead you need to use L3 and L4 instead.  
  


 
