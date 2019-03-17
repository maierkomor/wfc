#include "tt.h"

#include <iostream>
#include <assert.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include <math.h>

#include <string>

#include "runcheck.h"
#include "runcheck.cpp"

#ifdef SUBCLASSES
#define two TypeTest::two
#define three TypeTest::three
#define four TypeTest::four
#endif

using namespace std;

float F32_Values[] = {
	0,
	1.1E-5,
	1.1E-10,
	1.1E-20,
	1.1E-100,
	1,
	1.1E5,
	1.1E10,
	1.1E20,
	M_PI,
	M_PI_2,
	M_PI_4,
	M_LOG2E,
	M_LOG10E,
	M_LN2,
	M_LN10,
	M_2_SQRTPI,
	M_SQRT2,
	M_SQRT1_2,
};

double F64_Values[] = {
	0,
	1.1E-5,
	1.1E-10,
	1.1E-20,
	1.1E-100,
	1,
	1.1E5,
	1.1E10,
	1.1E20,
	1.1E100,
	M_PI,
	M_PI_2,
	M_PI_4,
	M_LOG2E,
	M_LOG10E,
	M_LN2,
	M_LN10,
	M_2_SQRTPI,
	M_SQRT2,
	M_SQRT1_2,
	HUGE_VAL,
};

int64_t I64_Values[] = {
	INT64_MIN,
	INT64_MIN+1,
	0,

	1ULL<<0,
	1ULL<<1,
	1ULL<<2,
	1ULL<<3,
	1ULL<<4,
	1ULL<<5,
	1ULL<<6,
	1ULL<<7,
	1ULL<<8,
	1ULL<<9,
	1ULL<<10,
	1ULL<<11,
	1ULL<<12,
	1ULL<<13,
	1ULL<<14,
	1ULL<<15,
	1ULL<<16,
	1ULL<<17,
	1ULL<<18,
	1ULL<<19,
	1ULL<<20,
	1ULL<<21,
	1ULL<<22,
	1ULL<<23,
	1ULL<<24,
	1ULL<<25,
	1ULL<<26,
	1ULL<<27,
	1ULL<<28,
	1ULL<<29,
	1ULL<<30,
	1ULL<<31,
	1ULL<<32,
	1ULL<<33,
	1ULL<<34,
	1ULL<<35,
	1ULL<<36,
	1ULL<<37,
	1ULL<<38,
	1ULL<<39,
	1ULL<<40,
	1ULL<<41,
	1ULL<<42,
	1ULL<<43,
	1ULL<<44,
	1ULL<<45,
	1ULL<<46,
	1ULL<<47,
	1ULL<<48,
	1ULL<<49,
	1ULL<<50,
	1ULL<<51,
	1ULL<<52,
	1ULL<<53,
	1ULL<<54,
	1ULL<<55,
	1ULL<<56,
	1ULL<<57,
	1ULL<<58,
	1ULL<<59,
	1ULL<<60,
	1ULL<<61,
	1ULL<<62,

	-(1LL<<0),
	-(1LL<<1),
	-(1LL<<2),
	-(1LL<<3),
	-(1LL<<4),
	-(1LL<<5),
	-(1LL<<6),
	-(1LL<<7),
	-(1LL<<8),
	-(1LL<<9),
	-(1LL<<10),
	-(1LL<<11),
	-(1LL<<12),
	-(1LL<<13),
	-(1LL<<14),
	-(1LL<<15),
	-(1LL<<16),
	-(1LL<<17),
	-(1LL<<18),
	-(1LL<<19),
	-(1LL<<20),
	-(1LL<<21),
	-(1LL<<22),
	-(1LL<<23),
	-(1LL<<24),
	-(1LL<<25),
	-(1LL<<26),
	-(1LL<<27),
	-(1LL<<28),
	-(1LL<<29),
	-(1LL<<30),
	-(1LL<<31),
	-(1LL<<32),
	-(1LL<<33),
	-(1LL<<34),
	-(1LL<<35),
	-(1LL<<36),
	-(1LL<<37),
	-(1LL<<38),
	-(1LL<<39),
	-(1LL<<40),
	-(1LL<<41),
	-(1LL<<42),
	-(1LL<<43),
	-(1LL<<44),
	-(1LL<<45),
	-(1LL<<46),
	-(1LL<<47),
	-(1LL<<48),
	-(1LL<<49),
	-(1LL<<50),
	-(1LL<<51),
	-(1LL<<52),
	-(1LL<<53),
	-(1LL<<54),
	-(1LL<<55),
	-(1LL<<56),
	-(1LL<<57),
	-(1LL<<58),
	-(1LL<<59),
	-(1LL<<60),
	-(1LL<<61),
	-(1LL<<62),

	INT64_MAX-1,
	INT64_MAX,
};

uint64_t U64_Values[] = {
	0,
	1,
	1ULL<<1,
	1ULL<<2,
	1ULL<<3,
	1ULL<<4,
	1ULL<<5,
	1ULL<<6,
	1ULL<<7,
	1ULL<<8,
	1ULL<<9,
	1ULL<<10,
	1ULL<<11,
	1ULL<<12,
	1ULL<<13,
	1ULL<<14,
	1ULL<<15,
	1ULL<<16,
	1ULL<<17,
	1ULL<<18,
	1ULL<<19,
	1ULL<<20,
	1ULL<<21,
	1ULL<<22,
	1ULL<<23,
	1ULL<<24,
	1ULL<<25,
	1ULL<<26,
	1ULL<<27,
	1ULL<<28,
	1ULL<<29,
	1ULL<<30,
	1ULL<<31,
	1ULL<<32,
	1ULL<<33,
	1ULL<<34,
	1ULL<<35,
	1ULL<<36,
	1ULL<<37,
	1ULL<<38,
	1ULL<<39,
	1ULL<<40,
	1ULL<<41,
	1ULL<<42,
	1ULL<<43,
	1ULL<<44,
	1ULL<<45,
	1ULL<<46,
	1ULL<<47,
	1ULL<<48,
	1ULL<<49,
	1ULL<<50,
	1ULL<<51,
	1ULL<<52,
	1ULL<<53,
	1ULL<<54,
	1ULL<<55,
	1ULL<<56,
	1ULL<<57,
	1ULL<<58,
	1ULL<<59,
	1ULL<<60,
	1ULL<<61,
	1ULL<<62,
	1ULL<<63,
	UINT64_MAX-1,
	UINT64_MAX,
	0xdeadbeefdeadbeef,
	0xfeedfeedfeedfeed,
	0xfeedfeedfeed,
	0xfeedfeed,
	0xfeed
};


void fill_1(TypeTest &tt)
{
	for (int32_t i32 = INT32_MIN; i32 < INT32_MIN+5; ++i32) {
		tt.add_fi32(i32);
	}
	for (int32_t i32 = -5; i32 < 5; ++i32) {
		tt.add_fi32(i32);
	}
}


void fill_2(TypeTest &tt)
{
	for (int32_t i32 = INT32_MIN; i32 < INT32_MIN+5; ++i32) {
		tt.add_i32(i32);
	}
	for (int32_t i32 = -5; i32 < 5; ++i32) {
		tt.add_i32(i32);
	}
}


void fill_3(TypeTest &tt)
{
	for (int32_t i32 = INT32_MIN; i32 < INT32_MIN+5; ++i32) {
		tt.add_si32(i32);
	}
	for (int32_t i32 = -5; i32 < 5; ++i32) {
		tt.add_si32(i32);
	}
}


void fill_4(TypeTest &tt)
{
	for (int64_t i64 = INT64_MIN; i64 < INT64_MIN+5; ++i64) {
		tt.add_i64(i64);
	}
	for (int64_t i64 = -5; i64 < 5; ++i64) {
		tt.add_i64(i64);
	}
	for (unsigned x = 0; x < sizeof(I64_Values)/sizeof(int64_t); ++x)
		tt.add_i64(I64_Values[x]);
}


void fill_5(TypeTest &tt)
{
	for (int64_t i64 = INT64_MIN; i64 < INT64_MIN+5; ++i64) {
		tt.add_si64(i64);
	}
	for (int64_t i64 = -5; i64 < 5; ++i64) {
		tt.add_si64(i64);
	}
	for (unsigned x = 0; x < sizeof(I64_Values)/sizeof(int64_t); ++x)
		tt.add_i64(I64_Values[x]);
	for (unsigned x = 0; x < sizeof(U64_Values)/sizeof(uint64_t); ++x)
		tt.add_u64(U64_Values[x]);
	for (unsigned x = 0; x < sizeof(U64_Values)/sizeof(uint64_t); ++x)
		tt.add_pu64(U64_Values[x]);
}


void fill_6a(TypeTest &tt)
{
	tt.add_num(two);
	tt.add_num(three);
	tt.add_num(four);
}


void fill_6b(TypeTest &tt)
{
	tt.add_pnum(two);
	tt.add_pnum(three);
	tt.add_pnum(four);
}


void fill_7(TypeTest &tt)
{
	for (unsigned x = 0; x < sizeof(F32_Values)/sizeof(float); ++x)
		tt.add_f32(F32_Values[x]);
}


void fill_8(TypeTest &tt)
{
	for (unsigned x = 0; x < sizeof(F32_Values)/sizeof(float); ++x)
		tt.add_pf32(F32_Values[x]);
}


void fill_9(TypeTest &tt)
{
	for (unsigned x = 0; x < sizeof(F64_Values)/sizeof(double); ++x)
		tt.add_f64(F64_Values[x]);
}


void fill_10(TypeTest &tt)
{
	for (unsigned x = 0; x < sizeof(F64_Values)/sizeof(double); ++x)
		tt.add_pf64(F64_Values[x]);
}


int main(int argc, const char *argv[])
{
	TypeTest tt;
	
	cout << hex;
	fill_1(tt);
	runcheck(tt);
	fill_2(tt);
	runcheck(tt);
	fill_3(tt);
	runcheck(tt);
	fill_4(tt);
	runcheck(tt);
	fill_5(tt);
	runcheck(tt);
	fill_6a(tt);
	runcheck(tt);
	fill_6b(tt);
	runcheck(tt);
	fill_7(tt);
	runcheck(tt);
	fill_8(tt);
	runcheck(tt);
	fill_9(tt);
	runcheck(tt);
	fill_10(tt);
	runcheck(tt);
	printf("%s: %s\n",argv[0],testcnt());
}

