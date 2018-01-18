# Why do we need a different serial library?
The Arduino system is based around certain Atmel microcontrollers.
For ease of use f.i. libraries were created so you can design a high level sketch without the need to bother about highly specific hardware access to those microcontrollers.
To access the serial ports (UARTS) of the Atmel microcontroller you can use the Arduino Serial library.<br>
Although these libraries are mostly pretty complete, sometimes you need to do something more specific.<br>
In the case of the EMS bus we have to do something specific; detect and send a EMS BREAK signal.

## EMS BREAK condition
In some (older) serial systems and also in the case of the EMS bus a BREAK condition is used to signal the end of a data transmission.<br>
In general, a BREAK condition on a serial interface is a condition where a zero level ('0') is send to the bus for a duration longer than one character.
In the case of the EMS bus a BREAK condition is a 11 bit sequence of zero's.<br>
Although the Atmel microcontroller itself can detect a BREAK condition, this function is not available in the Arduino Serial library. Therefore this function needs to be added.
Furthermore there is also no function to generate such a BREAK condition. This also needs to be added.

## Which library is used as a template?
The early 2012 version of the Arduino Serial library. <br>
In the mean time the Arduino Serial Library has been heavily modified, but for most cases this replacement still works.

## What additions are made to the Arduino Serial library?
The functions writeEOF() to write the EMS BREAK condition and frameerror() to detect the EMS BREAK condition on the bus.<br>
Furthermore the write() function has been modified.

## Can we omit the use of the Nefitserial library?
To some extent yes. <br>
It is possible to simulate the BREAK condition by temporarily ending the serial port, change it to a digital output LOW and after a short delay start the serial port again.<br>
You can do this by performing the sequence endSerial(), DigitalWrite (0,1); delay(x), and then reinitiate the serial port again.

## Can we port your sketch to the ESP8266?
The ESP also has a detection method for the BREAK signal. See the ESP UART documentation section 'Error detection interrupt'.
