/* Simple EMS register request sketch for Arduino Mega.
 * * Version 1.0 Github - April 2018
 * 
 * Copyright (c) 2018 - 'bbqkees' @ www.domoticz.com/forum
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
 * last edit  :  30-APR-2018
 * 
 * 30-APR-2018:  Initial version
 * 
 * What does it do?
 * This sketch requests a specific datagram from an EMS bus device every 30 seconds.
 * If it has been correctly received, it is printed via the serial console.
 * 
 * Hardware:
 * -Arduino Mega 2560.
 * -EMS interface circuit on Serial1.
 * -PC via USB for debug on Serial(0).
 * 
 * Usage:
 * The core function is registerRequest(byte sender, byte receiver, byte frameType, byte frameSize).
 * It needs the sender, receiver, EMS frame type and frame size as input.
 * See the void loop for some examples.
 * You can get the frame size of a frame type from the EMS wiki at:
 * https://emswiki.thefischer.net/doku.php?id=wiki:ems:telegramme
 * 
 * f.i.: registerRequest(INTERFACE_ID, UBA_ID, 0x18, 32)
 * This machine with INTERFACE_ID (0x0B) requests UBAMonitorFast (0x18) with length 32 from the boiler with UBA_ID (0x8). 
 * This of course equivalent to registerRequest(0x0B, 0x08, 0x18, 32).
 * 
 * Sometimes you do not get a response when the frame length is incorrect. Check the EMS wiki for the values.
 * 
 * However, on some boilers the length is different.
 * F.i. on my boiler 0x18 is 32 bytes instead of 36 and 0x19 is 24 bytes instead of 28.
 * 
 */


#include <NefitSerial.h>


#define INTERFACE_ID 0x0B // Our bus interface address
#define RC10_ID 0x17 // Buderus RC10 and Nefit ModuLine 100
#define RC20_ID 0x17 // Buderus RC20 and Nefit ModuLine 200
#define RC30_ID 0x10 // Buderus RC30 and Nefit ModuLine 300 (Some ModuLine 300 also use 0x17!)
#define RC35_ID 0x10 // Buderus RC35 and Nefit ModuLine 400
#define UBA_ID 0x08 // The boiler address


char buffer[36];                              // frame buffer
byte pollAdres = 0;                           // if not 0, last polling address
char xmitBuffer[7] = {0x0B,0x17,0x00,0x00,0x00,0x00,0x00};            // load some defaults

long lastTime = 0;
long requestInterval = 30000; //30 second request interval

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



// Send a read request for a specific EMS register
boolean registerRequest(byte sender, byte receiver, byte frameType, byte frameSize)
{
boolean returnStatus = false;
int ptr;
xmitBuffer[0] = sender; // Address of the sender. Normally this is 0x0B.
xmitBuffer[1] = receiver | 0x80; // Receiver address masked with 0x80 so the receiver sees it as a read command.
xmitBuffer[2] = frameType; // The specific dataframe to be requested.
xmitBuffer[3] = 0; // The value 0 means we want to read the entire dataframe. (For setting register values this is the offset byte).
xmitBuffer[4] = frameSize; // frameSize of the dataframe in DEC.

sendRequest(xmitBuffer);
  long timeout = (long)millis() + 600; //This waiting time was previously 300ms, sometimes 600ms seems better.
  while ((((long)millis()-timeout)<0) && (!nefitSerial1.available())){}        // Wait for answer
  if (nefitSerial1.available()) {
    ptr = readBytes(buffer);
    // If more than 4 bytes are received, it means it is not a polling message but a datagram.
    if (ptr>4) {
      nefitSerial.println("Received a dataframe!");
      // Do a CRC check on the datagram to see if it has been received correctly.
      if (crcCheckOK(buffer,ptr)){
        // Check if the frameType we just received is the same as we requested.
       if(buffer[2] == frameType){
         nefitSerial.println("Correct frametype received.");
         nefitSerial.print("-----Datagram----- BYTES=");nefitSerial.println(ptr+1);
         nefitSerial.print("HEX: ");
            for (int i = 0; i<=ptr; i++){
              nefitSerial.print("0x");
            nefitSerial.print((uint8_t)buffer[i],HEX);
            nefitSerial.print(" ");
            }
        nefitSerial.println();nefitSerial.println("-----End of Datagram-----");
        returnStatus = true;
        }
else
        {
          nefitSerial.print("Wrong frametype received (0x");
          nefitSerial.print(buffer[2],HEX);
          nefitSerial.println(").");
         
          returnStatus = false;
        }
 
      }
    }
  }
  return returnStatus;
}



void setup() {
  //Init EMS bus
  nefitSerial1.begin(9700);

  //Init serial port debug
  nefitSerial.begin(9600);
  nefitSerial.println("EMS bus request started.");
}

void loop() {
  // The following is executed every time requestInterval expires (30 sec).
if(millis() - lastTime > requestInterval) {
    lastTime = millis(); 
    
    nefitSerial.println("Do request");

    registerRequest(INTERFACE_ID, UBA_ID, 0x18, 32); //This one reads 0x18 UBAMonitorFast from the boiler.

// Other examples:
// registerRequest(INTERFACE_ID, RC35_ID, 0x3E, 17); //This one reads 0x3E from the thermostat.
// registerRequest(INTERFACE_ID, UBA_ID, 0x19, 24); //This one reads 0x19 UBAMonitorSlow from the boiler.
  }
}
