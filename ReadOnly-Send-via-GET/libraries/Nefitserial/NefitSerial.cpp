/*
  NefitSerial.cpp - Hardware serial library for Wiring
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
  
  Modified 23 November 2006 by David A. Mellis
  Modified 28 September 2010 by Mark Sproul
  Modified 14 August 2012 by Alarus
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include "Arduino.h"
#include "wiring_private.h"

// this next line disables the entire NefitSerial.cpp, 
// this is so I can support Attiny series and any other chip without a uart
#if defined(UBRRH) || defined(UBRR0H) || defined(UBRR1H) || defined(UBRR2H) || defined(UBRR3H)

#include "NefitSerial.h"

/*
 * on ATmega8, the uart and its bits are not numbered, so there is no "TXC0"
 * definition.
 */
#if !defined(TXC0)
#if defined(TXC)
#define TXC0 TXC
#elif defined(TXC1)
// Some devices have uart1 but no uart0
#define TXC0 TXC1
#else
#error TXC0 not definable in NefitSerial.h
#endif
#endif

inline void store_char(unsigned char c, bool fe, NefitSerial *s)
{
  int i = (unsigned int)(s->_rx_buffer_head + 1) % SERIAL_BUFFER_SIZE;

  // if we should be storing the received character into the location
  // just before the tail (meaning that the head would advance to the
  // current location of the tail), we're about to overflow the buffer
  // and so we don't write the character or advance the head.
  if (i != s->_rx_buffer_tail) {
    s->_rx_buffer[s->_rx_buffer_head] = c;
	s->_error_flag[s->_rx_buffer_head] = fe;
    s->_rx_buffer_head = i;
  }
}

#if !defined(USART0_RX_vect) && defined(USART1_RX_vect)
// do nothing - on the 32u4 the first USART is USART1
#else
#if !defined(USART_RX_vect) && !defined(USART0_RX_vect) && \
    !defined(USART_RXC_vect)
  #error "Don't know what the Data Received vector is called for the first UART"
#else
  void nefitSerialEvent() __attribute__((weak));
  void nefitSerialEvent() {}
  #define nefitSerialEvent_implemented
#if defined(USART_RX_vect)
  ISR(USART_RX_vect)
#elif defined(USART0_RX_vect)
  ISR(USART0_RX_vect)
#elif defined(USART_RXC_vect)
  ISR(USART_RXC_vect) // ATmega8
#endif
  {
  #if defined(UDR0)

	bool fe = bitRead(UCSR0A, FE0);
	unsigned char c = UDR0;
	store_char(c, fe, &nefitSerial);
  #elif defined(UDR)
    bool fe = bitRead(UCSRA, FE);
	unsigned char c = UDR;
	store_char(c, fe, &nefitSerial);

  #else
    #error UDR not defined
  #endif
  }
#endif
#endif

#if defined(USART1_RX_vect)
  void nefitSerialEvent1() __attribute__((weak));
  void nefitSerialEvent1() {}
  #define nefitSerialEvent1_implemented
  ISR(USART1_RX_vect)
  {

  	bool fe = bitRead(UCSR1A, FE1);
	unsigned char c = UDR1;
	
	store_char(c, fe, &nefitSerial1);
  }
#endif

#if defined(USART2_RX_vect) && defined(UDR2)
  void nefitSerialEvent2() __attribute__((weak));
  void nefitSerialEvent2() {}
  #define nefitSerialEvent2_implemented
  ISR(USART2_RX_vect)
  {

	bool fe = bitRead(UCSR2A, FE2);
	unsigned char c = UDR2;
	
	store_char(c, fe, &nefitSerial2);
  }
#endif

#if defined(USART3_RX_vect) && defined(UDR3)
  void nefitSerialEvent3() __attribute__((weak));
  void nefitSerialEvent3() {}
  #define nefitSerialEvent3_implemented
  ISR(USART3_RX_vect)
  {

	bool fe = bitRead(UCSR3A, FE3);
	unsigned char c = UDR3;
	
	store_char(c, fe, &nefitSerial3);
  }
#endif

void nefitSerialEventRun(void)
{
#ifdef nefitSerialEvent_implemented
  if (nefitSerial.available()) nefitSerialEvent();
#endif
#ifdef nefitSerialEvent1_implemented
  if (nefitSerial1.available()) nefitSerialEvent1();
#endif
#ifdef nefitSerialEvent2_implemented
  if (nefitSerial2.available()) nefitSerialEvent2();
#endif
#ifdef nefitSerialEvent3_implemented
  if (nefitSerial3.available()) nefitSerialEvent3();
#endif
}



// Constructors ////////////////////////////////////////////////////////////////

NefitSerial::NefitSerial(
  volatile uint8_t *ubrrh, volatile uint8_t *ubrrl,
  volatile uint8_t *ucsra, volatile uint8_t *ucsrb,
  volatile uint8_t *ucsrc, volatile uint8_t *udr,
  uint8_t rxen, uint8_t txen, uint8_t rxcie)
{
  _rx_buffer_head = _rx_buffer_tail = 0;
  _error = false;
  _ubrrh = ubrrh;
  _ubrrl = ubrrl;
  _ucsra = ucsra;
  _ucsrb = ucsrb;
  _ucsrc = ucsrc;
  _udr   = udr;
  _rxen = rxen;
  _txen = txen;
  _rxcie = rxcie;

}

// Public Methods //////////////////////////////////////////////////////////////

void NefitSerial::begin(unsigned long baud)
{
  uint16_t baud_setting;

    *_ucsra = 0;

    baud_setting = (F_CPU /8 / baud - 1)/2;

  // assign the baud_setting, a.k.a. ubbr (USART Baud Rate Register)
  *_ubrrh = baud_setting >> 8;
  *_ubrrl = baud_setting;

  sbi(*_ucsrb, _rxen);
  sbi(*_ucsrb, _txen);
  sbi(*_ucsrb, _rxcie);

}

void NefitSerial::end()
{

  cbi(*_ucsrb, _rxen);
  cbi(*_ucsrb, _txen);
  cbi(*_ucsrb, _rxcie);  

  
  // clear any received data
  _rx_buffer_head = _rx_buffer_tail;
}

int NefitSerial::available(void)
{
  return (unsigned int)(SERIAL_BUFFER_SIZE + _rx_buffer_head - _rx_buffer_tail) % SERIAL_BUFFER_SIZE;
}

int NefitSerial::peek(void)
{
  if (_rx_buffer_head == _rx_buffer_tail) {
    return -1;
  } else {
    return _rx_buffer[_rx_buffer_tail];
  }
}

int NefitSerial::read(void)
{
  // if the head isn't ahead of the tail, we don't have any characters
  if (_rx_buffer_head == _rx_buffer_tail) {
    return -1;
  } else {
    unsigned char c = _rx_buffer[_rx_buffer_tail];
	_error = _error_flag[_rx_buffer_tail];
    _rx_buffer_tail = (unsigned int)(_rx_buffer_tail + 1) % SERIAL_BUFFER_SIZE;
    return c;
  }
}

void NefitSerial::flush()
{
  // clear read buffer
  _rx_buffer_head = _rx_buffer_tail;
  _error = false;

}

size_t NefitSerial::write(uint8_t c)
{
// we do direct write to the UART, for NEFIT we do not want interrupts for TX
  while (!(bitRead(*_ucsra,UDRE0))) {}
  *_udr = c;
  
  return 1;
}
  //----------------------------------------------------------------------------
  /**
   * Write a NEFIT end-of-frame character to the port
   * We do this by temporarily halting reception, change parity to even and then send a break-character.
   * The we restore the settings and re-enable reception
   * 
   */
  void NefitSerial::writeEOF(){
	uint8_t t;
	cbi(*_ucsrb, _rxen);   					//disable reception
	while (!(bitRead(*_ucsra,UDRE0))) {}		// wait for data register empty
	t = *_ucsrc;							// save settings
	sbi(*_ucsrc,UPM01);						//set parity even
	sbi(*_ucsra,TXC0);						//reset TX-complete (seems to be needed to get parity change)
	*_udr = 0;								//directly write a break
	while (!(bitRead(*_ucsra,TXC0))) {}		//wait for transmit complete
	*_ucsrc = t;							//restore settings
	sbi(*_ucsra,TXC0);						//reset TX-complete (seems to be needed to get parity change)
	sbi(*_ucsrb, _rxen);   					//re-enable reception
  }
 
NefitSerial::operator bool() {
	return true;
}

// Preinstantiate Objects //////////////////////////////////////////////////////

#if defined(UBRRH) && defined(UBRRL)
  NefitSerial nefitSerial(&UBRRH, &UBRRL, &UCSRA, &UCSRB, &UCSRC, &UDR, RXEN, TXEN, RXCIE);
#elif defined(UBRR0H) && defined(UBRR0L)
  NefitSerial nefitSerial(&UBRR0H, &UBRR0L, &UCSR0A, &UCSR0B, &UCSR0C, &UDR0, RXEN0, TXEN0, RXCIE0);
#elif defined(USBCON)
  // do nothing - Serial object and buffers are initialized in CDC code
#else
  #error no serial port defined  (port 0)
#endif

#if defined(UBRR1H)
  NefitSerial nefitSerial1(&UBRR1H, &UBRR1L, &UCSR1A, &UCSR1B, &UCSR1C, &UDR1, RXEN1, TXEN1, RXCIE1);
#endif
#if defined(UBRR2H)
  NefitSerial nefitSerial2(&UBRR2H, &UBRR2L, &UCSR2A, &UCSR2B, &UCSR2C, &UDR2, RXEN2, TXEN2, RXCIE2);
#endif
#if defined(UBRR3H)
  NefitSerial nefitSerial3(&UBRR3H, &UBRR3L, &UCSR3A, &UCSR3B, &UCSR3C, &UDR3, RXEN3, TXEN3, RXCIE3);
#endif

#endif // whole file

