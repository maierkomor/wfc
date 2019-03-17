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

#ifndef CSTRING_H
#define CSTRING_H

#include <string.h>

#ifdef __AVR__
#include <avr/pgmspace.h>

struct CString
{
	CString()
	: str(0)
	, len(0)
	, pgm_space(false)
	{ }

	CString(const char *d, bool p)
	: str(d)
	, len(d ? strlen_P(d) + 1 : 0)
	, pgm_space(p)
	{ }

	CString(const char *d, size_t l, bool p)
	: str(d)
	, len(l)
	, pgm_space(p)
	{ }

	void assign(const char *d)
	{
		str = d;
		if (d) {
			while (*d++);
			len = d-str;
		} else
			len = 0;
		pgm_space = false;
	}

	void assign_p(const char *d)
	{
		str = d;
		if (d) {
			while (pgm_read_byte(d++));
			len = d-str;
		} else
			len = 0;
		pgm_space = true;
	}

	void assign(const char *d, size_t l)
	{
		str = d;
		len = l;
		pgm_space = false;
	}

	void assign_p(const char *d, size_t l)
	{
		str = d;
		len = l;
		pgm_space = true;
	}

	// works only for strings initialized with terminating \0
	const char *c_str() const
	{
		return str;
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

	CString &operator = (const char *s)
	{
		assign(s);
		return *this;
	}

	void toWire(void (*put)(unsigned char)) const
	{
		for (size_t i = 0; i < len; ++i)
			put(pgm_read_byte(str+i));

	}

	bool inPgmSpace() const
	{ return pgm_space; }

	private:
	const char *str;
	size_t len;
	bool pgm_space;
};

#else

struct CString
{
	CString()
	: str(0)
	, len(0)
	{ }

	CString(const char *d)
	: str(d)
	, len(d ? strlen(d) + 1 : 0)
	{ }

	CString(const char *d, size_t l)
	: str(d)
	, len(l)
	{ }

	void assign(const char *d)
	{
		str = d;
		if (d) {
			len = strlen(d) + 1;
		} else {
			len = 0;
		}
	}

	void assign(const char *d, size_t l)
	{
		if (l && d && d[l-1] == 0) {
			str = d;
			len = l;
		} else {
			str = 0;
			len = 0;
		}
	}

	// works only for strings initialized with terminating \0
	const char *c_str() const
	{
		if (str && (str[len-1] == 0))
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

	CString &operator = (const char *s)
	{
		assign(s);
		return *this;
	}

	void toWire(void (*put)(unsigned char)) const
	{
		for (size_t i = 0; i < len; ++i)
			put(str[i]);
	}

	private:
	const char *str;
	size_t len;
};

#endif
#endif
