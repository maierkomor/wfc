/*
 * This is example source code for use of WFC.
 *
 * Feel free to use and modify as you like.
 */

#include "letter_r.h"

#include <assert.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>

#include <iostream>

#ifndef O_BINARY
#define O_BINARY 0
#endif

using namespace std;

int main()
{
	int fd = open("myletter.bin",O_RDONLY|O_BINARY);
	assert(fd != -1);

	struct stat st;
	int r = fstat(fd,&st);
	assert(r != -1);

	char *data = (char*)malloc(st.st_size);

	int n = read(fd,data,st.st_size);
	assert(n == st.st_size);

	letter l;
	ssize_t p = l.fromMemory(data,st.st_size);
	cout << "parsed " << p << " bytes" << endl;

	cout << "form: " << l.sender() << endl;
	cout << "to: " << l.receiver() << endl;
	cout << "text: " << l.text() << endl;
}
