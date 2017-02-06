
This folder contains the Arduino sketch that reads the bus, decodes the data and send selected values to Domoticz via HTTP GET requests if the value has changed.

The Arduino sketch uses a modified Arduino Serial library. It is included.
The library that was modified is a pretty old one, I am planning to incorporate the changes into the latest Arduino Serial library.
However, it will work for most purposes with this old library.

In the code ALL pieces are already there to send data to the EMS bus, I have not tested this.
The parts I did not need to acquire the bus data, I commented out or just ignored.
What is not in there is fetching the HTTP GET requests from Domoticz (so you can send commands from Domoticz to the Arduino).

If you intend to send data, you also need to change the thermostat type in the writeRegister() function!

There might also be some left over redundant stuff in the code. You can ignore it or remove it.

You need to create a virtual hardware and virtual devices in Domoticz for the sketch to send to.
So if you want to see the burner percentage create a percentage sensor in Domoticz.
Write down the ID so you can use it in the sketch.

