#include "arraycheck.h"
#include "runcheck.h"
#include "runcheck.cpp"

#include <math.h>

int main()
{
	M m;
	SM sm;

	char buf[4];
	buf[0] = 's';
	buf[2] = 0;
	buf[3] = 0xff;
	sm.set_i(7);
	sm.set_f(M_PI);
	sm.set_a(buf);
	sm.set_x(buf);
	sm.set_s(buf);
	sm.mutable_b()->assign(buf,sizeof(buf));
	for (int i = 0; i < 8; ++i) {
		buf[0] = 's';
		buf[1] = '0'+i;
		sm.set_s(buf);
		m.add_s(buf);
		m.mutable_b()->emplace_back(buf,4);
		m.add_i(i);
		m.add_f((float)i);
		buf[0] = 'a';
		m.add_a(buf);
		sm.set_a(buf);
		buf[0] = 'x';
		m.add_x(buf);
		sm.set_x(buf);
		m.mutable_m()->push_back(sm);
	}
	runcheck(m);
	m.mutable_s()->erase(m.mutable_s()->begin()+7);
	m.mutable_b()->erase(m.mutable_b()->begin()+7);
	m.mutable_i()->erase(m.mutable_i()->begin()+7);
	m.mutable_f()->erase(m.mutable_f()->begin()+7);
	m.mutable_x()->erase(m.mutable_x()->begin()+7);
	m.mutable_a()->erase(m.mutable_a()->begin()+7);
	m.mutable_m()->erase(m.mutable_m()->begin()+7);
	assert(m.s_size() == 7);
	assert(m.b_size() == 7);
	assert(m.i_size() == 7);
	assert(m.f_size() == 7);
	assert(m.a_size() == 7);
	assert(m.x_size() == 7);
	runcheck(m);

	m.mutable_s()->erase(m.mutable_s()->begin());
	m.mutable_b()->erase(m.mutable_b()->begin());
	m.mutable_i()->erase(m.mutable_i()->begin());
	m.mutable_f()->erase(m.mutable_f()->begin());
	m.mutable_x()->erase(m.mutable_x()->begin());
	m.mutable_a()->erase(m.mutable_a()->begin());
	m.mutable_m()->erase(m.mutable_m()->begin());
	runcheck(m);

	assert(m.s(0) == "s1");
	assert(m.a(0) == "a1");
	assert(m.x(0) == "x1");
	assert(m.m(0).x() == "x1");
	assert(m.m(0).s() == "s1");
	assert(m.m(0).a() == "a1");

	m.mutable_s()->erase(m.mutable_s()->begin()+1);
	m.mutable_b()->erase(m.mutable_b()->begin()+1);
	m.mutable_i()->erase(m.mutable_i()->begin()+1);
	m.mutable_f()->erase(m.mutable_f()->begin()+1);
	m.mutable_x()->erase(m.mutable_x()->begin()+1);
	m.mutable_a()->erase(m.mutable_a()->begin()+1);
	m.mutable_m()->erase(m.mutable_m()->begin()+1);
	runcheck(m);

	assert(m.s(0) == "s1");
	assert(m.a(0) == "a1");
	assert(m.x(0) == "x1");
	assert(m.m(0).x() == "x1");
	assert(m.m(0).s() == "s1");
	assert(m.m(0).a() == "a1");
}
