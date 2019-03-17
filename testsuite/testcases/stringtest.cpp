#include <string>
#include "stringtypes.h"
#include "runcheck.h"
#include "runcheck.cpp"
#include <stdio.h>

using namespace std;


int main(int argc, const char *argv[])
{
	StringTypes st;
	runcheck(st);
	st.set_s1("s1");
	st.set_s2("s2");
	st.set_s3("s3");
	//st.toASCII(cout);
	runcheck(st);

	printf("%s: %s\n",argv[0],testcnt());
}
