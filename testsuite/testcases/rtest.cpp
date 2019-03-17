#include "recursion.h"
#include <stdio.h>
#include <iostream>

using namespace std;

#include "runcheck.h"
#include "runcheck.cpp"

int main(int argc, char **argv)
{
	Directory root;
	root.set_name("root");
	runcheck(root);

	Directory *d = root.add_subdirs();
	d->set_name("subdir");
	runcheck(root);

	printf("%s: %s\n",argv[0],testcnt());
}
