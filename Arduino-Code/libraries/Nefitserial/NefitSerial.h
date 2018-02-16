/*
  NefitSerial.h - Hardware serial library for Wiring
  Copyright (c) 2006 Nicholas Zambetti.  All right reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

  Modified 28 September 2010 by Mark Sproul
  Modified 14 August 2012 by Alarus
*/

#ifndef NefitSerial_h
#define NefitSerial_h

#include <inttypes.h>

#include "Stream.h"

// Define constants and variables for buffering incoming serial data.  We're
// using a ring buffer (I think), in which head is the index of the location
// to which to write the next incoming character and tail is the index of the
// location from which to read.

#define SERIAL_BUFFER_SIZE 48


class NefitSerial : public Stream
{
  protected:
    volatile uint8_t *_ubrrh;
    volatile uint8_t *_ubrrl;
    volatile uint8_t *_ucsra;
    volatile uint8_t *_ucsrb;
    volatile uint8_t *_ucsrc;
    volatile uint8_t *_udr;
    uint8_t _rxen;
    uint8_t _txen;
    uint8_t _rxcie;
	uint8_t _error;

  public:
    volatile uint8_t _rx_buffer_head;
    volatile uint8_t _rx_buffer_tail;


    // Don't put any members after these buffers, since only the first
    // 32 bytes of this struct can be accessed quickly using the ldd
    // instruction.
    unsigned char _rx_buffer[SERIAL_BUFFER_SIZE];
    bool 		  _error_flag[SERIAL_BUFFER_SIZE];

    NefitSerial(
      volatile uint8_t *ubrrh, volatile uint8_t *ubrrl,
      volatile uint8_t *ucsra, volatile uint8_t *ucsrb,
      volatile uint8_t *ucsrc, volatile uint8_t *udr,
      uint8_t rxen, uint8_t txen, uint8_t rxcie);
    void begin(unsigned long);
    void begin(unsigned long, uint8_t);
    void end();
	void writeEOF();
    virtual int available(void);
	bool frameError() { bool ret = _error; 	_error = false;  return ret; }
    virtual int peek(void);
    virtual int read(void);
    virtual void flush(void);
    virtual size_t write(uint8_t);
    inline size_t write(unsigned long n) { return write((uint8_t)n); }
    inline size_t write(long n) { return write((uint8_t)n); }
    inline size_t write(unsigned int n) { return write((uint8_t)n); }
    inline size_t write(int n) { return write((uint8_t)n); }
    using Print::write; // pull in write(str) and write(buf, size) from Print
    operator bool();
};

#if defined(UBRRH) || defined(UBRR0H)
  extern NefitSerial nefitSerial;
#elif defined(USBCON)
  #include "USBAPI.h"
//  extern NefitSerial Serial_;  
#endif
#if defined(UBRR1H)
  extern NefitSerial nefitSerial1;
#endif
#if defined(UBRR2H)
  extern NefitSerial nefitSerial2;
#endif
#if defined(UBRR3H)
  extern NefitSerial nefitSerial3;
#endif

extern void serialEventRun(void) __attribute__((weak));

#endif
