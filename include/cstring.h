/*
 *  Copyright (C) 2017-2020, Thomas Maier-Komor
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

#ifndef _CSTRING_H
#define _CSTRING_H

#include <string.h>

// - string as pointer/length pair
// - terminating \0 is required for c_str() and pointer assignment,
//   but is otherise optional
// - access to location of \0 termination must be possible, even if it
//   is not there

struct CString
{
	CString()
	: str(0)
	, len(0)
	{ }

	CString(const char *d)
	: str(d)
	, len(d ? strlen(d) : 0)
	{ }

	CString(const char *d, size_t l)
	: str(d)
	, len(l)
	{ }

	void assign(const char *d)
	{
		str = d;
		if (d)
			len = strlen(d);
		else
			len = 0;
	}

	void assign(const char *d, size_t l)
	{
		str = d;
		len = l;
	}

	// works only for strings initialized with terminating \0
	const char *c_str() const
	{
		if (str && (str[len] == 0))
			return str;
		return 0;
	}

	const char *data() const
	{ return str; }

	bool empty() const
	{ return (len == 0); }

	size_t size() const
	{ return len; }

	void clear()
	{ str = 0; len = 0; }

	bool operator == (const CString &r) const
	{ return (len == r.len) && (0 == memcmp(str,r.str,len)); }

	bool operator != (const CString &r) const
	{ return (len != r.len) || (0 != memcmp(str,r.str,len)); }

	bool operator == (const char *r) const
	{ return (len == strlen(r)) && (0 == memcmp(str,r,len)); }

	bool operator != (const char *r) const
	{ return (len != strlen(r)) || (0 != memcmp(str,r,len)); }

	CString &operator = (const char *s)
	{
		assign(s);
		return *this;
	}

	void toWire(void (*put)(unsigned char)) const
	{
		const char *at = str;
		const char *e = at + len;
		while (at != e) {
			put(*at);
			++at;
		}
	}


	private:
	const char *str;
	size_t len;
};


#endif
