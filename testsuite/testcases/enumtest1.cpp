#include <string>
#include "enumtest.h"
#include <stdio.h>
#include "runcheck.h"
#include "runcheck.cpp"

#ifdef SUBCLASSES
#define EnumTest_A EnumTest::A
#define EnumTest_B EnumTest::B
#define fluid EnumTest::A::fluid
#define frozen EnumTest::A::frozen
#define ice EnumTest::B::ice
#endif

using namespace std;


int main(int argc, const char *argv[])
{
	EnumTest et;
	EnumTest_A *a = et.mutable_a();
	a->set_w(fluid);
	runcheck(et);
	a->clear_w();
	runcheck(et);
	a->set_w(frozen);
	runcheck(et);
	EnumTest_B *b = et.mutable_b();
	b->set_water(ice);
	runcheck(et);
	et.clear_a();
	runcheck(et);
	et.clear_b();
	runcheck(et);

	int r = et.setByName("a.w","frozen");
	assert(r == 6);
	assert(et.a().w() == -1);

	r = et.setByName("b.water","ice");
	assert(r == 3);
	assert(et.b().water() == -1);

	r = et.setByName("a.w","fluid");
	assert(r == 5);
	assert(et.a().w() == 1);

	r = et.setByName("b.water","steam");
	assert(r == 5);
	assert(et.b().water() == 1);


	printf("%s: %s\n",argv[0],testcnt());
}
