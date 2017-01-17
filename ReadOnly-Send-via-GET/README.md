
This folder contains the Arduino sketch that reads the bus (no write), decodes the data and send selected values to Domoticz via HTTP GET requests if the value has changed.

The Arduino sketch uses a modified Arduino Serial library. It is included.
The library that was modified is a pretty old one, I am planning to incorporate the changes into the latest Serial library.
However, it will work for most purposes with this old library.

In the code most pieces are already there to send data to the EMS bus, I have not tested this.
