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

#ifndef _ARRAY_H
#define _ARRAY_H

#include <assert.h>
#include <stdint.h>
#include <string.h>

#ifdef __AVR__
typedef uint16_t size_t;
typedef int16_t ssize_t;
#endif

template <typename T, size_t maxsize>
class array;

template <typename T,size_t arraysize>
bool operator != (const array<T,arraysize> &l, const array<T,arraysize> &r);

template <typename T, size_t maxsize>
class array
{
	public:
	array()
	: n(0)
	, data()
	{ }

	void clear()
	{ n = 0; }

	size_t size() const
	{ return n; }

	void resize(size_t ns)
	{
		assert(ns <= maxsize);
		n = ns;
	}

	void push_back(T d)
	{
		assert(n < maxsize);
		data[n++] = d;
	}

	const T& operator [] (size_t x) const
	{
		assert(x < n);
		return data[x];
	}

	T& operator [] (size_t x)
	{
		assert(x < n);
		return data[x];
	}

	const T& back() const
	{
		assert(n != 0);
		return data[n-1];
	}

	T& back()
	{
		assert(n != 0);
		return data[n-1];
	}

	bool empty() const
	{ return n == 0; }

	private:
	size_t n;
	T data[maxsize];

	friend bool operator != <T,maxsize>(const array<T,maxsize> &,const array<T,maxsize> &);
};


template <typename T,size_t arraysize>
bool operator != (const array<T,arraysize> &l, const array<T,arraysize> &r)
{
	return 0 != memcmp(l.data,r.data,sizeof(T)*arraysize);
}

#endif
