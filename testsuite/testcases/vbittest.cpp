#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <fcntl.h>
#include <float.h>
#include <assert.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#ifdef __linux__
#include <values.h>
#else
#include <float.h>
#endif

#include "validbits.h"

#include <sink.h>
#include "runcheck.h"
#include "runcheck.cpp"

using namespace std;


void check71()
{
	OptionalFields71 tb;

	tb.clear();
	assert(!tb.has_ob0());
	tb.set_ob0(true);
	assert(tb.has_ob0());
	tb.set_ob2(false);
	tb.set_oi32_3(INT32_MIN);
	tb.set_oi32_5(INT32_MAX);
	runcheck(tb);

	assert(!tb.has_of32_1());
	tb.set_of32_1(0);
	assert(tb.has_of32_1());
	tb.set_of32_4(999999);
	tb.set_of32_7(-999999);
	runcheck(tb);

	assert(!tb.has_oi64_2());
	tb.set_oi64_2(INT64_MIN);
	assert(tb.has_oi64_2());
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
	assert(!tb.has_of64_4());
	tb.set_of64_4(0);
	assert(tb.has_of64_4());
	tb.set_of64_1(-1);
	tb.set_of64_0(1);
	runcheck(tb);

	tb.set_of16_2(INT16_MIN);
	tb.set_of16_5(INT16_MAX);
	tb.set_of16_3(INT8_MIN);
	assert(!tb.has_of16_6());
	tb.set_of16_6(INT8_MAX);
	assert(tb.has_of16_6());
	tb.set_of16_4(0);
	tb.set_of16_1(-1);
	tb.set_of16_0(1);
	runcheck(tb);

	tb.set_of8_3(INT8_MIN);
	assert(!tb.has_of8_6());
	tb.set_of8_6(INT8_MAX);
	assert(tb.has_of8_6());
	tb.set_of8_4(0);
	tb.set_of8_1(-1);
	tb.set_of8_0(1);
	runcheck(tb);

	tb.set_ofloat_2(M_PI);
	tb.set_ofloat_5(M_E);
	tb.set_ofloat_3(M_E*M_E);
	assert(!tb.has_ofloat_6());
	tb.set_ofloat_6(M_PI*M_PI);
	assert(tb.has_ofloat_6());
	tb.set_ofloat_4(1E77);
	tb.set_ofloat_1(FLT_MAX);
	tb.set_ofloat_0(FLT_MIN);
	runcheck(tb);

	tb.set_odouble_2(M_PI);
	tb.set_odouble_5(M_E);
	tb.set_odouble_3(M_E*M_E);
	tb.set_odouble_6(M_PI*M_PI);
	tb.set_odouble_4(1E77);
	assert(!tb.has_odouble_1());
	tb.set_odouble_1(DBL_MAX);
	assert(tb.has_odouble_1());
	tb.set_odouble_0(DBL_MIN);
	runcheck(tb);
}


void check63()
{
	OptionalFields63 tb;

	tb.clear();
	assert(!tb.has_ob0());
	tb.set_ob0(true);
	assert(tb.has_ob0());
	tb.set_ob2(false);
	tb.set_oi32_3(INT32_MIN);
	tb.set_oi32_5(INT32_MAX);
	runcheck(tb);

	assert(!tb.has_of32_1());
	tb.set_of32_1(0);
	assert(tb.has_of32_1());
	tb.set_of32_4(999999);
	tb.set_of32_7(-999999);
	runcheck(tb);

	assert(!tb.has_oi64_2());
	tb.set_oi64_2(INT64_MIN);
	assert(tb.has_oi64_2());
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
	assert(!tb.has_of64_4());
	tb.set_of64_4(0);
	assert(tb.has_of64_4());
	tb.set_of64_1(-1);
	tb.set_of64_0(1);
	runcheck(tb);

	tb.set_of16_2(INT16_MIN);
	tb.set_of16_5(INT16_MAX);
	tb.set_of16_3(INT8_MIN);
	assert(!tb.has_of16_6());
	tb.set_of16_6(INT8_MAX);
	assert(tb.has_of16_6());
	tb.set_of16_4(0);
	tb.set_of16_1(-1);
	tb.set_of16_0(1);
	runcheck(tb);

	tb.set_of8_3(INT8_MIN);
	assert(!tb.has_of8_6());
	tb.set_of8_6(INT8_MAX);
	assert(tb.has_of8_6());
	tb.set_of8_4(0);
	tb.set_of8_1(-1);
	tb.set_of8_0(1);
	runcheck(tb);

	tb.set_ofloat_2(M_PI);
	tb.set_ofloat_5(M_E);
	tb.set_ofloat_3(M_E*M_E);
	assert(!tb.has_ofloat_6());
	tb.set_ofloat_6(M_PI*M_PI);
	assert(tb.has_ofloat_6());
	tb.set_ofloat_4(1E77);
	tb.set_ofloat_1(FLT_MAX);
	tb.set_ofloat_0(FLT_MIN);
	runcheck(tb);
}


void check39()
{
	OptionalFields39 tb;

	tb.clear();
	assert(!tb.has_ob0());
	tb.set_ob0(true);
	assert(tb.has_ob0());
	tb.set_ob2(false);
	tb.set_oi32_3(INT32_MIN);
	tb.set_oi32_5(INT32_MAX);
	runcheck(tb);

	assert(!tb.has_of32_1());
	tb.set_of32_1(0);
	assert(tb.has_of32_1());
	tb.set_of32_4(999999);
	tb.set_of32_7(-999999);
	runcheck(tb);

	assert(!tb.has_oi64_2());
	tb.set_oi64_2(INT64_MIN);
	assert(tb.has_oi64_2());
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
	assert(!tb.has_of64_4());
	tb.set_of64_4(0);
	assert(tb.has_of64_4());
	tb.set_of64_1(-1);
	tb.set_of64_0(1);
	runcheck(tb);
}


void check31()
{
	OptionalFields31 tb;

	tb.clear();
	assert(!tb.has_ob0());
	tb.set_ob0(true);
	assert(tb.has_ob0());
	tb.set_ob2(false);
	tb.set_oi32_3(INT32_MIN);
	tb.set_oi32_5(INT32_MAX);
	runcheck(tb);

	assert(!tb.has_of32_1());
	tb.set_of32_1(0);
	assert(tb.has_of32_1());
	tb.set_of32_4(999999);
	tb.set_of32_7(-999999);
	runcheck(tb);

	assert(!tb.has_oi64_2());
	tb.set_oi64_2(INT64_MIN);
	assert(tb.has_oi64_2());
	tb.set_oi64_5(INT32_MAX);
	tb.set_oi64_3(INT32_MIN);
	tb.set_oi64_6(INT64_MAX);
	tb.set_oi64_4(0);
	tb.set_oi64_1(-1);
	tb.set_oi64_0(-1);
	runcheck(tb);
}


void check23()
{
	OptionalFields23 tb;

	tb.clear();
	assert(!tb.has_ob0());
	tb.set_ob0(true);
	assert(tb.has_ob0());
	tb.set_ob2(false);
	tb.set_oi32_3(INT32_MIN);
	tb.set_oi32_5(INT32_MAX);
	runcheck(tb);

	assert(!tb.has_of32_1());
	tb.set_of32_1(0);
	assert(tb.has_of32_1());
	tb.set_of32_4(999999);
	tb.set_of32_7(-999999);
	runcheck(tb);
}


void check15()
{
	OptionalFields15 tb;

	tb.clear();
	assert(!tb.has_ob0());
	tb.set_ob0(true);
	assert(tb.has_ob0());
	tb.set_ob2(false);
	tb.set_oi32_3(INT32_MIN);
	tb.set_oi32_5(INT32_MAX);
	runcheck(tb);
}

void check7()
{
	OptionalFields7 tb;

	tb.clear();
	assert(!tb.has_ob0());
	tb.set_ob0(true);
	assert(tb.has_ob0());
	assert(!tb.has_ob2());
	tb.set_ob2(false);
	assert(tb.has_ob2());
	tb.set_ob2(false);
	runcheck(tb);
}




int main(int argc, char *argv[])
{
	check71();
	check63();
	check39();
	check31();
	check23();
	check15();
	check7();

	printf("%s: %s\n",argv[0],testcnt());
}
