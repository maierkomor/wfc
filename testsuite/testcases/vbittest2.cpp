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

#include "validbits2.h"

#include <sink.h>
#include "runcheck.h"
#include "runcheck.cpp"

using namespace std;


int main(int argc, char *argv[])
{
	M tb,fw;

	tb.clear();
	if (tb != fw)
		fail(&tb,&fw);

	tb.set_s1e("s1e");
	assert(tb.has_s1e());
	tb.clear_s1e();
	assert(!tb.has_s1e());
	tb.set_s1e("s1e");
	runcheck(tb);

	tb.set_s1i("s1i");
	assert(tb.has_s1i());
	tb.clear_s1i();
	assert(!tb.has_s1i());
	tb.set_s1i("s1i");
	runcheck(tb);

	tb.set_s2e("s2e");
	assert(tb.has_s2e());
	tb.clear_s2e();
	assert(!tb.has_s2e());
	tb.set_s2e("s2e");
	runcheck(tb);

	tb.set_s3e("s3e");
	assert(tb.has_s3e());
	tb.clear_s3e();
	assert(!tb.has_s3e());
	tb.set_s3e("s3e");
	runcheck(tb);

	tb.set_s3i("s3i");
	assert(tb.has_s3i());
	tb.clear_s3i();
	assert(!tb.has_s3i());
	tb.set_s3i("s3i");
	runcheck(tb);

	tb.set_b1e("b1e",4);
	assert(tb.has_b1e());
	tb.clear_b1e();
	assert(!tb.has_b1e());
	tb.set_b1e("b1e",4);
	runcheck(tb);

	tb.set_b1i("b1i",4);
	assert(tb.has_b1i());
	tb.clear_b1i();
	assert(!tb.has_b1i());
	tb.set_b1i("b1i",4);
	runcheck(tb);

	tb.set_b2e("b2e",4);
	assert(tb.has_b2e());
	tb.clear_b2e();
	assert(!tb.has_b2e());
	tb.set_b2e("b2e",4);
	runcheck(tb);

	tb.set_b2i("b2i",4);
	assert(tb.has_b2i());
	tb.clear_b2i();
	assert(!tb.has_b2i());
	tb.set_b2i("b2i",4);
	runcheck(tb);

	tb.set_e1e(forefinger);
	assert(tb.has_e1e());
	tb.clear_e1e();
	assert(!tb.has_e1e());
	tb.set_e1e(forefinger);
	runcheck(tb);

	tb.set_e1i(thumb);
	assert(tb.has_e1i());
	tb.clear_e1i();
	assert(!tb.has_e1i());
	tb.set_e1i(thumb);
	runcheck(tb);

	tb.set_f1e(M_PI);
	assert(tb.has_f1e());
	tb.clear_f1e();
	assert(!tb.has_f1e());
	tb.set_f1e(M_PI);
	runcheck(tb);

	tb.set_f1i(M_PI);
	assert(tb.has_f1i());
	tb.clear_f1i();
	assert(!tb.has_f1i());
	tb.set_f1i(M_PI);
	runcheck(tb);

	tb.set_d1e(M_PI);
	assert(tb.has_d1e());
	tb.clear_d1e();
	assert(!tb.has_d1e());
	tb.set_d1e(M_PI);
	runcheck(tb);

	tb.set_d1i(M_PI);
	assert(tb.has_d1i());
	tb.clear_d1i();
	assert(!tb.has_d1i());
	tb.set_d1i(M_PI);
	runcheck(tb);

	printf("%s: %s\n",argv[0],testcnt());
}

