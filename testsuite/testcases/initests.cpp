#include <string>
#include "initest.h"
#include <stdio.h>
#include "runcheck.h"
#include "runcheck.cpp"

int main(int argc, char *argv[])
{
	M m;
	runcheck(m);

	assert(m.VI32() == 1);
	assert(m.SVI32() == 2);
	assert(m.VI64() == 3);
	assert(m.SVI64() == 4);
	assert(m.RDouble() == 5);
	assert(m.B0() == false);
	assert(m.B1() == true);
	assert(m.FI8() == 8);
	assert(m.FI16() == 9 );
	assert(m.FI32() == 10);
	assert(m.FI64() == 11);
	assert(m.STR() == "12");
	assert(m.STRu() == "13");
	assert(m.Float() == 14);

	assert(m.oVI32() == 21);
	assert(m.oSVI32() == 22);
	assert(m.oVI64() == 23);
	assert(m.oDouble() == 25);
	assert(m.oB0() == false);
	assert(m.oB1() == true);
	assert(m.oFI8() == 28);
	assert(m.oFI16() == 29 );
	assert(m.oFI32() == 30);
	assert(m.oFI64() == 31);
	assert(m.oSTR() == "32");
	assert(m.oSTRu() == "33");
	assert(m.oFloat() == 34);
}
