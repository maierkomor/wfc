#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <fcntl.h>
#include <assert.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#ifdef __MINGW32__
#include <float.h>
#else
#include <values.h>
#endif

#include "validbits.h"

#include <sink.h>
#include "runcheck.h"
#include "runcheck.cpp"

using namespace std;


int main(int argc, char *argv[])
{
	OptionalFields tb;

	tb.clear();
	tb.set_ob0(true);
	tb.set_ob2(false);
	tb.set_oi32_3(INT32_MIN);
	tb.set_oi32_5(INT32_MAX);
	runcheck(tb);

	tb.set_of32_1(0);
	tb.set_of32_4(999999);
	tb.set_of32_7(-999999);
	runcheck(tb);

	tb.set_oi64_2(INT64_MIN);
	tb.set_oi64_5(INT32_MAX);
	tb.set_oi64_3(INT32_MIN);
	tb.set_oi64_6(INT64_MAX);
	tb.set_oi64_4(0);
	tb.set_oi64_1(-1);
	tb.set_oi64_0(-1);
	runcheck(tb);

	tb.set_of64_2(INT64_MIN);
	tb.set_of64_5(INT32_MAX);
	tb.set_of64_3(INT32_MIN);
	tb.set_of64_6(INT64_MAX);
	tb.set_of64_4(0);
	tb.set_of64_1(-1);
	tb.set_of64_0(1);
	runcheck(tb);

	tb.set_of16_2(INT16_MIN);
	tb.set_of16_5(INT16_MAX);
	tb.set_of16_3(INT8_MIN);
	tb.set_of16_6(INT8_MAX);
	tb.set_of16_4(0);
	tb.set_of16_1(-1);
	tb.set_of16_0(1);
	runcheck(tb);

	tb.set_of8_3(INT8_MIN);
	tb.set_of8_6(INT8_MAX);
	tb.set_of8_4(0);
	tb.set_of8_1(-1);
	tb.set_of8_0(1);
	runcheck(tb);

	tb.set_ofloat_2(M_PI);
	tb.set_ofloat_5(M_E);
	tb.set_ofloat_3(M_E*M_E);
	tb.set_ofloat_6(M_PI*M_PI);
	tb.set_ofloat_4(1E77);
	tb.set_ofloat_1(FLT_MAX);
	tb.set_ofloat_0(FLT_MIN);
	runcheck(tb);

	tb.set_odouble_2(M_PI);
	tb.set_odouble_5(M_E);
	tb.set_odouble_3(M_E*M_E);
	tb.set_odouble_6(M_PI*M_PI);
	tb.set_odouble_4(1E77);
	tb.set_odouble_1(DBL_MAX);
	tb.set_odouble_0(DBL_MIN);
	runcheck(tb);

	printf("%s: %s\n",argv[0],testcnt());
}

