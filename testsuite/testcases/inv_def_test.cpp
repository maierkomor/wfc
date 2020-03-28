#include "runcheck.h"
#include "runcheck.cpp"
#include "inv_def.h"


int main()
{
	M m,m2;

	assert(m.has_str());
	assert(m.has_u());
	assert(m.has_s());
	assert(m.str() == "def");
	assert(m.u() == 1);
	assert(m.s() == 0);
	assert(m.n0() == one);
	assert(m.n1() == one);
	assert(m.n2() == one);

	m.clear();
	m2.clear_str();
	m2.clear_u();
	m2.clear_s();
	m2.clear_n0();
	m2.clear_n1();
	m2.clear_n2();
	assert(m == m2);

	assert(!m.has_str());
	assert(!m.has_u());
	assert(!m.has_s());
	assert(m.str() == "n/a");
	assert(m.u() == 0);
	assert(m.s() == -1);
}
