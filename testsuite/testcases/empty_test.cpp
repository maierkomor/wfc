#include "empty_elements.h"

#include "runcheck.cpp"

#include <stdio.h>


int main(int argc, char *argv[])
{
	Something S;

	assert(S.calcSize() == 0);
	runcheck(S);

	S.add_empty();
	Empty *e = S.add_empty();
	runcheck(S);

	e->set_i32(3);
	runcheck(S);

	S.add_empty();
	runcheck(S);
	printf("%s: %s\n",argv[0],testcnt());
}
