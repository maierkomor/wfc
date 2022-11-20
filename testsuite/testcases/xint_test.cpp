#include "xint.h"
#include <assert.h>
#include "runcheck.h"
#include "runcheck.cpp"


int main()
{
	xint x;
	x.set_i8(-1);
	x.set_i16(-1);
	x.set_s8(-1);
	x.set_s16(-1);
	runcheck(x);

	x.set_i8(1);
	x.set_i16(1);
	x.set_s8(1);
	x.set_s16(1);
	runcheck(x);

	x.set_i8(INT8_MAX);
	x.set_i16(INT16_MAX);
	x.set_s8(INT8_MAX);
	x.set_s16(INT16_MAX);
	runcheck(x);

	x.set_i8(INT8_MIN);
	x.set_i16(INT16_MIN);
	x.set_s8(INT8_MIN);
	x.set_s16(INT16_MIN);
	runcheck(x);
}
