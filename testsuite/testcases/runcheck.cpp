/*
 *  Copyright (C) 2017-2021, Thomas Maier-Komor
 *
 *  This source file belongs to Wire-Format-Compiler.
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "runcheck.h"

#ifdef __AVR__
#include <avr/io.h>

void uart_config()
{
#define BAUD 57600
#include <util/setbaud.h>
	// set uart clock scaling based on system clock
	UBRR0H = UBRRH_VALUE;
	UBRR0L = UBRRL_VALUE;
#if USE_2X
	UCSR0A |= (1 << U2X0);
#else
	UCSR0A &= ~(1 << U2X0);
#endif
	PORTD |= (1 << PD1);	// enable TX Pin
	DDRD |= (1 << PD1);
	UCSR0B |= (1<<TXEN0);	// enable sender
	UCSR0C = (1<<UPM01) | (0<<UPM00) | (1 << UCSZ01) | (1 << UCSZ00) | (0 << USBS0);	// 8 data bits, no parity, 1 stop bit
}

void uart_send_char(char c)
{
	while (!(UCSR0A&(1<<UDRE0)));
	UDR0 = c;
}

void uart_test()
{
	uart_send_char('a');
	uart_send_char('b');
	uart_send_char('c');
	uart_send_char('\n');
	uart_send_char('\r');
}

void uart_hex_byte(uint8_t v)
{
	char x = (v >> 4) & 0xf;
	if (x > 9)
		uart_send_char(x + 'a' - 10);
	else
		uart_send_char(x + '0');
	x = v & 0xf;
	if (x > 9)
		uart_send_char(x + 'a' - 10);
	else
		uart_send_char(x + '0');
}

void uart_send(const char *d)
{
	char c;
	while ((c = *d++) != 0) {
		while (!(UCSR0A&(1<<UDRE0)));
		UDR0 = c;
	}
}

void uart_send_p(const char *d)
{
	char c;
	while ((c = pgm_read_byte(d++)) != 0) {
		while (!(UCSR0A&(1<<UDRE0)));
		UDR0 = c;
	}
}

void fail(const char *err)
{
	uart_send_char('f');
	uart_send_char('f');
	uart_send_p(err);
	uart_send_char('\n');
	uart_send_char('\r');
}
#else // PC

#include <string>
#include <stdint.h>

using namespace std;

stringtype Wire;
unsigned NumToASCII, NumToWire, NumToMem, NumFromMem, NumToSink, NumErrThrow;

const char *testcnt()
{
	static char buf[128];
	snprintf(buf,sizeof(buf),"fromMem: %u, toMem: %u, toWire: %u, toSink: %u, toASCII: %u, throw/catch: %u"
		, NumFromMem, NumToMem, NumToWire, NumToSink, NumToASCII, NumErrThrow);
	return buf;
}


void putwire(uint8_t b)
{
	Wire.push_back(b);
}


void hexdump(uint8_t *a, size_t s)
{
	size_t off = 0;
	while (s >= 16) {
		printf("%04lx:   ",(unsigned long)off);
		for (int i = 0; i < 8; ++i)
			printf("%02x ",(unsigned)a[off++]);
		for (int i = 0; i < 8; ++i)
			printf(" %02x",(unsigned)a[off++]);
		printf("\n");
		s -= 16;
	}
	printf("%04lx:  ",(unsigned long)off);
	for (int i = 0; (i < 8) && (off < s); ++i)
		printf("%02x ",(unsigned)a[off++]);
	for (int i = 0; (i < 8) && (off < s); ++i)
		printf(" %02x",(unsigned)a[off++]);
	printf("\n");
}

#endif

