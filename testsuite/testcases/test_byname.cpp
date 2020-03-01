#include "byname.h"

using namespace std;

int main()
{
	int x;
	M m;
	x = m.setByName("pairs[0].key","key0");
	assert(x <= 0);	// pairs[0] not existing

	m.mutable_pairs()->resize(1);
	x = m.setByName("pairs[0].key","key0");
	assert(x > 0);
	assert(m.pairs(0).has_key());

	x = m.setByName("pairs[0].value","v0");
	assert(x > 0);
	assert(m.pairs(0).has_value());

	x = m.setByName("N","1");
	assert(x > 0);
	assert(m.has_N());
	assert(m.N() == one);

	m.mutable_RN()->resize(1);
	x = m.setByName("RN[0]","2");
	assert(x > 0);
	assert(m.RN(0) == two);

	x = m.setByName("F16","66000");
	assert(x < 0);
	assert(m.F16() == 0);
	x = m.setByName("F16","65535");
	assert(m.F16() == 65535);

	x = m.setByName("S16","66000");
	assert(x < 0);
	x = m.setByName("S16","-99");
	assert(m.S16() == -99);

	x = m.setByName("D","0.25");
	assert(x > 0);
	assert(m.D() == 0.25);

	m.mutable_RD()->resize(1);
	x = m.setByName("RD[0]","-1.5");
	assert(x > 0);
	assert(m.RD(0) == -1.5);

	x = m.setByName("S","text");
	assert(x > 0);
	assert(m.S() == "text");

	m.mutable_RS()->resize(1);
	x = m.setByName("RS[0]","nothing");
	assert(x > 0);
	assert(m.RS(0) == "nothing");

	x = m.setByName("B","1");
	assert(x > 0);
	assert(m.B() == true);
	x = m.setByName("B","0");
	assert(x > 0);
	assert(m.B() == false);

	x = m.setByName("B","true");
	assert(x > 0);
	assert(m.B() == true);
	x = m.setByName("B","false");
	assert(x > 0);
	assert(m.B() == false);

	x = m.setByName("B","on");
	assert(x > 0);
	assert(m.B() == true);
	x = m.setByName("B","off");
	assert(x > 0);
	assert(m.B() == false);

	m.mutable_RB()->resize(2);
	x = m.setByName("RB[0]","on");
	assert(x > 0);
	assert(m.RB(0) == true);
	x = m.setByName("RB[1]","true");
	assert(x > 0);
	assert(m.RB(1) == true);
}
