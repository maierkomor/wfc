#include <string>
#include "fixed_only.h"
#include <stdio.h>
#include "runcheck.h"
#include "runcheck.cpp"

#ifdef SUBCLASSES
#define EnumTest_A EnumTest::A
#define EnumTest_B EnumTest::B
#define fluid EnumTest::A::fluid
#define frozen EnumTest::A::frozen
#define ice EnumTest::B::ice
#endif

using namespace std;


int main(int argc, const char *argv[])
{
	Fixed f;
	f.set_f8(0);
	f.set_sf8(0);
	f.set_f16(0);
	f.set_sf16(0);
	f.set_f32(0);
	f.set_sf32(0);
	f.set_f64(0);
	f.set_sf64(0);
	runcheck(f);
	f.set_f8(UINT8_MAX);
	f.set_sf8(INT8_MAX);
	f.set_f16(UINT16_MAX);
	f.set_sf16(INT16_MAX);
	f.set_f32(UINT32_MAX);
	f.set_sf32(INT32_MAX);
	f.set_f64(UINT64_MAX);
	f.set_sf64(INT64_MAX);
	runcheck(f);
	f.set_f8(0);
	f.set_sf8(INT8_MIN);
	f.set_f16(0);
	f.set_sf16(INT16_MIN);
	f.set_f32(0);
	f.set_sf32(INT32_MIN);
	f.set_f64(0);
	f.set_sf64(INT64_MIN);
	runcheck(f);
	f.set_f8(UINT8_MAX);
	f.set_sf8(INT8_MIN);
	f.set_f16(UINT16_MAX);
	f.set_sf16(INT16_MIN);
	f.set_f32(UINT32_MAX);
	f.set_sf32(INT32_MIN);
	f.set_f64(UINT64_MAX);
	f.set_sf64(INT64_MIN);
	runcheck(f);

	f.add_rf8(0);
	runcheck(f);
	f.add_rf8(UINT8_MAX);
	f.add_rsf8(0);
	f.add_rsf8(INT8_MAX);
	f.add_rsf8(INT8_MIN);
	f.add_rf16(UINT16_MAX);
	f.add_rf16(0);
	f.add_rsf16(INT16_MAX);
	runcheck(f);
	f.add_rsf16(0);
	f.add_rsf16(INT16_MIN);
	f.add_rf32(UINT32_MAX);
	f.add_rsf32(INT32_MAX);
	f.add_rsf32(0);
	f.add_rsf32(INT32_MIN);
	runcheck(f);
	f.add_rf64(UINT64_MAX);
	f.add_rf64(0);
	f.add_rsf64(INT64_MAX);
	f.add_rsf64(INT64_MIN);
	runcheck(f);

	printf("%s: %s\n",argv[0],testcnt());
}
