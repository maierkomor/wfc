#include "comp_v1.h"
#include "comp_v2.h"
#include "runcheck.h"
#include "runcheck.cpp"
#include <math.h>


int main()
{
	V1::M m_v1;
	m_v1.set_i0(16);
	m_v1.set_s0("bla text");
	m_v1.set_f0(M_PI);
	m_v1.add_rd(100.1);
	m_v1.add_rd(1.0/3.0);
	runcheck(m_v1);
	m_v1.mutable_sm()->set_a("a string");
	m_v1.mutable_sm()->set_f(-3.1);
	*m_v1.add_rsm() = m_v1.sm();
	*m_v1.add_rsm() = m_v1.sm();
	runcheck(m_v1);
	m_v1.add_rn(V1::two);
	m_v1.add_rn(V1::one);
	m_v1.add_rn(V1::zero);
	runcheck(m_v1);
	m_v1.add_rs("kein");
	m_v1.add_rs("inhalt");
	m_v1.add_rs("nix");
	m_v1.add_rs("weiter");

	string xfer;
	m_v1.toString(xfer);

	V2::M m_v2;
	m_v2.i0();
	m_v2.s0();
	m_v2.f0();
	m_v2.sm();
	m_v2.rsm();
	m_v2.fromMemory(xfer.data(),xfer.size());
	stringstream ss1,ss2;
	m_v1.toASCII(ss1);
	m_v2.toASCII(ss2);
	string s1 = ss1.str();
	string s2 = ss2.str();
	if (s1 != s2) {
		cout << "v1_a:\n";
		cout << s1 << endl;
		cout << "v2_a:\n";
		cout << s2 << endl;
		abort();
	}
	xfer = "";
	m_v2.toString(xfer);
	assert(m_v2.calcSize() == xfer.size());
}
