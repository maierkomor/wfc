#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <iostream>

#include <unistd.h>
#include "hs.h"

using namespace std;


int main()
{
	int fd = open("hostscope_msg.bin",O_RDONLY);
	assert(fd != -1);
	struct stat st;
	int r = fstat(fd,&st);
	assert(r != -1);
	uint8_t *buf = (uint8_t*)malloc(st.st_size);
	int n = read(fd,buf,st.st_size);
	HostScope h;
	h.ParseFromArray(buf,st.st_size);
	const HostScope_HostInfo &hi = h.hostinfo();
	cout << hi.hostname() << endl;
	abort();
}
