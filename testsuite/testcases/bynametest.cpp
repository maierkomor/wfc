#include "reference.h"
#include "runcheck.h"
#include "runcheck.cpp"


int main()
{
	TestBench tb;
	int x;

	x = tb.setByName("VI32","-1");
	assert(x > 0);
	assert(tb.VI32() == -1);
	x = tb.setByName("SVI32","-999");
	assert(x > 0);
	assert(tb.SVI32() == -999);
	x = tb.setByName("RDouble","3.5");
	assert(x > 0);
	assert(tb.RDouble() == 3.5);
	x = tb.setByName("B0","on");
	assert(x > 0);
	assert(tb.B0() == true);
	x = tb.setByName("B1","0");
	assert(x > 0);
	assert(tb.B1() == false);
	x = tb.setByName("OFloat","-0.25");
	assert(x > 0);
	assert(tb.OFloat() == -0.25);
	x = tb.setByName("kvpair1.key","key");
	assert(x > 0);
	assert(tb.kvpair1().key() == "key");
	runcheck(tb);

	// invalid settings
	x = tb.setByName("SI8","-129");
	assert(x < 0);
	assert(tb.SI8() == 0);
	x = tb.setByName("SI8","-128");
	assert(x > 0);
	assert(tb.SI8() == -128);
	x = tb.setByName("PackedMsg.Bool","fail");
	assert(x < 0);
	assert(tb.PackedMsg().Bool() == true);
	x = tb.setByName("PackedMsg.Bool","false");
	assert(x > 0);
	assert(tb.PackedMsg().Bool() == false);
	runcheck(tb);

	// arrays
	x = tb.setByName("kvpairs[+]","");
	assert(x == 0);
	assert(!tb.kvpairs().empty());
	x = tb.setByName("kvpairs[0].key","k0");
	assert(x > 0);
	x = tb.setByName("kvpairs[0].value","v0");
	assert(x > 0);
	x = tb.setByName("kvpairs[+].key","k1");
	assert(x > 0);
	assert(tb.kvpairs().size() == 2);
	x = tb.setByName("kvpairs[1].value","v1");
	assert(x > 0);

	// delete element
	x = tb.setByName("kvpairs",0);
	assert(x == 0);
	assert(tb.kvpairs().size() == 0);
}
