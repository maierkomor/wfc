#include <assert.h>
#include <stdio.h>
#include <iostream>
#include <sstream>

#include "xvarint.h"
#include "xvarint_16.h"
#include "xvarint_32.h"
#include "runcheck.h"
#include "runcheck.cpp"

using namespace std;


int main(int argc, char **argv)
{
	uint8_t buf[1024];

	VI16::M m16;
	m16.set_i0(-1);
	m16.set_i1(1111);
	m16.set_i2(-1111);
	m16.set_s0(-2);
	m16.set_s1(10000);
	m16.set_s2(-20000);
	runcheck(m16);

	ssize_t n = m16.toMemory(buf,sizeof(buf));
	assert(n > 0);
	stringstream s16;
	m16.toASCII(s16);
	/*
	for (ssize_t i = 0; i < n; ++i) {
		if (n - i >= 8) {
			printf("%02x %02x %02x %02x  %02x %02x %02x %02x\n"
				,buf[i+0],buf[i+1],buf[i+2],buf[i+3]
				,buf[i+4],buf[i+5],buf[i+6],buf[i+7]);
			i += 7;
		} else {
			printf("%02x ",buf[i]);
		}
	}
	printf("\n");
	*/

	stringstream s32;
	VI32::M m32;
	m32.fromMemory(buf,n);
	runcheck(m32);
	m32.toASCII(s32);
	if (s16.str() != s32.str()) {
		cout << "16bit:\n" << s16.str() << endl;
		cout << "32bit:\n" << s32.str() << endl;
		abort();
	}

	stringstream s64;
	M m;
	m.fromMemory(buf,n);
	runcheck(m);
	m.toASCII(s64);
	if (s16.str() != s64.str()) {
		cout << "16bit:\n" << s16.str() << endl;
		cout << "64bit:\n" << s64.str() << endl;
		abort();
	}
	printf("%s: %s\n",argv[0],testcnt());
	return 0;
}
