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

struct Bytes
{
	Bytes()
	: bytes(0)
	, len(0)
	{ }

	Bytes(const char *d, size_t l)
	: bytes(d)
	, len(l)
	{ }

	void assign(const char *d, size_t l)
	{
		bytes = d;
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
		bytes = 0;
		len = 0;
	}

	bool operator != (const Bytes &r) const
	{ return (len != r.len) || (0 != memcmp(bytes,r.bytes,len)); }

	private:
	const char *bytes;
	size_t len;
};

#endif
