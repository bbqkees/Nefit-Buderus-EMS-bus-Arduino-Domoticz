## Example log files from Domoticz

As soon as you have created the virtual sensors in Domoticz and have the Arduino send the data to it, Domoticz will log the data into its database.

Below a few examples:

### Boiler out and return water temperature

![EMS schematic on a breadboard](https://github.com/bbqkees/Nefit-Buderus-EMS-bus-Arduino-Domoticz/blob/master/Documentation/examples/nefit-in-out-temp1.JPG?raw=true)

### Circulation pump on/off

![EMS schematic on a breadboard](https://github.com/bbqkees/Nefit-Buderus-EMS-bus-Arduino-Domoticz/blob/master/Documentation/examples/cv-pump1.JPG?raw=true)

### System pressure (bar)

![EMS schematic on a breadboard](https://github.com/bbqkees/Nefit-Buderus-EMS-bus-Arduino-Domoticz/blob/master/Documentation/examples/cv-pressure1.JPG?raw=true)

### Burner power (%)

![EMS schematic on a breadboard](https://github.com/bbqkees/Nefit-Buderus-EMS-bus-Arduino-Domoticz/blob/master/Documentation/examples/cv-burner-power1.JPG?raw=true)

## Better log and plots
Domoticz only logs the data with 5 minute interval resolution for up to 7 days.<br>
Older data is compressed to one daily value, which is not so useful.<br>
Switch data is logged for up to 30 days.<br>
If you want a better (or even beautiful) dashboard overview and every detail logged, have a look at InfluxDB together with [Grafana](https://grafana.com/).<br>
Domoticz can send all data into InfluxDB which then can be used to create nice plots and dashboards.
See the topic [here on the Domoticz forum](https://www.domoticz.com/forum/viewtopic.php?t=15088) for more information.
