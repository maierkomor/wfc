/*
 *  Copyright (C) 2017-2018, Thomas Maier-Komor
 *
 *  This source file belongs to Wire-Format-Compiler.
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include <iostream>
#include <sstream>

#include "wirefuncs.h"
#include "log.h"

using namespace std;


typedef uint64_t varint_t;


static bool isAscii(const uint8_t *data, size_t size)
{
	if (size == 0)
		return false;
	while (size > 1) {
		uint8_t c = *data++;
		--size;
		if (c & 0x80)
			return false;
		if (c < 0x20) {
			if ((c != '\n') && (c != '\t') && (c != '\r') && (c != '\f'))
				return false;
		}
	}
	return true;
}


static void dumpAscii(std::ostream &o, const char *data, size_t size)
{
	assert(size != 0);
	uint64_t off = 0;
	char line[80];
	line[0] = '\t';
	line[0] = '"';
	int l = sprintf(line,"\t\"");
	while (off < size) {
		char c = *data++;
		++off;
		if ((c < 0x20) || (c == 0x7f)) {
			line[l++] = '\\';
			switch (c) {
			case '\n': line[l++] = 'n'; break;
			case '\r': line[l++] = 'r'; break;
			case '\b': line[l++] = 'b'; break;
			case '\t': line[l++] = 't'; break;
			case '\f': line[l++] = 'f'; break;
			case '\0': line[l++] = '0'; break;
			default:
				l += sprintf(line+l,"0%o",c);
			}
		} else {
			line[l++] = c;
		}
		if (l >= 70) {
			line[l++] = '"';
			line[l++] = '\n';
			line[l] = 0;
			o << line;
			l = sprintf(line,"\t\"");
		}
	}
	line[l++] = '"';
	line[l++] = '\n';
	line[l] = 0;
	o << line;
}


static int decodeBuffer(ostream &out, const uint8_t *buffer, ssize_t s, unsigned indent = 0)
{
	char ind[64] = "";
	assert(indent < sizeof(ind));
	ind[indent] = 0;
	for (unsigned u = 0; u < indent; ++u)
		ind[u] = '\t';
	const uint8_t *buf = buffer;
	const uint8_t *end = buf + s;
	out << ind << "block of size " << dec << s << endl;
	while (buf < end) {
		uint64_t fid;
		int n = read_varint(buf,end-buf,&fid);
		out << ind << "@" << dec << (buf-buffer) << ": tag = 0x" << hex << fid << " fid = " << dec << (fid >> 3) << ", type = 0x" << hex << (fid & 7UL) << ": ";
		buf += n;
		switch (fid & 7) {
		case 0:	{
			uint64_t v;
			n = read_varint(buf,end-buf,&v);
			if (n == 0) {
				out << "no varint at 0x" << hex << (buf-(end-s)) << '\n';
				return buf-(end-s);
			}
			buf += n;
			out << "varint 0x" << hex << v << '\n';
			} break;
		case 1: {
			uint64_t v;
			n = read_u64(buf,end-buf,&v);
			if (n == 0) {
				out << "no 64bit at 0x" << hex << (buf-(end-s)) << '\n';
				return buf-(end-s);
			}
			buf += n;
			out << "64bit 0x" << hex << v << '\n';
			} break;
		case 2: {
			uint64_t v;
			n = read_varint(buf,end-buf,&v);
			if (n == 0) {
				out << "no varint at 0x" << hex << (buf-(end-s)) << '\n';
				return buf-(end-s);
			}
			buf += n;
			if ((ssize_t)v > (end-buf)) {
				out << "array size at 0x" << hex << (buf-(end-s)) << " has length " << dec << v << " beyond end of buffer (" << (end-buf) << ")\n";
				return buf-(end-s);
			}
			if (isAscii(buf,v)) {
				out << "ascii string length " << dec << v << endl;
				dumpAscii(out,(const char *)buf,v);
			} else {
				stringstream ss;
				n = decodeBuffer(ss,buf,v,indent+1);
				if ((ssize_t)v == n)
					out << ind << "message length " << dec << v << endl << ss.str();
				else
					out << ind << "binary data length " << dec << v << endl;
			}
			buf += v;
			} break;
		case 3: {
				uint8_t b = *buf++;
				out << "8bit 0x" << hex << (uint32_t)b << '\n';
			} break;
		case 4: {
				uint16_t w;
				read_u16(buf,end-buf,&w);
				buf += 2;
				out << "16bit 0x" << hex << (uint32_t)w << '\n';

			} break;
		case 5: {
			uint32_t v;
			n = read_u32(buf,end-buf,&v);
			if (n == 0) {
				out << "no 32bit at 0x" << hex << (buf-(end-s)) << '\n';
				return buf-(end-s);
			}
			buf += n;
			out << "32bit 0x" << hex << v << '\n';
			} break;
		default:
			out << "invalid type\n";
			return buf-(end-s);
		}
	}
	return buf-(end-s);
}


void decodeFile(const char *fn)
{
	int fd = open(fn,O_RDONLY);
	if (fd == -1)
		fatal("unable to open input file %s: %s\n",fn,strerror(errno));
	struct stat st;
	int r = fstat(fd,&st);
	if (r == -1)
		fatal("unable to stat input file %s: %s\n",fn,strerror(errno));
	uint8_t *buf = (uint8_t *)malloc(st.st_size);
	ssize_t n = read(fd,buf,st.st_size);
	close(fd);
	if (n == -1)
		fatal("unable to read input file %s: %s\n",fn,strerror(errno));
	r = decodeBuffer(cout,buf,n);
	free(buf);
	exit(r == 0 ? EXIT_SUCCESS : EXIT_FAILURE);
}


