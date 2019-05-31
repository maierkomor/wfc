#include "reference.h"

#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <fcntl.h>
#include <assert.h>
#include <string.h>
#include <unistd.h>
#include <math.h>

#include <sink.h>
#include "runcheck.cpp"

#ifdef SUBCLASSES
#define TestBench_KVPair TestBench::KVPair 
#endif


uint8_t ValuesU8[] = {0,  1,  INT8_MAX, INT8_MAX+1, UINT8_MAX};
uint16_t ValuesU16[] = {0,  1,  INT16_MAX, INT16_MAX+1, UINT16_MAX};
uint32_t ValuesU32[] = {0,  1,  INT32_MAX, (uint32_t)INT32_MAX+1ULL, UINT32_MAX};
uint64_t ValuesU64[] = {0,  1,  INT64_MAX, (uint64_t)INT64_MAX+1ULL, UINT64_MAX};

int8_t ValuesS8[] = {0, -1, 1, -99, 99, INT8_MIN, INT8_MAX};
int16_t ValuesS16[] = {0, -1, 1, INT8_MIN, INT8_MAX, INT8_MIN-1, INT8_MAX+1, INT16_MIN, INT16_MAX};
int32_t ValuesS32[] = {0, -1, 1, INT16_MIN, INT16_MAX, INT16_MIN-1, INT16_MAX+1, INT32_MIN, INT32_MAX};
int64_t ValuesS64[] = {0, -1, 1, INT32_MIN, INT32_MAX, (int64_t)INT32_MIN-1, (int64_t)INT32_MAX+1, INT64_MIN, INT64_MAX};


int main(int argc, char *argv[])
{
	uint8_t data[] = {5,4,3,2,1,0,1,2,3,4,5};
	TestBench tb,tb2;
	tb2.clear();
	if (tb != tb2)
		fail(&tb,&tb2);
	runcheck(tb);

	tb.set_VI32(123456);
	runcheck(tb);
	tb.set_SVI32(-123456);
	runcheck(tb);
	tb.set_VI64(1234567890);
	runcheck(tb);
	tb.set_SVI64(-1234567890);
	runcheck(tb);
	tb.set_RDouble(M_PI);
	runcheck(tb);
	tb.set_FI8(88);
	runcheck(tb);
	tb.set_FI16(1616);
	runcheck(tb);
	tb.set_FI16(0xffff);
	runcheck(tb);
	tb.set_FI32(323232);
	runcheck(tb);
	tb.set_FI32(0xffffffff);
	runcheck(tb);
	tb.set_FI64(64646464646464);
	runcheck(tb);
	tb.set_STR("test string");
	runcheck(tb);
	tb.add_STRV("string0");
	runcheck(tb);
	tb.add_STRV("string1");
	runcheck(tb);
	tb.add_STRV("string2");
	runcheck(tb);
	tb.set_Float(M_PI_2);
	tb.set_B0(false);
	runcheck(tb);
	tb.set_B1(true);
	runcheck(tb);

	for (size_t i = 0; i < sizeof(ValuesU8)/sizeof(ValuesU8[0]); ++i)
		tb.add_PackedF8Vector(ValuesU8[i]);
	runcheck(tb);

	for (size_t i = 0; i < sizeof(ValuesU16)/sizeof(ValuesU16[0]); ++i)
		tb.add_PackedF16Vector(ValuesU16[i]);
	runcheck(tb);

	for (size_t i = 0; i < sizeof(ValuesU32)/sizeof(ValuesU32[0]); ++i)
		tb.add_PackedF32Vector(ValuesU32[i]);
	runcheck(tb);

	for (size_t i = 0; i < sizeof(ValuesU64)/sizeof(ValuesU64[0]); ++i)
		tb.add_PackedF64Vector(ValuesU64[i]);
	runcheck(tb);

	for (size_t i = 0; i < sizeof(ValuesS8)/sizeof(ValuesS8[0]); ++i)
		tb.add_PackedS8Vector(ValuesS8[i]);
	runcheck(tb);

	for (size_t i = 0; i < sizeof(ValuesS16)/sizeof(ValuesS16[0]); ++i)
		tb.add_PackedS16Vector(ValuesS16[i]);
	runcheck(tb);

	for (size_t i = 0; i < sizeof(ValuesS32)/sizeof(ValuesS32[0]); ++i)
		tb.add_PackedS32Vector(ValuesS32[i]);
	runcheck(tb);

	for (size_t i = 0; i < sizeof(ValuesS64)/sizeof(ValuesS64[0]); ++i)
		tb.add_PackedS64Vector(ValuesS64[i]);
	runcheck(tb);

	for (size_t i = 0; i < sizeof(ValuesS8)/sizeof(ValuesS8[0]); ++i) {
		tb.set_SI8(ValuesS8[i]);
		runcheck(tb);
	}

	for (size_t i = 0; i < sizeof(ValuesS16)/sizeof(ValuesS16[0]); ++i) {
		tb.set_SI16(ValuesS16[i]);
		runcheck(tb);
	}

	for (size_t i = 0; i < sizeof(ValuesS32)/sizeof(ValuesS32[0]); ++i) {
		tb.set_SI32(ValuesS32[i]);
		runcheck(tb);
	}

	for (size_t i = 0; i < sizeof(ValuesS64)/sizeof(ValuesS64[0]); ++i) {
		tb.set_SI64(ValuesS64[i]);
		runcheck(tb);
	}

	tb.mutable_PackedMsg()->set_U8(33);
	runcheck(tb);

	Packable *p = tb.add_PackedMsgV();
	p->set_U8(0);
	runcheck(tb);
	p->set_I8(0);
	runcheck(tb);

	FixedPackable *f = tb.add_FPackedMsgV();
	f->set_F8(0);
	runcheck(tb);

	tb.set_BYTESR(data,sizeof(data));
	runcheck(tb);

	tb.add_FloatV(M_PI);
	tb.add_FloatV(M_PI_2);
	runcheck(tb);

	tb.add_DoubleV(M_PI);
	tb.add_DoubleV(M_PI_2);
	runcheck(tb);

	tb.add_GenericEnumV(ge_1);
	tb.add_GenericEnumV(ge_2);
	tb.add_GenericEnumV(ge_3);
	tb.add_GenericEnumV(ge_4);
	runcheck(tb);

	tb.add_GenericEnumPV(ge_1);
	tb.add_GenericEnumPV(ge_2);
	tb.add_GenericEnumPV(ge_3);
	tb.add_GenericEnumPV(ge_4);
	runcheck(tb);

	assert(!tb.has_SSO());
	tb.set_SSO("optional string");
	assert(tb.has_SSO());
	tb.clear_SSO();
	assert(!tb.has_SSO());
	tb.set_SSO("optional string");
	tb.set_SSR("required string");
	tb.add_SSV("repeated string 0");
	tb.add_SSV("repeated string 1");
	runcheck(tb);

	assert(!tb.has_SCO());
	tb.set_SCO("optional string class");
	if (!tb.has_SCO())
		fail(&tb);
	tb.clear_SCO();
	assert(!tb.has_SCO());
	tb.set_SCO("optional string class");
	tb.set_SCR("required string class");
	tb.add_SCV("repeated string class 0");
	tb.add_SCV("repeated string class 1");
	runcheck(tb);

	assert(!tb.has_SPO());
	tb.set_SPO("optional char *");
	assert(tb.has_SPO());
	tb.clear_SPO();
	assert(!tb.has_SPO());
	tb.set_SPR("required char *");
	tb.add_SPV("repeated char * 0");
	tb.add_SPV("repeated char * 1");
	runcheck(tb);

	TestBench_KVPair kvp;
	kvp.set_key("key0");
	kvp.set_value("value0");
	tb.set_kvpair1(kvp);
	runcheck(tb);

	*tb.add_kvpairs() = kvp;
	runcheck(tb);

	kvp.set_key("key1");
	kvp.set_value("value1");
	tb.add_kvpairs();
	tb.set_kvpairs(1,kvp);
	runcheck(tb);

	printf("%s: %s\n",argv[0],testcnt());
}
