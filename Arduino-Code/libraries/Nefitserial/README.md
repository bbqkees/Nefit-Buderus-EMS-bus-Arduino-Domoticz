# Note
*This library is only compatible with Arduino boards that have an Atmel ATmega type microcontroller like the Uno and the Mega (2560).*<br>

## Why is only the ATmega type supported?
Each microcontroller architecture has its own set of Arduino libraries to interface with the hardware. So a Serial library for an ATmega based Arduino Uno is different from a Serial library for a SAMD21 based Arduino M0. This is also true for f.i. the ESP8266 and the ESP32.<br>
Although you can program all those boards with the Arduino IDE as well, they bring their own set of libraries with them. 
Because the NefitSerial library is based on the ATmega Arduino Serial library, it is therefore only compatible with the ATmega based boards.

## Why do we need a different serial library anyway?
The Arduino system is based around certain (Atmel) microcontrollers.
For ease of use these libraries were created so you can design a high level sketch without the need to bother about highly specific hardware access to those microcontrollers.
To access the serial ports (UART) of the Atmel microcontroller you can use the Arduino Serial library.<br>
Although these libraries are mostly pretty complete, sometimes you need to do something more specific.<br>
In the case of the EMS bus we have to do something really specific; detect and send a EMS BREAK signal.

## EMS BREAK condition
In some (older) serial systems and also in the case of the EMS bus a BREAK condition is used to signal the end of a data transmission.<br>
In general, a BREAK condition on a serial interface is a condition where a zero level ('0') is send to the bus for a duration longer than one character.
In the case of the EMS bus a BREAK condition is a 11 bit sequence of zero's.<br>
Although the Atmel microcontroller itself can detect a BREAK condition on the lowest level, a detection function is not available in the higher level Arduino Serial library. Therefore this function needs to be added.
Furthermore there is also no function to generate such a BREAK condition. This also needs to be added.

### Detecting the BREAK condition
If the Atmel microcontroller 'detects' a BREAK, it will interpret this as a frame error (FE) and set a certain bit (FEn) in a register (USART Control and Status Register A). This register for most Arduino AVR's is UCSR0A and the bit is FE0 (bit 4). If you use another serial port, it might be f.i. register UCSR1A. This register value must be read (and set) at the right moment.

### Generating a BREAK condition
To generate a BREAK condition, data reception is temporarily halted. Then parity is changed to to even and a break-character is sent (basically a 0 byte). After the break character settings are restored and reception is re-enabled.

## Which library is used as a template?
The mid 2012 version of the core Arduino Serial library. <br>
In the mean time the Arduino Serial Library has been heavily modified, but for most cases the NefitSerial library still works as a full drop-in replacement.

## What additions are made to the Arduino Serial library?
The functions writeEOF() to write the EMS BREAK condition and frameerror() to detect the EMS BREAK condition on the bus.<br>

## What other modifications were made?
The write() function has been modified. Also the store_char() function has been changed to include the frame error detection.<br>
(Basically everything containing 'error' was added).

## Can we omit the use of the Nefitserial library?
To some extent yes. <br>
It is possible to simulate the BREAK condition by temporarily ending the serial port, change it to a digital output LOW and after a short delay start the serial port again.<br>
You can do this by performing the sequence endSerial(), DigitalWrite (0,1); delay(x), and then reinitiate the serial port again.<br>
This leaves us with detecting the BREAK when it occurs. For this I have no easy workaround at the moment.

## Can we port your sketch/concept to the ESP?
This has been recently completed by 'Proddy' see below:

### ESP8266
See full working code here: https://github.com/proddy/EMS-ESP-Boiler

### ESP32
The ESP32 has a built-in method in the ESP-IDF UART API to send data with a BREAK condition. See [here](http://esp-idf.readthedocs.io/en/latest/api-reference/peripherals/uart.html#_CPPv227uart_write_bytes_with_break11uart_port_tPKc6size_ti).<br>
Furthermore it also has an interrupt for BREAK detection called 'UART_BRK_DET_INT'.
