#include "packed.h"
#include "runcheck.h"
#include "runcheck.cpp"

uint8_t test_f16_err_1[] = {
	0xa, 1, 1
};

uint8_t test_f16_err_2[] = {
	0xa, 3, 1,2,3,
};


uint8_t test_f32_err_1[] = {
	0x12, 3, 1,2,3
};

uint8_t test_f32_err_2[] = {
	0x12, 5, 1,2,3,4,5
};


uint8_t test_f64_err_1[] = {
	0x1a, 7, 1,2,3,4,5,6,7
};

uint8_t test_f64_err_2[] = {
	0x1a, 9, 1,2,3,4,5,6,7,8,9
};




int main()
{
	M m;
	runcheck(m);
	m.add_F16(0);
	m.add_F16(UINT16_MAX);
	runcheck(m);
	m.add_F32(0);
	m.add_F32(UINT32_MAX);
	runcheck(m);
	m.add_F64(0);
	m.add_F64(UINT64_MAX);
	runcheck(m);
	m.clear();
	ssize_t r;
	r = m.fromMemory(test_f16_err_1,sizeof(test_f16_err_1));
	assert(r < 0);
	r = m.fromMemory(test_f32_err_1,sizeof(test_f32_err_1));
	assert(r < 0);
	r = m.fromMemory(test_f64_err_1,sizeof(test_f64_err_1));
	assert(r < 0);
	r = m.fromMemory(test_f16_err_2,sizeof(test_f16_err_2));
	assert(r < 0);
	r = m.fromMemory(test_f32_err_2,sizeof(test_f32_err_2));
	assert(r < 0);
	r = m.fromMemory(test_f64_err_2,sizeof(test_f64_err_2));
	assert(r < 0);
}
