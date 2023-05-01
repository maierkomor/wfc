#include "runcheck.h"
#include "runcheck.cpp"
#include "inv_def.h"


int main()
{
	M m,m2;

	assert(!m.has_str());	// init empty
	assert(m.has_strd());	// init with default
	assert(!m.has_stru());	// init with unset
	assert(m.has_strdu());	// init with default

	// string: clear after init
	m.clear_str();		// no effect without default and unset
	m.clear_strd();		// no effect with default, whithout unset
	m.clear_stru();		// no effect without default, with unset
	m.clear_strdu();	// Effect with default and unset, but no impact on transmission
	if (m != m2)
		fail("clear string",&m,&m2);
	runcheck(m);

	// static string: clear after init
	m.clear_st_str();	// no effect without default and unset
	m.clear_st_strd();	// no effect with default, whithout unset
	m.clear_st_stru();	// no effect without default, with unset
	m.clear_st_strdu();	// Effect with default and unset, but no impact on transmission!
	assert(m.st_strd() == m.st_strdu());
	if (m != m2)
		fail("clear static string",&m,&m2);
	runcheck (m);

	// unsigned: clear after init
	m.clear_u();		// no effect without default and unset
	m.clear_ud();		// no effect with default, whithout unset
	m.clear_uu();		// no effect without default, with unset
	m.clear_udu();		// Effect with default and unset, but no impact on transmission
	if (m != m2)
		fail("clear unsigned",&m,&m2);
	runcheck (m);

	// signed: clear after init
	m.clear_s();		// no effect without default and unset
	m.clear_sd();		// no effect with default, whithout unset
	m.clear_su();		// no effect without default, with unset
	m.clear_sdu();		// Effect with default and unset, but no impact on transmission
	if (m != m2)
		fail("clear signed",&m,&m2);
	runcheck (m);

	// enum: clear after init
	m.clear_n();		// no effect without default and unset
	m.clear_nd();		// no effect with default, whithout unset
	m.clear_nu();		// no effect without default, with unset
	m.clear_ndu();		// Effect with default and unset, but no impact on transmission
	if (m != m2)
		fail("clear enum",&m,&m2);
	runcheck(m);

	m.clear();
	runcheck(m);

	/*
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
	*/
}
