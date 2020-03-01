#include <assert.h>

#include "astr.h"
#include "runcheck.h"
#include "runcheck.cpp"


int main()
{
	A a;
	a.set_str("abc");
	assert(a.str() == "abc");
	*a.mutable_str() += "def";
	assert(a.str() == "abcdef");
	*a.mutable_str() += 'g';
	assert(a.str() == "abcdefg");
	runcheck(a);
	a.add_arr("abc");
	assert(a.arr().back() == "abc");
	a.add_arr(AString("defg",3));
	assert(a.arr().back() == "def");
	runcheck(a);
	a.add_arr(AString("ghi"));
	runcheck(a);
}
