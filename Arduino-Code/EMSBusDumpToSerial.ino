/* EMS register bus dump sketch for Arduino Mega.
 * * Version 1.1 Github - May 2018
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
 * last edit  :  08-MAY-2018
 * 
 * 08-MAY-2018:  Added a quick filter option to the serial output.
 * 02-MAY-2018:  Initial version
 * 
 * What does it do?
 * This sketch listens to incoming datagram messages on the EMS bus and prints them via the Serial debug console.
 * Set POLLING to 1 if you also want to see all the polling messages on the bus.
 * 
 * Hardware:
 * -Arduino Mega 2560.
 * -EMS interface circuit on Serial1.
 * -PC via USB for debug on Serial(0).
 * 
 * 
 */


#include <NefitSerial.h>

#define POLLING 0 //Set this to 1 if you want to see all messages on the bus.
                  //There can be quite some polling messages.


char buffer[40];                              // frame buffer

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


void setup() {
  //Init EMS bus
  nefitSerial1.begin(9700);

  //Init serial port debug
  nefitSerial.begin(9600);
  nefitSerial.println("EMS bus dump started.");
}

void loop() {
 int ptr;
 if (nefitSerial1.available()){  // check if we need to read
  ptr = readBytes(buffer);
  
#if POLLING  

    if (ptr == 2) {
        nefitSerial.print("Received a polling message. HEX: ");
        for (int i = 0; i<=ptr; i++){
            nefitSerial.print("0x");
            nefitSerial.print((uint8_t)buffer[i],HEX);
            nefitSerial.print(" ");
      }
      nefitSerial.println();
    }

else if (ptr>4){
  
#endif
#if !POLLING
if (ptr>4){
#endif  
 
    if (crcCheckOK(buffer,ptr)){
      // Uncomment ONE of the following three lines if you want to filter on a specific transmitter/receiver/frametype.
      // if(buffer[0] == 0x10){ //filter on a specific transmitter f.i. only data sent by the RC35 thermostat.
      // if(buffer[1] == 0x10 || buffer[1] == (0x10 | 0x80)){ //filter on a specific receiver f.i. only data sent to RC35 thermostat.
      // if(buffer[2] == 0x18){ //filter on a specific frametype f.i. 0x18 UBAMonitorFast.
      
      nefitSerial.println("Received datagram.");
      nefitSerial.print("-----Datagram----- BYTES=");nefitSerial.println(ptr+1);
      nefitSerial.print("HEX: ");
            for (int i = 0; i<=ptr; i++){
              nefitSerial.print("0x");
            nefitSerial.print((uint8_t)buffer[i],HEX);
            nefitSerial.print(" ");
            }
      nefitSerial.println();nefitSerial.println("-----End of Datagram-----");
    // } // end if buffer[] -> uncomment this bracket when enabling any specific filter (see above).
    } // end crccheck
  } // end if ptr>4
  } // end if nefitSerial1.available()
} // end void loop
