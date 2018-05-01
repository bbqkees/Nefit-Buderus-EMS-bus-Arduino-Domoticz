/*
/* EMS bus decoder/encoder to/from Domoticz sketch for Arduino with Ethernet module
 * * Version 1.03 Github - January 2018
 * 
 * Copyright (c) 2017 - 'bbqkees' @ www.domoticz.com/forum
 * What now follows is the MIT license, this means you can do whatever you want with the code.
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software
 * and associated documentation files (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge, publish, distribute,
 * sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED
 * TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 * 
 * Legal Notices
 * 'Worcester', 'Bosch Group', 'Buderus' and 'Nefit' are brands of Bosch Thermotechnology.
 * All other trademarks are the property of their respective owners.

 * last edit  :  08-JAN-2018
 * 
 * 08-JAN-2018:  Added some more detailed explanations.
 * 03-JAN-2018:  Added explanation about decoding of the array values.
 * 29-NOV-2017:  Added alert sensor request function in preparation for status code notification to Domoticz
 *		 Also corrected the text sensor request function and added some more comments. 
 * 17-JAN-2017:  Github version 1.0
 * 16-NOV-2016:  Data only sent if value has changed.
 *               #if DEBUG added to print debug data over Serial0 port. 
 * 15-NOV-2016:  Initial version
 * 
 * What does it do?
 * This sketch reads the EMS interface data and depending on the format decodes the data and puts it in variables.
 * You can then send the values via HTTP GET requests to Domoticz or do whatever you want with it.
 * Usage is of course not limited to Domoticz, you can extract the decoding part for other purposes.
 * 
 * Most boilers send out datagrams every few seconds, these will be read automatically by this sketch.
 * You cannot set any setpoint directly via the boiler, you need to send this data to the thermostat.
 * 
 * Please not ALL code is in here to send data to the EMS but those are either ignored or commented out.
 * I have not tested the send functions, I did not need it. If you do, uncomment the relevant parts.
 * 
 * As I am Dutch, some variables might be in partial Dutch instead of English.
 * F.i. 'CV' is 'Centrale Verwarming', which means 'Central Heating'. So if you see 'cv' think 'ch'.
 * Same for 'WW" which is 'Warm Water' which translates to 'hot tap water' in English.
 * However I translated most of the comments to English.
 * 
 * If you have question about this sketch and its methods, please ask me on the Domoticz forum.
 */
 
 
#define NEFIT_REG_MAX	17
 
// Set to "1" when debugging 
// If enabled, the Arduino sends the decoded values over the Serial port.
#define DEBUG 1
 
#include <NefitSerial.h>
#include <avr/pgmspace.h>
#include <SPI.h>
#include <stdint.h>

// SPI and Ethernet are used for Ethernet shield
#include <Ethernet.h>

// MAC and IP of the Arduino
byte mac[] = {
  0xDE, 0xAD, 0xBE, 0xEF, 0x00, 0x01}; // Set this to your preferred MAC address
IPAddress ip(192,168,1,110); // Set this to your preferred IP.

// IP address and port of Domoticz
IPAddress domoticz(192,168,1,100); // Set this to the IP address of your Domoticz
int port = 8080; // This is the default port of Domoticz.
EthernetClient client;

/*
* The following table is used in converting the register to and from the Nefit bus messages.
* It also supplies the encoding it is in.
*
* It is placed in flash instead of RAM. Hence the 'PROGMEM' part.
* These values will never change and therefore you can store it in flash.
* This saves up some space in RAM.
* Storing it in flash means you need to use pgm_read_byte_near() to retrieve the values.
* 
* Format: <transmitter ID>,<frametype>,<data-offset>,<vartype>     // data-offset start at byte 4
* vartype bit 3-0 	= 0 boolean (bitnr in bit 6-4)
*			= 1 byte
*			= 2 ascii
*			= 3 int
*			= 4 byte x 2 (For reading value divide by two)
*			= 5 int x 10 (for reading value divide by ten)
*     = 6 byte x 10 (for reading value divide by ten)
*
* Transmitter ID 0x08 is the listening address for the EMS bus.
* The boiler will provide this data.
* Transmitter ID 0x17 is from the RC20 type thermostat. 
* You will only see this data if you have this specific thermostat.
* Otherwise you need to check http://emswiki.thefischer.net/ for your thermostat.
* 
* You can only change the setpoint etc if you have connected the thermostat to the bus.
*
* Decoding examples:
* 0x08,0x18,0x04,0x01 ->
* Transmitter 0x08 = UBA (a.k.a. the boiler itself)
* Frametype 0x18 = type UBAMonitorFast (See https://emswiki.thefischer.net/doku.php?id=wiki:ems:telegramme)
* Data-offset 0x04 = 4 + start offset of 5 = 9 -> burner power (%).
* vartype 0x01 = 0b0000 0001 -> 1 byte.
*
* 0x08,0x34,0x01,0x05 ->
* Transmitter 0x08 = UBA
* Frametype 0x34 = type UBAMonitorWWMessage 
* Data-offset 0x01 = 1 + start offset of 5 = 6 -> tap water temperature x10.
* vartype 0x05 = int x 10.
*
* 0x08,0x34,0x05,0x50 ->
* Transmitter 0x08 = UBA
* Frametype 0x34 = type UBAMonitorWWMessage 
* Data-offset 0x05 = 5 + start offset of 5 = 10.
* vartype 0x50 = 0b0101 0000 -> bit 3-0 -> 0b000 = 0 = type boolean. bit 6-4 -> 0b101 = start bit 5 -> WW OK.
* 
* Vartypes 0x80 and above are used in determining if the value is writeable in the 'writeRegister' function.
* 0x81 = 0b1000 0001 -> writeable value of type byte.
*
* Note for transmitter ID 0x17 with frametypes 0x91 and 0xA8:
* These are reverse engineered values found by observing the bus. They are specific for the RC20.
* If you want to set values for the RC35 instead check the EMS wiki for it's values.
*
*
* You can expand the array 'regNefitCoding' below by adding lines to it with your own set of values.
* Do not forget to increase 'NEFIT_REG_MAX' accordingly.
*
* Keep in mind that the regNefitCoding array is nicely formatted here, in memory it is just a concatenation like
* 0x08,0x18,0x01,0x05,0x08,0x18,0x04,0x01,0x08,0x18,0x07,0x00, etc.
* Each line is a sequence of 4 bytes.
* in order to address a certain 'line', you need to start looking at every n+4 byte. So 0, 4, 8....
*  
*/

PROGMEM const unsigned char regNefitCoding[]={

// 0x18 UBAMonitorFast (boiler status high frequency report)
0x08,0x18,0x01,0x05,					//#1   0 temperature water out (uitgaand) x 10
0x08,0x18,0x04,0x01,					//#2   1 burner power
0x08,0x18,0x07,0x00,					//#3   2 burner of/off
0x08,0x18,0x07,0x50,					//#4   3 heating pump on/off
0x08,0x18,0x07,0x60,					//#5   4 tap water heating on/off
0x08,0x18,0x0B,0x05,					//#6   5 boiler temperature x 10
0x08,0x18,0x0D,0x05,					//#7   6 temperature water in (cv return) x 10
0x08,0x18,0x11,0x06,					//#8   7 water pressure x 10
0x08,0x18,0x12,0x02,					//#9   8 status code 1st letter
0x08,0x18,0x13,0x02,					//#10  9 status code 2nd letter
// 0x34 UBAMonitorWWMessage (hot water status report)	
0x08,0x34,0x01,0x05,					//#11 10 tap water temperature x 10
0x08,0x34,0x05,0x50,					//#12 11 heater boiler on/off (not always available)
// 0x91 and 0xA8 are specific to the RC20 and are setting regarding the thermostat.
// If you have f.i. a RC35 you need to set other messages here.	
0x17,0x91,0x01,0x04,					//#13 12 setpoint room temperature x 2
0x17,0x91,0x02,0x05,					//#14 13 room temperature x 10
0x17,0xA8,0x17,0x81,					//#15 14 setting 0=low, 1=manual, 2=clock
0x17,0xA8,0x1C,0x84,					//#16 15 overruled clock setting x 2 ( 0 = not overruled)
0x17,0xA8,0x1D,0x84					//#17 16 manual setpoint temperature x 2
};

void printbuffer(char * buffer, int len );

char buffer[32];                		// frame buffer
int nefitRegister[NEFIT_REG_MAX];	        // index number see above
unsigned long register_changed =0;              // flag if registy changed
byte pollAdres = 0;                             // if not 0, last polling address
byte nefitRegToVar[NEFIT_REG_MAX];          	// Write to which variable
int regToWrite =0 ;                             // Split execution event handling (normally not needed)
int regValue;                                   // Same as above
char xmitBuffer[7] = {0x0B,0x17,0xA8,0x17,0x00,0x00,0x00};            // load some defaults

boolean EtherComm = false;
long startTimeBurner =0;
long burnerTime =0;
long startTimeWW =0;
long WWTime =0;

long lastTime = 0;  
long interval = 10000;
long lastTimeeth = 0;  
long intervaleth = 30000;

// Set here your own Domoticz Idx numbers
int IDXcvpressure = 000; 
int IDXburnerpower = 000;
int IDXburner = 000;
int IDXboiler = 000;
int IDXcvreturn = 000;
int IDXsupply = 000;
int IDXhotwater = 000;
int IDXcvpump = 000;
// End of user Idx numbers

float lastcvpressure = 0;
float lastburnerpower = 0;
float lastburner = 0;
float lastboiler = 0;
float lastcvreturn = 0;
float lastsupply = 0;
float lasthotwater = 0;
float lastcvpump = 0;

String burnerstatus = "Off";
String cvpumpstatus = "Off";
String hotwaterstatus = "Off";



// Calculate CRC for buffer
uint8_t nefit_ems_crc( char * buffer, int len ){
  uint8_t i,crc = 0x0;
  uint8_t d;
  for(i=0;i<len-2;i++){
    d = 0;
    if ( crc & 0x80 ){
      crc ^= 12;
      d = 1;
    }
    crc  = (crc << 1)&0xfe;
    crc |= d;
    crc ^= buffer[i];
  }
  return crc;
}

// CRC check 
boolean crcCheckOK(char * buffer, int len ){
  int crc = nefit_ems_crc(buffer, len );
  boolean crcOK= (crc==(uint8_t)buffer[len-2]);
  return crcOK;
}

// Read one bus frame, return number of read bytes
int readBytes(char * buffer){
  int ptr = 0;

//  nefitIO.flush();
  while(nefitSerial1.available()){
    if((uint8_t)nefitSerial1.peek()==0){
      uint8_t temp = nefitSerial1.read();                    // skip zero's
    } else { break;}
  }

  while((!nefitSerial1.frameError())&&ptr<32){          // read data until frame-error
     if(nefitSerial1.available()){
      buffer[ptr]=nefitSerial1.read();
      ptr++;
     }
  }
  nefitSerial1.flush();
  return ptr;
}

void sendAck(){                                         // send polling acknowledge, our address is xmitBuffer[0]
  delay(1);
  sendBuffer(xmitBuffer,2);                             // send own address and a break
}

void sendRequest(char *xmitbuffer){
  xmitBuffer[5]= nefit_ems_crc(xmitBuffer,7);           //set crc value
  pollAdres = 0;
  while ((pollAdres&0x7F)!=0x0B){                       // wait for interface polling
    int ptr = readBytes(buffer);
    if (ptr==2){ pollAdres=buffer[0];}
  }
  delay(2);                                             // delay 2ms (found by trial and error)
  sendBuffer(xmitBuffer, 7);                            // always send 7 bytes + break.
}

// Send a data frame to the bus in EMS format
void sendBuffer(char * xmitBuffer, int len){
  char j;
  for(j=0;j<len-1;j++){                                   
    nefitSerial1.write(xmitBuffer[j]);
    delay(3);
  }
  nefitSerial1.writeEOF();
  delay(2);
  nefitSerial1.flush();
}

// Read a register value from frame-buffer, decide if necessary and return as int
/*
* Detailed explanation getValue
* Important to recall here is that 'vartype' is one byte of which bit 0 to 3 represent the variable type.
* Furthermore bit 4 to 6 represent the bit index number in case the vartype is a boolean.
* Writeable vartypes have byte 7 set to 1.
*
* As the input of this function you use the buffer array, the position of the relevant databyte (offset) and its vartype.
*
* --------
* Step 1 - Do vartype AND 0x0F.
*	In a binary AND function, the result is true only if both inputs are true.
*	Otherwise the result is false.
* 	0x0F is 0b0000 1111. So bit 0 to 3 are 1, all others are 0.
*	This means that if you perform and AND with 0x0F on any given byte, only bit 0-3 will ever be important.
*	All other bits will always be 0 in the result.
*	0x05 is 0b0000 0101. 0x05 AND 0x0F is 0b0000 0101, which is of course 5.
*	0x60 is 0b0110 0000. 0x60 AND 0x0F is 0b0000 0000, which is 0.
*	0x84 0s 0b1000 0100. 0x84 AND 0x0F is 0b0000 0100, which is 4.
*	The result of the AND function is used as input for the following switch case.
* --------
* Step 2 - Switch case
*	There are 3 possible functions here. One for switch case 0, one for switch cases 1,2,4,6 and one for cases 3,5.
*	-> Case 0: The var type is a boolean.
*		The function bitRead is executed with parameters the offset and the result of '(vartype&0x70)>>4'.
*		BitRead is an Arduino function that returns the value of a certain bit in a byte.
*		'(vartype&0x70)>>4' -> Do vartype AND 0x70, and after that shift all resulting bits 4 positions to the right.
*		Now, AND with 0x70 is a measure to cut off all irrelevant bits, just like in the AND function in step 1.
*		The result is shifted 4 bits to the right.
*		0x50 & 0x70 is still 0x50. 0x50 is 0b0101 0000. 0b0101 000 >>4 = 0b0000 0101 = 5.
*		So you read bit 5 of the data byte. 		
*	-> Case 1,2,4 or 6: the resulting value is just the byte value of the single data byte.
*	-> Case 3 or 5: Byteshift first databyte to the left, and then add the second databyte.
*		The part '<<8' means that you shift every bit 8 positions to the left.
*		So 0b1010 1010 will get to 0b1010 1010 0000 0000.
*		Now if you add a second byte 0b1111 1111 the result will be 0b1010 1010 1111 1111.
*		This is done to properly add up a MSB and a LSB byte to represent f.i. an integer.
* --------
*/ 
int getValue(char * buffer, byte offset, byte vartype){
  int result;
  uint8_t type = vartype & 0x0F;
	switch (type){
		case 0:			//boolean
			result = bitRead(buffer[offset],(vartype&0x70)>>4);
			break;
		case 1:			//byte
//			result = (uint8_t)buffer[offset];
//			break;
		case 2:			//ascii
//			result = (uint8_t)buffer[offset];
//                        break;
		case 4:			//byte x 2
//			result = (uint8_t)buffer[offset];
//			break;
		case 6:			//byte x 10
			result = (uint8_t)buffer[offset];
                        break;
		case 3:			//int
//			result = getDoubleByte(buffer,offset);
//			break;
		case 5:			//int x 10
		    result = buffer[offset]<<8;
			result = result + (uint8_t)buffer[offset+1];
			break;

	}
    return result;
}

// Decode frame with help of the table and load register
/*
* Explanation nefitFrame2register
* This function loops over every 'line' in regNefitCoding and checks if there is a match.
* If there is a match, it decodes the data bytes and puts the resulting value in the corresponding nefitRegister address.
*
* As the input if this function you use the buffer array and the buffer size (len).
*
* Detailed explanation: 
*
* Recall the typical EMS bus datagram:
* | Byte 1 |  Byte 2  |  Byte 3   | Byte 4 | Byte 5 .. n-2 | Byte n-1 | Byte n |
* | sender | Receiver | Frametype | Offset |  Data bytes   |   CRC    | BREAK  |
*
* So the buffer array contains the following relevant information:
* buffer[0]: Sender
* buffer[1]: Receiver (not used here)
* buffer[2]: Frametype
* buffer[3]: Offset
* buffer[4]: First databyte
* buffer[5]: Next databyte
* .....
*
* --------
* Step 1 - for loop:
*	This loop will check every line in regNefitCoding.
* 	Remember, a line in regNefitCoding is a sequence of 4 bytes. So to address the next line,
*	the counter needs to be incremented by 4 each time (i=i+4).
*	So line 1 starts at byte nr 0, line 2 at byte nr 4, line 3 at byte nr 8 etc. 
*	The loop will run from 0 until the end of the regNefitCoding array. which is NEFIT_REG_MAX*4. 
* --------
* Step 2 - Read the first 2 bytes from a regNefitCoding line and put those in 2 variables.
* --------
* Step 3 - Check if there is an approximate match of the data with the line.
*	If so, continue and read the 3rd byte of the line. If not, do next loop iteration.
*	the IF statement checks if the Sender (buffer[0]) in the datagram matches the sender of the current line from refNefitCoding.
*	It also checks if the Frametype (buffer[2]) in the datagram matches the frametype in the current line. 
*	(You should have noticed that buffer[1] is skipped, because it has no relevance here.)
*	This 3rd step might seem not needed because step 4 does almost the same,
*	but apparently reading from flash takes some additional time so we want to prevent it as much as needed.
* --------
* Step 4 - If previous step was ok, now check is there is an exact match.
*	If so, recalculate the offset. If not, do next loop iteration.
*	The offset is recalculated so that it is compensated for the datagram header bytes (4 of them).
*	the IF statement checks if the Sender (buffer[0]) in the datagram exactly matches the sender of the current line from refNefitCoding.
*	It also checks if the Frametype (buffer[2]) in the datagram exactly matches the frametype of the current line.
*	Furthermore it checks if the Offset (buffer[3]) matches the offset of the current line.
* --------
* Step 5 - check if the recalculated offset is smaller than the amount of databytes in the datagram.
*	If so, read the variable type of the current line of regNefitCoding.
*	Also, execute the function getValue with the parameters buffer,offset,vartype.
*	(See the explanation there for more details).
* --------
* Step 6 - check if the newly calculated data is different from the current value of the corresponding item in nefitRegister. 
*	If it is different, write the new value.
* --------
*/
void nefitFrame2register(char * buffer, int len){
  byte sender, message;
  int register_data, difference;
  uint8_t offset, vartype;
  for(uint8_t i=0;i<((NEFIT_REG_MAX)*4);i=i+4){
    sender = pgm_read_byte_near(regNefitCoding+i);
    message = pgm_read_byte_near(regNefitCoding+1+i);
    if ((sender<=(uint8_t)buffer[0]) && (message<=(uint8_t)buffer[2])){
      offset = pgm_read_byte_near(regNefitCoding+2+i);
      if ((sender==(uint8_t)buffer[0]) && (message==(uint8_t)buffer[2])&&(offset>=(uint8_t)buffer[3])){
        offset = offset - (uint8_t)buffer[3]+4;       // rekening houden met lange frames (gesplitst)
        if (offset<len){
           vartype = pgm_read_byte_near(regNefitCoding+3+i);
           register_data = getValue(buffer,offset,vartype);
           if (register_data != nefitRegister[i/4])
             {
                nefitRegister[i/4] = register_data;
				if (nefitRegToVar[i/4]>0) {
				   bitSet(register_changed,i/4);     // alleen wanneer dit een Nodo var is
				}
             }
        }
      }
    }else {break;}
  }
 return;
}

// Send query command to the bus to read register values
float queryBus(byte regnr){
  float result = 0xFFFFFFFF;
  int reg_ptr = regnr*4;
  int ptr;
  xmitBuffer[1] = pgm_read_byte_near(regNefitCoding+reg_ptr) | 0x80;    // Load destination with read command
  xmitBuffer[2] = pgm_read_byte_near(regNefitCoding+(reg_ptr+1));       // message type
  xmitBuffer[3] = pgm_read_byte_near(regNefitCoding+(reg_ptr+2));       // offset in destination registers
  xmitBuffer[4] = 20;                                                   // !! and always max num bytes so 20
  sendRequest(xmitBuffer);
  long timeout = (long)millis() + 300;
  while ((((long)millis()-timeout)<0) && (!nefitSerial1.available())){}        // Wait for answer
  if (nefitSerial1.available()) {
    ptr = readBytes(buffer);
    if (ptr>4) {
      if (crcCheckOK(buffer,ptr)){
        nefitFrame2register(buffer, ptr);
//        printReg();
	if (nefitRegToVar[regnr]>0) {
	   bitSet(register_changed,regnr); 
        }
	result = nefitRegister[regnr];
      }
    }
  }
  return result;
}

// Read register and convert to float
float nefitRegister2float(char regnr){
  float result;
  if (regnr>=0)
  {
    uint8_t format = (pgm_read_byte_near(regNefitCoding+3+(regnr*4)))&0x0F;
    switch (format){
      case 0:                                                   // read type 0, same as 1
      // result = nefitRegister[regnr]; break;
      case 1:                                                   // read type 1, same as 2
      // result = nefitRegister[regnr]; break;
      case 2:
      // result = nefitRegister[regnr]; break;                   // read type 2, same as 3
      case 3:  result = nefitRegister[regnr]; break;
      case 4:  result = nefitRegister[regnr]/2; break;
      case 5:
      //result = nefitRegister[regnr]/10; break;                 // read type 5, same as 6
      case 6:  result = (float)nefitRegister[regnr]/10.0; break;
    }
  } else { result = -1;}
  return result;
}

int writeRegister(byte regnr, int val){
  int result = 1;                                // timeout error preset
  int reg_ptr = regnr*4;
  int ptr;
 
  if (regnr<=NEFIT_REG_MAX){
    if ((pgm_read_byte_near(regNefitCoding+(reg_ptr+3))&0x80)==0x80){   // test if writing is permissible
      xmitBuffer[1] = 0x17;
      xmitBuffer[2] = 0xA8;
      xmitBuffer[3] = pgm_read_byte_near(regNefitCoding+(reg_ptr+2));   // offset in destination registers
      uint8_t type = pgm_read_byte_near(regNefitCoding+(reg_ptr+3))& 0x0F;
      if ((type==1) && (val<3)) {                                       // extra check if value <3 (for thermostate setting)
        xmitBuffer[4] = (byte)val;
      }else if (type==4) {
        xmitBuffer[4] = (byte)val*2;
      }
      sendRequest(xmitBuffer);
      long timeout = (long)millis() + 800;
      ptr = 0;
      while ((((long)millis()-timeout)<0)&&(ptr==0)){     // wait for answer
        if (nefitSerial1.available()) {
          ptr = readBytes(buffer);
          if ((ptr>0)&&(buffer[0]==1)) {	//If the response is 0x01, the setting was succesfully written.
            nefitRegister[regnr] = xmitBuffer[4];
            result = 0;
            break;
          } else {
           result = 2;                                               // error code received on bus
          }
        }
     }
    }else {
      result = 3;                                                    // not writeable
    }
  } else {
    result = 4;                                                      // register number
  }
  return result;
}

//The part below contain all the HTTP request functions.


// Domoticz general value JSON URL. Prints 2 decimals
// /json.htm?type=command&param=udevice&idx=IDX&nvalue=0&svalue=TEMP
// /json.htm?type=command&param=udevice&idx=IDX&nvalue=0&svalue=BAR
// /json.htm?type=command&param=udevice&idx=IDX&nvalue=0&svalue=PERCENTAGE
void httpRequestvalue(int IDX, float value) {
  // if there's a successful connection:
  if (client.connect(domoticz, port)) {
    client.print( "GET /json.htm?type=command&param=udevice&idx=");
    client.print(IDX);
    client.print("&nvalue=0&svalue=");
    client.print(value,1);
    client.println( " HTTP/1.1");
    client.print( "Host: ");
    client.println(ip);
    client.println( "Connection: close");
    client.println();

    client.println();
    client.stop();
    delay(150);
  } 
  else {
    client.stop();
  }
}

//  Domoticz switch
// /json.htm?type=command&param=switchlight&idx=XX&switchcmd=On
void httpRequestswitch(int IDX, String status) {
  // if there's a successful connection:
  if (client.connect(domoticz, port)) {
    client.print( "GET /json.htm?type=command&param=switchlight&idx=");
    client.print(IDX);
    client.print("&switchcmd=");
    client.print(status);
    client.println( " HTTP/1.1");
    client.print( "Host: ");
    client.println(ip);
    client.println( "Connection: close");
    client.println();

    client.println();
    client.stop();
    delay(150);
  } 
  else {
    client.stop();
  }
}

//  Domoticz text sensor
//  /json.htm?type=command&param=udevice&idx=IDX&nvalue=0&svalue=TEXT
void httpRequestText(int IDX, String text) {
  // if there's a successful connection:
  if (client.connect(domoticz, port)) {
    client.print( "GET /json.htm?type=command&param=udevice&idx=");
    client.print(IDX);
    client.print("&nvalue=0&svalue=");
    client.print(text);
    client.println( " HTTP/1.1");
    client.print( "Host: ");
    client.println(ip);
    client.println( "Connection: close");
    client.println();

    client.println();
    client.stop();
    delay(150);
  } 
  else {
    client.stop();
  }
}

//  Domoticz Alert sensor (Same as text sensor, but with additional alert level)
//  /json.htm?type=command&param=udevice&idx=IDX&nvalue=LEVEL&svalue=TEXT
//  Level = (0=gray, 1=green, 2=yellow, 3=orange, 4=red)
void httpRequestTextAlert(int IDX, int level, String text) {
  // if there's a successful connection:
  if (client.connect(domoticz, port)) {
    client.print( "GET /json.htm?type=command&param=udevice&idx=");
    client.print(IDX);
    client.print("&nvalue=");
    client.print(level);
    client.print("&svalue=");
    client.print(text);
    client.println( " HTTP/1.1");
    client.print( "Host: ");
    client.println(ip);
    client.println( "Connection: close");
    client.println();

    client.println();
    client.stop();
    delay(150);
  } 
  else {
    client.stop();
  }
}




// -----------------------------------------------------------------------------------------------------------------

void setup(){
  nefitSerial1.begin(9700);
  
#if DEBUG  
  nefitSerial.begin(9600);
  nefitSerial.println("Nefit to Domoticz");
#endif  

  Ethernet.begin(mac, ip);

#if DEBUG
  nefitSerial.print("IP address: ");
  nefitSerial.println(ip);
#endif  

  for (char i=0;i<=NEFIT_REG_MAX;i++) {
    nefitRegister[i]=0;
    nefitRegToVar[i]=0;
  }
  // We migth already be intered in some register values, set a trigger
  nefitRegToVar[2]  = 1;    // burner on/off
  nefitRegToVar[4]  = 2;    // wwvalve on/off

}


// -----------------------------------------------------------------------------------------------------------------

void loop(){
  int ptr;
  //if (pollAdres == 0) {queryBus(14);}     // Force reading of thermostat setting at start up

 // if (regToWrite > 0){
 //   uint8_t res=writeRegister(regToWrite,regValue);
 //   regToWrite = 0;
 //   } else 
 if (nefitSerial1.available()){  // check if we need to read
	ptr = readBytes(buffer);
	if (ptr == 2) {
          pollAdres = buffer[0];
//          if (pollAdres==0x8B){                                       // we are being polled,send acknowledge
//            sendAck();                                                // this causes to be polled more often
//          }
	}else if (ptr>4){
	  if (crcCheckOK(buffer,ptr)){
	    nefitFrame2register(buffer,ptr);
	    if (register_changed>0) {
              if (bool burner = bitRead(register_changed,2)){          // if burner-state changed
                if (burner) {                                          // burner turns on
                  startTimeBurner = millis();
                } else {                                                // burner turns off
                  burnerTime = burnerTime + millis() - startTimeBurner;
                }
              }else if (bool wwvalve = bitRead(register_changed,4))      // wwvalve setting changed
                if (wwvalve) {
                  startTimeWW = millis();
                } else {                                                // wwvalve off
                  WWTime = WWTime + millis() - startTimeWW;
                }                  
              register_changed = 0;                                     // clear change flags
	    }
	  }
	}
    }

#if DEBUG

    if(millis() - lastTime > interval) {
    lastTime = millis(); 
   for (int i = 0; i<NEFIT_REG_MAX; i++){
   nefitSerial.print("nefitRegister["); nefitSerial.print(i); nefitSerial.print("]: ");
   nefitSerial.println(nefitRegister[i]);}
  nefitSerial.println("------------------------");
  nefitSerial.println("NEFIT EMS bus:");
  
  nefitSerial.print("cv water uitgaand: ");
  nefitSerial.println((float)nefitRegister[0]/10,1);      

  nefitSerial.print("burner vermogen: ");
  nefitSerial.println((float)nefitRegister[1],1);      

  nefitSerial.print("burner on/off: ");
  nefitSerial.println((float)nefitRegister[2],DEC);   // burner  

  nefitSerial.print("cv pump on/off: ");
  nefitSerial.println((float)nefitRegister[3],DEC);   //cv pump  
 
  nefitSerial.print("hot watervraag on/off: ");
  nefitSerial.println((float)nefitRegister[4],DEC);    

  nefitSerial.print("keteltemp: ");
  nefitSerial.println((float)nefitRegister[5]/10,1);    //keteltemp
  
  nefitSerial.print("cv return temp: ");
  nefitSerial.println((float)nefitRegister[6]/10,1);    //waterterug temp
  
  nefitSerial.print("waterpressure: ");
  nefitSerial.println((float)nefitRegister[7]/10,1);    // waterpressure

  nefitSerial.print("statuscode: ");
  nefitSerial.print((char)nefitRegister[8]);    
  nefitSerial.println((char)nefitRegister[9]);  
  
  nefitSerial.print("hotwater temp: ");
  nefitSerial.println((float)nefitRegister[10]/10,1);   //hotwatertemp

  nefitSerial.print("boiler verhoting on/off: ");
  nefitSerial.println((float)nefitRegister[11],DEC);  

  //nefitSerial.print(",");
  //nefitSerial.print(nefitRegister[14],DEC);           //thermostat 0=low, 1=manual, 2=auto

  nefitSerial.println("END");
  nefitSerial.println("------------------------");
  nefitSerial.println("------------------------");

#endif

    
    }


  // loop for GET requests
  
  
  if(millis() - lastTimeeth > intervaleth) {
    lastTimeeth = millis(); 
    
    float cvpressure = float(nefitRegister[7])/10;
    if(cvpressure >=0 && cvpressure != lastcvpressure){
    httpRequestvalue(IDXcvpressure , cvpressure );
    }
    lastcvpressure = cvpressure;

    float burnerpower = nefitRegister[1];
    if(burnerpower >=0 && burnerpower != lastburnerpower){
    httpRequestvalue(IDXburnerpower , burnerpower );
    }
    lastburnerpower = burnerpower;

    float supply = float(nefitRegister[0])/10;
    if(supply >0 && supply !=lastsupply){
    httpRequestvalue(IDXsupply , supply );
    }    
    lastsupply = supply;

    float cvreturn = float(nefitRegister[6])/10;
    if(cvreturn >0 && cvreturn != lastcvreturn){
    httpRequestvalue(IDXcvreturn , cvreturn );
    } 
    lastcvreturn = cvreturn;

    float burner = nefitRegister[2];
    if (burner == 1){burnerstatus = "On";}
    else if (burner == 0){burnerstatus = "Off";}
    else {burnerstatus = "Off";}
    if (burner != lastburner){
    httpRequestswitch(IDXburner, burnerstatus);
    }
    lastburner = burner;

    float cvpump = nefitRegister[3];
    if (cvpump == 1){cvpumpstatus = "On";}
    else if (cvpump == 0){cvpumpstatus = "Off";}
    else {cvpumpstatus = "Off";}
    if (cvpump != lastcvpump) {
    httpRequestswitch(IDXcvpump, cvpumpstatus);
    }
    lastcvpump = cvpump;

    float hotwater = nefitRegister[4];
    if (hotwater ==1) {hotwaterstatus = "On";}
    else if (hotwater == 0) {hotwaterstatus = "Off";}
    else {hotwaterstatus = "Off";}
    if (hotwater != lasthotwater){
    httpRequestswitch(IDXhotwater, hotwaterstatus);
    }
    lasthotwater = hotwater;
    
  }//end GET requests
    

    
}//end void loop
