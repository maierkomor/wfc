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

#ifndef _BYTES_H
#define _BYTES_H

#include <inttypes.h>
#include <stdlib.h>

struct Bytes
{
	Bytes()
	: bytes(0)
	, len(0)
	{ }

	explicit Bytes(const char *d)
	: len(strlen(d))
	{
		bytes = (char *)malloc(len);
		memcpy(bytes,d,len);
	}

	Bytes(const char *d, size_t l)
	: bytes((char *)malloc(l))
	, len(l)
	{ memcpy(bytes,d,l); }

	Bytes(const Bytes &b)
	: bytes((char *)malloc(b.len))
	, len(b.len)
	{ memcpy(bytes,b.bytes,len); }
	
	~Bytes()
	{ free(bytes); }

	Bytes &operator = (const Bytes &b)
	{
		assign(b.bytes,b.len);
		return *this;
	}
	
	Bytes &operator = (const char *cstr)
	{
		size_t l = strlen(cstr);
		assign(cstr,l);
		return *this;
	}
	
	void assign(const char *d, size_t l)
	{
		bytes = (char*)realloc((void*)bytes,l);
		memcpy(bytes,d,l);
		len = l;
	}

	const char *data() const
	{ return bytes; }

	bool empty() const
	{ return (bytes == 0) || (len == 0); }

	size_t size() const
	{ return len; }

	void clear()
	{
		free((void*)bytes);
		bytes = 0;
		len = 0;
	}

	void push_back(char c)
	{
		size_t at = len;
		++len;
		bytes = (char *) realloc((void*)bytes,len);
		bytes[at] = c;
	}

	bool operator != (const Bytes &r) const
	{ return (len != r.len) || (0 != memcmp(bytes,r.bytes,len)); }

	bool operator == (const Bytes &r) const
	{ return (len == r.len) && (0 == memcmp(bytes,r.bytes,len)); }

	bool operator == (const char *cstr) const
	{
		size_t l = strlen(cstr);
		return (l == len) && (0 == memcmp(bytes,cstr,l));
	}

	bool operator != (const char *cstr) const
	{
		size_t l = strlen(cstr);
		return (l != len) || (0 != memcmp(bytes,cstr,l));
	}

	private:
	char *bytes;
	size_t len;
};

#endif
