/*
 *  Copyright (C) 2019, Thomas Maier-Komor
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

#include <assert.h>
#include <stdlib.h>
#include <string.h>

class AString
{
	public:
	AString()
	: str(0)
	{ }

	AString(const char *s)
	{
		if (s)
			str = strdup(s);
		else
			str = 0;
	}

	AString(const AString &a)
	{
		if (a.str)
			str = strdup(a.str);
		else
			str = 0;
	}

	~AString()
	{
		if (str)
			free(str);
	}

	AString &operator = (const AString &a)
	{
		if (str)
			free(str);
		if (a.str)
			str = strdup(a.str);
		else
			str = 0;
		return *this;
	}

	AString &operator += (const AString &a)
	{
		if (str == 0) {
			if (a.str != 0)
				str = strdup(a.str);
			return *this;
		}
		if ((a.str == 0) || (a.str[0] == 0))
			return *this;
		size_t ol = strlen(str);
		size_t al = strlen(a.str);
		str = (char*)realloc(str,ol+al+1);
		memcpy(str+ol,a.str,al+1);
		return *this;
	}

	bool operator == (const AString &a) const
	{
		if (str == 0) {
			if (a.str == 0)
				return true;
			if (a.str[0] == 0)
				return true;
			return false;
		}
		if (a.str == 0) {
			if (str[0] == 0)
				return true;
			return false;
		}
		return 0 == strcmp(str,a.str);
	}

	bool operator != (const AString &a) const
	{ return !(*this == a); }


	AString &operator = (const char *s)
	{
		if (str)
			free(str);
		if (s)
			str = strdup(s);
		else
			str = 0;
		return *this;
	}

	AString &operator += (const char *s)
	{
		if (str == 0) {
			if (s != 0)
				str = strdup(s);
			return *this;
		}
		if ((s == 0) || (s[0] == 0))
			return *this;
		size_t ol = strlen(str);
		size_t al = strlen(s);
		str = (char*)realloc(str,ol+al+1);
		memcpy(str+ol,s,al+1);
		return *this;
	}

	bool operator == (const char *s) const
	{
		if (str == 0) {
			if (s == 0)
				return true;
			if (s[0] == 0)
				return true;
			return false;
		}
		if (s == 0) {
			if (str[0] == 0)
				return true;
			return false;
		}
		return 0 == strcmp(str,s);
	}

	bool operator != (const char *a) const
	{ return !(*this == a); }


	const char *c_str() const
	{
		if (str == 0) {
			str = (char*)malloc(1);
			*str = 0;
		}
		return str;
	}

	size_t size() const
	{ return str ? strlen(str) : 0; }

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
