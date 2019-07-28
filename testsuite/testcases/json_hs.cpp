#include "hostscope.h"
#include "runcheck.h"

#include <errno.h>
#include <sstream>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>


struct fbuf_t
{
	fbuf_t(char *d = 0, size_t s = 0)
	: data(d)
	, size(s)
	{ }

	char *data;
	ssize_t size;
};

fbuf_t readFile(const char *fn)
{
	int fd = open(fn,O_RDONLY);
	if (fd == -1) {
		printf("unable to open file %s: %s",fn,strerror(errno));
		return fbuf_t();
	}
	struct stat st;
	if (-1 == fstat(fd,&st)) {
		close(fd);
		printf("unable to stat file %s: %s",fn,strerror(errno));
		return fbuf_t();
	}
	char *buf = (char *)malloc(st.st_size+1);
	buf[st.st_size] = 0;
	int n = read(fd,buf,st.st_size);
	close(fd);
	if (n == -1) {
		printf("error while reading file %s: %s",fn,strerror(errno));
		free(buf);
		return fbuf_t();
	}
	return fbuf_t(buf,st.st_size);
}


int main()
{
	fbuf_t b;
	b = readFile("hs1.bin");
	HostScope h0;
	int r = h0.fromMemory(b.data,b.size);
	assert(r == b.size);

	stringstream json;
	h0.toJSON(json);
	printf("%s",json.str().c_str());
	return 0;
}
