/*
 *  Copyright (C) 2019-2020, Thomas Maier-Komor
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

#ifndef _ASTRING_H
#define _ASTRING_H

#include <stdlib.h>
#include <string.h>

class AString
{
	public:
	AString()
	: str((char*)malloc(1))
	, len(0)
	{
		str[0] = 0;
	}

	AString(const char *s)
	: len(strlen(s))
	{
		str = (char *)malloc(len + 1);
		memcpy(str,s,len+1);
	}

	AString(const char *s, size_t l)
	: str((char*)malloc(l+1))
	, len(l)
	{
		memcpy(str,s,l);
		str[l] = 0;
	}

	AString(const AString &a)
	: str((char*)(malloc(a.len+1)))
	, len(a.len)
	{
		memcpy(str,a.str,len+1);
	}

	~AString()
	{
		free(str);
	}

	AString &operator = (const AString &a)
	{
		str = (char*) realloc(str,a.len+1);
		len = a.len;
		memcpy(str,a.str,len+1);
		return *this;
	}

	AString &operator += (const AString &a)
	{
		str = (char*) realloc(str,len+a.len+1);
		memcpy(str+len,a.str,a.len+1);
		len += a.len;
		return *this;
	}

	bool operator == (const AString &a) const
	{ return (len == a.len) && (0 == memcmp(str,a.str,len)); }

	bool operator != (const AString &a) const
	{ return (len != a.len) || (0 != memcmp(str,a.str,len)); }


	AString &operator = (const char *s)
	{
		len = strlen(s);
		str = (char*) realloc(str,len+1);
		memcpy(str,s,len+1);
		return *this;
	}

	AString &operator += (const char *s)
	{
		size_t al = strlen(s);
		str = (char*)realloc(str,len+al+1);
		memcpy(str+len,s,al+1);
		len += al;
		return *this;
	}

	AString &operator += (char c)
	{
		str = (char*)realloc(str,len+2);
		str[len] = c;
		str[++len] = 0;
		return *this;
	}

	bool operator == (const char *s) const
	{
		size_t l = strlen(s);
		return (len == l) && (0 == memcmp(str,s,l));
	}

	bool operator != (const char *a) const
	{ return !(*this == a); }


	const char *c_str() const
	{ return str; }

	const char *data() const
	{ return str; }

	void clear()
	{
		str[0] = 0;
		len = 0;
	}

	bool empty() const
	{
		return str[0] == 0;
	}

	void assign(const char *m, size_t s)
	{
		str = (char *) realloc(str,s+1);
		str[s] = 0;
		memcpy(str,m,s);
		len = s;
	}

	void append(const char *m, size_t s)
	{
		str = (char *) realloc(str,s+len+1);
		memcpy(str+len,m,s);
		len += s;
		str[len] = 0;
	}

	void push_back(char c)
	{
		str = (char *) realloc(str,len+2);
		str[len] = c;
		str[++len] = 0;
	}

	size_t size() const
	{ return len; }

	friend bool operator < (const AString &, const AString &);
	friend bool operator <= (const AString &, const AString &);
	friend bool operator > (const AString &, const AString &);
	friend bool operator >= (const AString &, const AString &);
	friend bool operator < (const AString &, const char *);
	friend bool operator <= (const AString &, const char *);
	friend bool operator > (const AString &, const char *);
	friend bool operator >= (const AString &, const char *);

	private:
	mutable char *str;
	size_t len;
};


inline bool operator < (const AString &l, const AString &r)
{
	const char *ls = l.str ? l.str : "";
	const char *rs = r.str ? r.str : "";
	return strcmp(ls,rs) < 0;
}


inline bool operator <= (const AString &l, const AString &r)
{
	const char *ls = l.str ? l.str : "";
	const char *rs = r.str ? r.str : "";
	return strcmp(ls,rs) <= 0;
}


inline bool operator < (const AString &l, const char *rs)
{
	const char *ls = l.str ? l.str : "";
	return strcmp(ls,rs) < 0;
}


inline bool operator <= (const AString &l, const char *rs)
{
	const char *ls = l.str ? l.str : "";
	return strcmp(ls,rs) <= 0;
}


inline bool operator > (const AString &l, const AString &r)
{
	const char *ls = l.str ? l.str : "";
	const char *rs = r.str ? r.str : "";
	return strcmp(ls,rs) > 0;
}


inline bool operator >= (const AString &l, const AString &r)
{
	const char *ls = l.str ? l.str : "";
	const char *rs = r.str ? r.str : "";
	return strcmp(ls,rs) >= 0;
}


inline bool operator > (const AString &l, const char *rs)
{
	const char *ls = l.str ? l.str : "";
	return strcmp(ls,rs) > 0;
}


inline bool operator >= (const AString &l, const char *rs)
{
	const char *ls = l.str ? l.str : "";
	return strcmp(ls,rs) >= 0;
}


#endif
