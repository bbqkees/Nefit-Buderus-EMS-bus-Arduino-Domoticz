/*
/* EMS bus decoder to Domoticz sketch for Arduino with Ethernet module
 * * Version 1.0 Minimal - Github - May 2017
 * 
 * Copyright (c) 2017 - 'bbqkees' @ www.domoticz.com/forum
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

 * last edit  :  16-MAY-2017
 * 
 * 16-MAY-2017:  Minimal version for read only.
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
 * 
 * As I am Dutch, some variables might be in partial Dutch instead of English.
 * F.i. 'CV' is 'Centrale Verwarming', which means 'Central Heating'. So if you see 'cv' think 'ch'.
 * Same for 'WW" which is 'Warm Water' which translates to 'hot tap water' in English.
 * However I translated all the comments to English.
 * 
 * If you have question about this sketch and its methods, please ask me on the Domoticz forum.
 */
 
 
#define NEFIT_REG_MAX	17
 
// Set to "1" when debugging 
// If enabled, the Arduino sends the decoded values over the Serial port.
#define DEBUG 1
 
#include <NefitSerial.h>
#include <SPI.h>

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
* You will onyl see this data if you have this thermostat.
* Otherwise you need to check http://emswiki.thefischer.net/ for your thermostat.
* 
* You can only change the setpoint etc if you have connected the thermostat to the bus.
* 
*/

PROGMEM const unsigned char regNefitCoding[]={

0x08,0x18,0x01,0x05,					//#1   0 temperature water out (uitgaand) x 10
0x08,0x18,0x04,0x01,					//#2   1 burner power
0x08,0x18,0x07,0x00,					//#3   2 burner of/off
0x08,0x18,0x07,0x50,					//#4   3 heating pump on/off
0x08,0x18,0x07,0x60,					//#5   4 tap water heating on/off
0x08,0x18,0x0B,0x05,					//#6   5 boiler temperature x 10
0x08,0x18,0x0D,0x05,					//#7   6 temperature water in (cvreturn) x 10
0x08,0x18,0x11,0x06,					//#8   7 water pressure x 10
0x08,0x18,0x12,0x02,					//#9   8 status code 1st letter
0x08,0x18,0x13,0x02,					//#10  9 status code 2nd letter
0x08,0x34,0x01,0x05,					//#11 10 tap water temperature x 10
0x08,0x34,0x05,0x50,					//#12 11 heater boiler on/off (not always available)
0x17,0x91,0x01,0x04,					//#13 12 setpoint room temperature x 2
0x17,0x91,0x02,0x05,					//#14 13 room temperature x 10
0x17,0xA8,0x17,0x81,					//#15 14 setting 0=low, 1=manual, 2=clock
0x17,0xA8,0x1C,0x84,					//#16 15 overruled clock setting x 2 ( 0 = not overruled)
0x17,0xA8,0x1D,0x84					  //#17 16 manual setpoint temperature x 2
};

void printbuffer(char * buffer, int len );

char buffer[32];                				      // frame buffer
int nefitRegister[NEFIT_REG_MAX];	            // index number see above

long lastTime = 0;  
long interval = 10000;
long lastTimeeth = 0;  
long intervaleth = 30000;

int IDXcvpressure = 000; // Set here your own Domoticz ID's
int IDXburnerpower = 000;
int IDXburner = 000;
int IDXboiler = 000;
int IDXcvreturn = 000;
int IDXsupply = 000;
int IDXhotwater = 000;
int IDXcvpump = 000;

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


// Read a register value from frame-buffer, decide if necessary and return as int
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
             }
        }
      }
    }else {break;}
  }
 return;
}


//The part below contain all the httprequest functions.


// General value JSON URL. Prints 2 decimals
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

//  /json.htm?type=command&param=udevice&idx=IDX&nvalue=0&svalue=TEXT
void httpRequestTextWh(int IDX, float text) {
  // if there's a successful connection:
  if (client.connect(domoticz, port)) {
    client.print( "GET /json.htm?type=command&param=udevice&idx=");
    client.print(IDX);
    client.print("&nvalue=0&svalue=");
    client.print(text);
    client.println( " Wh HTTP/1.1");
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
  }
}


// -----------------------------------------------------------------------------------------------------------------

void loop(){
  int ptr;

 if (nefitSerial1.available()){  // check if we need to read
	ptr = readBytes(buffer);
	if (ptr>4){
	  if (crcCheckOK(buffer,ptr)){
	    nefitFrame2register(buffer,ptr);
	  }
	}
    }

#if DEBUG

    if(millis() - lastTime > interval) {
    lastTime = millis(); 
   for (int i = 0; i<=NEFIT_REG_MAX; i++){
   nefitSerial.print("nefitRegister["); nefitSerial.print(i); nefitSerial.print("]: ");
   nefitSerial.println(nefitRegister[i]);}
  nefitSerial.println("------------------------");
  nefitSerial.println("NEFIT EMS bus:");
  
  nefitSerial.print("cv water output: ");
  nefitSerial.println((float)nefitRegister[0]/10,1);      

  nefitSerial.print("burner power: ");
  nefitSerial.println((float)nefitRegister[1],1);      

  nefitSerial.print("burner on/off: ");
  nefitSerial.println((float)nefitRegister[2],DEC);   // burner  

  nefitSerial.print("cv pump on/off: ");
  nefitSerial.println((float)nefitRegister[3],DEC);   //cv pump  
 
  nefitSerial.print("hot water on/off: ");
  nefitSerial.println((float)nefitRegister[4],DEC);    

  nefitSerial.print("boiler temp: ");
  nefitSerial.println((float)nefitRegister[5]/10,1);    //keteltemp
  
  nefitSerial.print("cv return temp: ");
  nefitSerial.println((float)nefitRegister[6]/10,1);    //waterterug temp
  
  nefitSerial.print("waterpressure: ");
  nefitSerial.println((float)nefitRegister[7]/10,1);    // waterpressure

  nefitSerial.print("statuscode: ");
  nefitSerial.print((char)nefitRegister[8]);    
  nefitSerial.println((char)nefitRegister[9]);  
  
  nefitSerial.print("hot water temp: ");
  nefitSerial.println((float)nefitRegister[10]/10,1);   //hotwatertemp

  nefitSerial.print("boiler heater on/off: ");
  nefitSerial.println((float)nefitRegister[11],DEC);  

  //nefitSerial.print(",");
  //nefitSerial.print(nefitRegister[14],DEC);           //thermostat 0=low, 1=manual, 2=auto

  nefitSerial.println("END");
  nefitSerial.println("------------------------");
  nefitSerial.println("------------------------");
}
#endif



  // loop for GET requests
  // The idea is to send the values only if they have changed.
  
  
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
