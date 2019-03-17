#include "hostscope.h"
#include "runcheck.h"
#include "runcheck.cpp"

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>


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
		printf("unable to open file %s: %s\n",fn,strerror(errno));
		return fbuf_t();
	}
	struct stat st;
	if (-1 == fstat(fd,&st)) {
		close(fd);
		printf("unable to stat file %s: %s\n",fn,strerror(errno));
		return fbuf_t();
	}
	// 15 additional bytes to test terminator byte
	char *buf = (char *)malloc(st.st_size+16);
	buf[st.st_size] = 0xff;
	int n = read(fd,buf,st.st_size);
	close(fd);
	if (n == -1) {
		printf("error while reading file %s: %s\n",fn,strerror(errno));
		free(buf);
		return fbuf_t();
	}
	return fbuf_t(buf,st.st_size);
}


unsigned runtest(char *b, size_t s, uint8_t x, size_t off)
{
	char z = b[off];
	b[off] = x;
	HostScope h;
#ifdef ON_ERROR_CANCEL
	unsigned r = h.fromMemory(b,s);
#elif defined ON_ERROR_THROW
	unsigned r;
	try {
		r = h.fromMemory(b,s);
		abort();
	} catch (int x) {
		return x;
	}
#endif
	b[off] = z;
	return r;
}


int main(int argc, char **argv)
{
	uint8_t xbytes[] = {0x00, 0xff, 0x88, 0x44, 0xc3, 0x3c};
	fbuf_t b;
	b = readFile("hs1.bin");
	HostScope h0;
	int r = h0.fromMemory(b.data,b.size+16);
	assert(r == b.size);
	unsigned zero = 0, ok = 0, truncated = 0, error = 0;
	runtest(b.data,b.size,0x00,799);
	for (size_t x = 0; x < sizeof(xbytes); ++x) {
		//printf("xbyte 0x%hhx\n",xbytes[x]);
		for (ssize_t i = 0; i < b.size; ++i) {
			char z = b.data[i];
			b.data[i] = xbytes[x];
#ifdef ON_ERROR_CANCEL
			r = h0.fromMemory(b.data,b.size);
#elif defined ON_ERROR_THROW
			try {
				r = h0.fromMemory(b.data,b.size);
			} catch (int x) {
				r = x;
			}
#endif
			if (r == 0)
				++zero;
			else if (r < 0)
				++error;
			else if (r == b.size)
				++ok;
			else if (r < b.size)
				++truncated;
			else
				abort();
			b.data[i] = z;
		}
	}
	printf("%s: %u ok, %u error, %u zero, truncated %u\n",argv[0],ok,error,zero,truncated);
	printf("%s: %s\n",argv[0],testcnt());
}
