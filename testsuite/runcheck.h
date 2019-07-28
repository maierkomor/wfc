/*
 *  Copyright (C) 2017-2019, Thomas Maier-Komor
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

#ifndef RUNCHECK_H
#define RUNCHECK_H

#include <sink.h>

#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#ifdef __AVR__
#define F_CPU 16000000
#include <avr/pgmspace.h>
#include <util/delay.h>
#else
#include <string>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <iostream>
#include <sstream>
using namespace std;
#endif

void putwire(uint8_t b);


#ifdef __AVR__
void uart_config();
void uart_test();
void uart_hex_byte(uint8_t v);
void uart_send_char(char c);
void uart_send_p(const char *d);
void uart_send(const char *d);
void fail(const char *err);

template <class Message>
void runcheck(const Message &tb)
{
	size_t s = tb.calcSize();
	uint8_t buf[s*2];
	size_t m = tb.toMemory(buf,sizeof(buf));
	for (size_t i = 0; i != m; ++i) {
		uart_hex_byte(buf[i]);
		uart_send_char(' ');
	}
	if (m != s) 
		uart_send_p(PSTR("m!=s"));
	Message fw;
	fw.fromMemory(buf,s);
	if (tb != fw)
		uart_send_p(PSTR("from\n\r"));
	uart_send_p(PSTR("\n\rnode:'"));
	uart_send(fw.node().c_str());
	uart_send_p(PSTR("'\n\r"));
	while(1);
}

#else // PC

#include <stdio.h>
#define PSTR(STR) STR
extern std::string Wire;
extern unsigned NumToASCII, NumToWire, NumToMem, NumFromMem, NumToSink, NumErrThrow;

const char *testcnt();
void hexdump(uint8_t *a, size_t s);

template <class Message>
void fail(const char *msg, const Message *l, const Message *r = 0)
{
#ifdef HAVE_TO_ASCII
	std::cout << "l:\n";
	l->toASCII(std::cout);
	if (r) {
		std::cout << "\nr:\n";
		r->toASCII(std::cout);
	}
#else
	std::cout << "cannot output as ascii: toASCII support missing\n";
#endif
	int fd = open("dump.bin",O_WRONLY|O_CREAT,0666);
	assert(fd != -1);
	int n = write(fd,Wire.data(),Wire.size());
	assert(n >= 0);
	std::cout << "failure in " << msg << endl;
	abort();
}

template <class Message>
void runcheck(const Message &tb)
{
	ssize_t s = tb.calcSize();

	uint8_t *buf = (uint8_t *)malloc(s*2);
	memset(buf,0xff,s*2);
	ssize_t m;
#ifdef ON_ERROR_CANCEL
	if (s > 0) {
		m = tb.toMemory(buf,s-1);
		++NumToMem;
		assert(m <= 0);
	}
#elif defined ON_ERROR_THROW
	if (s > 0) {
		bool ok = false;
		try {
			++NumToMem;
			m = tb.toMemory(buf,s-1);
			abort();
		} catch (int x) {
			++NumErrThrow;
			ok = true;
			assert(x < 0);
		}
		assert(ok);
	}
#endif
	++NumToMem;
	m = tb.toMemory(buf,s*2);
	if (m != s)
		fail("toMemory",&tb);
	Message fw;
	++NumFromMem;
	fw.fromMemory(buf,s);
	if (tb != fw) {
		hexdump(buf,s);
		fail("fromMemory",&tb,&fw);
	}

#ifdef HAVE_TO_SINK
	DetermineSize ds;
	++NumToSink;
	tb.toSink(ds);
	assert(ds.getSize() == s);

	ToBufferUnchecked ucb(buf);
	++NumToSink;
	tb.toSink(ucb);
	assert(ucb.getSize() == s);

	fw.clear();
	++NumFromMem;
	fw.fromMemory(buf,ucb.getSize());
	if (tb != fw)
		fail("tb == fw",&tb,&fw);
#endif
	

#ifdef HAVE_TO_STRING
	std::string str;
	tb.toString(str);
	if (s != (ssize_t)str.size())
		fail("to_str size",&tb);
	fw.clear();
	++NumFromMem;
	fw.fromMemory(str.data(),str.size());
	assert((tb != fw) == !(tb == fw));
	assert((tb != fw) == (fw != tb));
	if (tb != fw) {
		hexdump(buf,s);
		fail("str tb==fw",&tb,&fw);
	}
#endif

	Wire.clear();
#ifdef WIREPUT_FUNCTION
	++NumToWire;
	tb.toWire();
#else
	++NumToWire;
	tb.toWire(putwire);
#endif
	if ((Wire.size() != (size_t)s) || memcmp(Wire.data(),buf,s)) {
		printf("expected:\n");
		hexdump(buf,s);
		printf("got:\n");
		hexdump((uint8_t*)Wire.data(),(ssize_t)Wire.size());
		abort();
	}
	fw.clear();
	++NumFromMem;
	fw.fromMemory(Wire.data(),Wire.size());
	if (tb != fw) {
		hexdump(buf,s);
		fail("fromMem",&tb,&fw);
	}

#ifdef HAVE_TO_STRING
	if (Wire.size() != str.size())
		fail("wirecmp",&tb);
	if (0 != memcmp(Wire.data(),str.data(),Wire.size()))
		fail("memcmp",&tb);
#endif

#ifdef HAVE_TO_ASCII
	stringstream ss0, ss1;
	++NumToASCII;
	tb.toASCII(ss0);
	++NumToASCII;
	fw.toASCII(ss1);
	if (ss0.str() != ss1.str())
		fail("ascii",&tb);
#endif

	free(buf);
}
#endif


#endif
