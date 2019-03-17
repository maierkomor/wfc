#include <string>
#include "libtest1.h"
#include <stdio.h>
#include "runcheck.h"
#include "runcheck.cpp"

using namespace std;


int main(int argc, const char *argv[])
{
	msg m;
	m.set_vi1(100000);
	m.set_vi2(-17);
	runcheck(m);

	printf("%s: %s\n",argv[0],testcnt());
}
