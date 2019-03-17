#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <math.h>

double read_double0(const uint8_t *wire)
{
	union { uint64_t i; double d; } r;
	r.i = (uint64_t)*wire++;
	r.i |= (uint64_t)*wire++ << 8;
	r.i |= (uint64_t)*wire++ << 16;
	r.i |= (uint64_t)*wire++ << 24;
	r.i |= (uint64_t)*wire++ << 32;
	r.i |= (uint64_t)*wire++ << 40;
	r.i |= (uint64_t)*wire++ << 48;
	r.i |= (uint64_t)*wire << 56;
	return r.d;
}

double read_double1(const uint8_t *wire)
{
	union { uint64_t i; double d; } r;
	r.i = 0;
	for (int i = 0 ; i < 64; i+=8)
		r.i |= (uint64_t)*wire++ << i;
	return r.d;
}


double read_double2(const uint8_t *wire)
{
	union { uint64_t i; double d; } r;
	r.i	= (uint64_t)wire[0]
		| (uint64_t)wire[1] << 8
		| (uint64_t)wire[2] << 16
		| (uint64_t)wire[3] << 24
		| (uint64_t)wire[4] << 32
		| (uint64_t)wire[5] << 40
		| (uint64_t)wire[6] << 48
		| (uint64_t)wire[7] << 56;
	return r.d;
}


int main()
{
	uint8_t wire[8];
	struct timeval s,e;
	volatile double d = M_PI;
	memcpy(wire,(const void *)&d,sizeof(double));

	printf("0: %g, 1: %g, 2: %g, 3: %g\n",read_double0(wire), read_double1(wire), read_double2(wire), *(double*)wire);
	assert((read_double0(wire) == read_double1(wire)) && (read_double1(wire) == read_double2(wire)));
	assert(read_double0(wire) == *(double*)wire);

	gettimeofday(&s,0);
	for (int i = 0; i < 1000000; ++i)
		d = read_double0(wire);
	gettimeofday(&e,0);
	printf("0: %lld\n",(e.tv_sec-s.tv_sec)*1000000+(e.tv_usec-s.tv_usec));

	gettimeofday(&s,0);
	for (int i = 0; i < 1000000; ++i)
		d = read_double1(wire);
	gettimeofday(&e,0);
	printf("1: %lld\n",(e.tv_sec-s.tv_sec)*1000000+(e.tv_usec-s.tv_usec));

	gettimeofday(&s,0);
	for (int i = 0; i < 1000000; ++i)
		d = read_double2(wire);
	gettimeofday(&e,0);
	printf("2: %lld\n",(e.tv_sec-s.tv_sec)*1000000+(e.tv_usec-s.tv_usec));

	gettimeofday(&s,0);
	for (int i = 0; i < 1000000; ++i)
		d = *(double*)(wire);
	gettimeofday(&e,0);
	printf("3: %lld\n",(e.tv_sec-s.tv_sec)*1000000+(e.tv_usec-s.tv_usec));
}
	

