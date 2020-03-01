/*
 *  Copyright (C) 2020, Thomas Maier-Komor
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

#ifndef _FSTRING_H
#define _FSTRING_H

#include <string.h>

template <size_t len>
class FString
{
	public:
	FString()
	{
		str[0] = 0;
	}

	FString(const char *s)
	{
		strncpy(str,s,len-1);
	}

	FString(const char *s, size_t l)
	{
		if (l >= len)
			l = len-1;
		memcpy(str,s,l);
		str[l] = 0;
	}

	FString(const FString &a)
	{
		strcpy(str,a.str);
	}

	FString &operator = (const FString &a)
	{
		strcpy(str,a.str);
		return *this;
	}

	FString &operator += (const FString &a)
	{
		strncat(str,a.str,len-1);
		return *this;
	}

	bool operator == (const FString &a) const
	{
		return 0 == strcmp(str,a.str);
	}

	bool operator != (const FString &a) const
	{ return 0 != strcmp(str,a.str); }


	FString &operator = (const char *s)
	{
		strncpy(str,s,len-1);
		return *this;
	}

	FString &operator += (const char *s)
	{
		strncat(str,s,len-1);
		return *this;
	}

	bool operator == (const char *s) const
	{ return 0 == strcmp(str,s); }

	bool operator != (const char *a) const
	{ return 0 != strcmp(str,s); }


	const char *c_str() const
	{ return str; }

	const char *data() const
	{ return str; }

	void assign(const char *s, size_t l)
	{ strncpy(str,s,l<len?l:len-1); }

	void clear()
	{ str[0] = 0; }

	size_t size() const
	{ return strlen(str); }

	bool empty() const
	{ return str[0] == 0; }

	friend bool operator < (const FString<len> &l, const FString<len> &r)
	{ return strcmp(l.str,r.str) < 0; }

	friend bool operator <= (const FString<len> &l, const FString<len> &r)
	{ return strcmp(l.str,r.str) <= 0; }
	
	friend bool operator > (const FString<len> &l, const FString<len> &r)
	{ return strcmp(l.str,r.str) > 0; }

	friend bool operator >= (const FString<len> &l, const FString<len> &r)
	{ return strcmp(l.str,r.str) >= 0; }
	
	friend bool operator < (const FString<len> &l, const char *r)
	{ return strcmp(l.str,r) < 0; }

	friend bool operator <= (const FString<len> &l, const char *r)
	{ return strcmp(l.str,r) <= 0; }

	friend bool operator > (const FString<len> &l, const char *r)
	{ return strcmp(l.str,r) > 0; }

	friend bool operator >= (const FString<len> &l, const char *r)
	{ return strcmp(l.str,r) >= 0; }

	private:
	char str[len];
};


#endif

