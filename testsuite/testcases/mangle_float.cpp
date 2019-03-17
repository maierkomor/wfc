#include <stdint.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>

float F32;
double F64;


union float_to_uint32
{
	explicit float_to_uint32(float f)
	: f32(f)
	{ }

	operator uint32_t () const
	{ return u32; }

	float f32;
	uint32_t u32;
};

void to_mem(uint8_t *mem, uint32_t src)
{
	*(uint32_t*)mem = src;
}


void usage0(uint8_t *m, float v)
{
	to_mem(m,*(uint32_t*)&v);
}

void usage1(uint8_t *m, float v)
{
	union float_to_uint32 f2u(v);
	//f2u.f32=v;
	to_mem(m,f2u.u32);
}

void usage2(uint8_t *m, float v)
{
	to_mem(m,float_to_uint32(v));
}


int main()
{
	uint8_t buf0[4];
	uint8_t buf1[4];
	uint8_t buf2[4];
	int r;

	usage0(buf0,1.3);
	usage1(buf1,1.3);
	usage2(buf2,1.3);

	r = memcmp(buf0,buf1,sizeof(buf0));
	assert(r == 0);
	r = memcmp(buf0,buf2,sizeof(buf0));
	assert(r == 0);
	printf("mangle_float: OK\n");
}
