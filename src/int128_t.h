/*
 *
 *  Copyright (c) 2007-2022, Thomas Maier-Komor
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

#ifndef INT128_T_H
#define INT128_T_H

#include <assert.h>
#include <limits.h>
#include <string>
#include <stdint.h>
#include <sys/types.h>

#include <ostream>


class int128_t
{
	public:
	static const int128_t inf, ninf;

	int128_t(double d)
	: hi((int64_t)(d / ((double)UINT64_MAX + 1.0)))
	, lo((int64_t)(d - (double)hi * ((double)UINT64_MAX) + 1.0))
	{
		if (d < 0) {
			hi = ~hi;
			lo = ~lo + 1;
			if (lo == 0)
				++hi;
		}
	}

#if defined __GNUC__ && __GNUC__ < 4
	int128_t(int i)
	: hi(i >= 0 ? 0 : UINT64_MAX)
	, lo((int64_t) i)
	{ }
	
	int128_t(size_t s)
	: hi(s >= 0 ? 0 : UINT64_MAX)
	, lo((int64_t) s)
	{ }
#endif
	
	int128_t(bool b)
	: hi(0)
	, lo((int64_t) b)
	{ }
	
	int128_t(int32_t i)
	: hi(i >= 0 ? 0 : UINT64_MAX)
	, lo((int64_t) i)
	{ }
	
	int128_t(int64_t i)
	: hi(i >= 0 ? 0 : UINT64_MAX)
	, lo(i)
	{ }
	
	int128_t(uint32_t i)
	: hi(0)
	, lo(i)
	{ }
	
	int128_t(uint64_t i)
	: hi(0)
	, lo(i)
	{ }
	
	int128_t(uint64_t h, uint64_t l)
	: hi(h)
	, lo(l)
	{ }
	
// equivalent to the default copy constructor, but default
// is probably faster...
//	int128_t(const int128_t &z)
//	: hi(z.hi)
//	, lo(z.lo)
//	{ }

	int128_t()
	: hi(0)
	, lo(0)
	{ }

	int128_t(const char *s);
//	int128_t(const mpz_class &);

	void to_dec(char *) const;
	std::string to_dec() const;
	std::string to_hex(bool = true) const;

	int128_t &operator = (double);
#if defined __GNUC__ && __GNUC__ < 4
	int128_t &operator = (int);
#endif
	int128_t &operator = (int32_t);
	int128_t &operator = (int64_t);
	int128_t &operator = (uint32_t);
	int128_t &operator = (uint64_t);
	int128_t &operator = (const char *);
	int128_t &operator <<= (uint32_t);
	int128_t &operator >>= (uint32_t);
	int128_t &operator += (const int128_t &);
	int128_t &operator += (uint64_t);
	int128_t &operator -= (const int128_t &);
	int128_t &operator *= (const int128_t &);
	int128_t &operator *= (int32_t);
	int128_t &operator /= (const int128_t &);
	int128_t &operator /= (int32_t);
	int128_t &operator %= (const int128_t &);
	int128_t &operator ++ ();
	int128_t &operator -- ();
//	operator mpz_class () const;
//	mpz_class to_mpz() const
//	{ return operator mpz_class (); }
	operator double () const;
	operator int32_t () const
	{
		//assert(((hi == 0) && (lo < INT_MAX)) || ((hi == UINT64_MAX) && (lo > (UINT64_MAX - INT_MAX))));
		return (int32_t) lo;
	}

	operator bool () const
	{ return (hi | lo) != 0; }

	operator uint32_t () const
	{ assert((hi == 0) && (lo <= UINT32_MAX)); return (uint32_t) lo; }

	int128_t &operator |= (const int128_t &r)
	{
		hi |= r.hi;
		lo |= r.lo;
		return *this;
	}

	int128_t &operator &= (const int128_t &r)
	{
		hi &= r.hi;
		lo &= r.lo;
		return *this;
	}

	bool is_inf() const
	{ return (((int64_t)hi == INT64_MAX) && (lo == UINT64_MAX)); }

	bool is_ninf() const
	{ return (((int64_t)hi == (int64_t)INT64_MIN) && (lo == 0U)); }

	bool fits_ulong_p() const
	{ return (hi == 0); }

	/*
	bool fits_char() const
	{ return ((hi == 0) && (lo <= 127)) || ((hi == UINT64_MAX) && (lo >= -128)); }

	*/

	bool fits_slong_p() const
	{ return (hi == 0) || (hi == UINT64_MAX); }

	bool fits_si() const
	{ return ((hi == 0) && (lo <= INT32_MAX)) || ((hi == UINT64_MAX) && ((int64_t)lo >= INT32_MIN)); }

	size_t get_si() const
	{ return (size_t) lo; }

	uint64_t get_ui() const
	{ return lo; }

	int128_t abs() const
	{
		if (hi >> 63) {
			int128_t r(~hi,~lo);
			++r.lo;
			if (0 == r.lo)
				++r.hi;
			return r;
		}
		return int128_t(hi,lo);
	}

	int128_t &negate()
	{
		if (hi | lo) {
			hi = ~hi;
			lo = ~lo + 1;
			if (lo == 0)
				++hi;
		}
		return *this;
	}

	unsigned bit_count() const
	{
		unsigned numbits = 0;
		uint64_t h = hi;
		do {
			numbits += (h&1);
			h >>= 1;
		} while (h);
		uint64_t l = lo;
		do {
			numbits += (l&1);
			l >>= 1;
		} while (l);
		return numbits;
	}

	unsigned log2() const
	{
		unsigned x = 0;
		while (x < 64) {
			if (lo & 1<<x) {
				assert((hi == 0) && ((lo ^ (1 << x)) == 0));
				return x;
			}	
			++x;
		}
		while (x < 128) {
			if (hi & 1<<(x-64)) {
				assert((hi ^ (1 << (x - 64))) == 0);
				return x;
			}
			++x;
		}
		assert(0);
		return 0;
	}

	const char *initfrom(const char *);

	uint64_t hi,lo;
};


std::ostream & operator << (std::ostream &, const int128_t &);


int128_t operator * (const int128_t &l, const int128_t &r);
int128_t operator / (const int128_t &l, const int128_t &r);
int128_t operator % (const int128_t &l, const int128_t &r);
int128_t operator + (const int128_t &l, const int128_t &r);
int128_t operator - (const int128_t &l, const int128_t &r);
int128_t operator + (const int128_t &l, int r);
int128_t operator - (const int128_t &l, int r);
int128_t operator | (const int128_t &l, const int128_t &r);
int128_t operator & (const int128_t &l, const int128_t &r);
int128_t operator ^ (const int128_t &l, const int128_t &r);
int128_t operator - (const int128_t &l);
int128_t operator ~ (const int128_t &l);
int128_t operator << (const int128_t &l, int r);
int128_t operator >> (const int128_t &l, int r);
int128_t abs(const int128_t &l);
bool operator < (const int128_t &l, int r);
bool operator <= (const int128_t &l, int r);
bool operator > (const int128_t &l, int r);
bool operator >= (const int128_t &l, int r);
bool operator > (const int128_t &l, const int128_t &r);
bool operator >= (const int128_t &l, const int128_t &r);
bool operator < (const int128_t &l, const int128_t &r);
bool operator <= (const int128_t &l, const int128_t &r);
bool operator == (const int128_t &l, const int128_t &r);
bool operator != (const int128_t &l, const int128_t &r);
bool operator == (const int128_t &l, int r);
bool operator != (const int128_t &l, int r);
inline bool operator ! (const int128_t &z)
{ return (z.hi | z.lo) == 0; }
inline bool operator < (int l, const int128_t &r)
{ return int128_t(l) < r; }
inline bool operator > (int l, const int128_t &r)
{ return int128_t(l) > r; }

void int128_t_divmod(const int128_t &l, const int128_t &r, int128_t &d, int128_t &m);


#ifdef TESTMODULE
#define inline
#endif
inline int128_t abs(const int128_t &l)
{
	//if (l.hi >> 63) {
	if ((int64_t)l.hi >= 0)
		return l;
	int128_t ret;
	if (l.lo == 0)
		ret.hi = -(int64_t)l.hi;
	else
		ret.hi = ~l.hi;
	ret.lo = (uint64_t)-(int64_t)l.lo;
	return ret;
}

inline int128_t abs2(const int128_t &l)
{
	if (l.hi >> 63) {
		int128_t ret;
		if (l.lo == 0)
			ret.hi = ~l.hi + 1;
		else
			ret.hi = ~l.hi;
		ret.lo = ~l.lo + 1;
		return ret;
	}
	return l;
}


inline int128_t operator - (const int128_t &r)
{
	if (r.hi | r.lo) {
		int128_t ret(~r.hi, ~r.lo + 1);
		if (ret.lo == 0)
			++ret.hi;
		return ret;
	}
	return int128_t();
}


#if defined __GNUC__ && __GNUC__ < 4
inline int128_t &int128_t::operator = (int i)
{
	if (i < 0) {
		hi = UINT64_MAX;
	} else {
		hi = 0;
	}
	lo = (int64_t) i;
	return *this;
}
#endif


inline int128_t &int128_t::operator = (int32_t i)
{
	if (i < 0) {
		hi = UINT64_MAX;
	} else {
		hi = 0;
	}
	lo = (int64_t) i;
	return *this;
}


inline int128_t &int128_t::operator = (int64_t i)
{
	hi = 0;
	if (i < 0) 
		--hi;
	lo = i;
	return *this;
}


inline int128_t &int128_t::operator = (uint32_t u)
{
	hi = 0;
	lo = (uint64_t) u;
	return *this;
}


inline int128_t &int128_t::operator = (uint64_t u)
{
	hi = 0;
	lo = u;
	return *this;
}


inline int128_t &int128_t::operator = (const char *z)
{
	initfrom(z);
	return *this;
}


/*
inline int128_t &int128_t::operator *= (int32_t i)
{
	*this *= int128_t(i);
	return *this;
}
*/


inline int128_t &int128_t::operator ++ ()
{
	if (!(++lo))
		++hi;
	return *this;
}


inline int128_t &int128_t::operator -- ()
{
	if (!(~--lo))
		--hi;
	return *this;
}


inline int128_t &int128_t::operator += (uint64_t u)
{
	lo += u;
	if (lo < u)
		++hi;
	return *this;
}


inline bool operator == (const int128_t &l, const int128_t &r)
{
	return ((l.hi ^ r.hi) | (l.lo ^ r.lo)) == 0;
}


inline bool operator != (const int128_t &l, const int128_t &r)
{
	return ((l.hi ^ r.hi) | (l.lo ^ r.lo)) != 0;
}


inline bool operator == (const int128_t &l, int r)
{
	if (r < 0) 
		return !((~l.hi) | (l.lo ^ (uint64_t)(-r)));
	return !(l.hi | (l.lo ^ r));
}


inline bool operator != (const int128_t &l, int r)
{
	if (r < 0) 
		return ((~l.hi) | (l.lo ^ (uint64_t)(-r)));
	return (l.hi | (l.lo ^ r));
}


inline int128_t operator + (const int128_t &l, int r)
{
	uint64_t hi,lo;
	hi = l.hi;
	if (r > 0) {
		lo = l.lo + r;
		if (lo < l.lo)
			++hi;
	} else {
		lo = l.lo + (~(uint64_t)(-r)) + 1;
		hi = l.hi + UINT64_MAX;
	}
	return int128_t(hi,lo);
}


inline int128_t operator + (const int128_t &l, const int128_t &r)
{
	uint64_t h = l.hi + r.hi, t = l.lo + r.lo;
	if (t < l.lo)
		++h;
	return int128_t(h,t);
}


inline int128_t &int128_t::operator += (const int128_t &r)
{
	hi += r.hi;
	lo += r.lo;
	if (lo < r.lo)
		++hi;
	return *this;
}


inline int128_t operator - (const int128_t &l, const int128_t &r)
{
	int128_t ret(-r);
	ret.hi += l.hi;
	ret.lo += l.lo;
	if (ret.lo < l.lo)
		++ret.hi;
	return ret;
}


inline int128_t &int128_t::operator -= (const int128_t &r)
{
	int128_t tmp(-r);
	hi += tmp.hi;
	lo += tmp.lo;
	if (lo < tmp.lo)
		++hi;
	return *this;
}


inline int128_t operator ~ (const int128_t &r)
{
	return int128_t(~r.hi,~r.lo);
}


inline int128_t operator & (const int128_t &l, const int128_t &r)
{
	return int128_t(l.hi & r.hi, l.lo & r.lo);
}


inline int128_t operator & (const int128_t &l, unsigned r)
{
	return int128_t(0, l.lo & r);
}


inline int128_t operator | (const int128_t &l, const int128_t &r)
{
	return int128_t(l.hi | r.hi, l.lo | r.lo);
}


inline int128_t operator ^ (const int128_t &l, const int128_t &r)
{
	return int128_t(l.hi ^ r.hi, l.lo ^ r.lo);
}


inline int128_t &int128_t::operator <<= (uint32_t u) 
{
	/*
	do {
		unsigned n = u;
		if (n > 32)
			n = 32;
		u -= n;
		uint64_t i0 = hi >> 32,
			 i1 = (uint32_t) hi,
			 i2 = lo >> 32,
			 i3 = (uint32_t) lo;
		i0 <<= n;
		i1 <<= n;
		i2 <<= n;
		i3 <<= n;
		i0 <<= 32;
		i0 |= i1;
		i0 |= i2 >> 32;
		i2 <<= 32;
		i2 |= i3;
		hi = i0;
		lo = i2;
	} while (u);
	return *this;
	*/
	if (u >= 127) {
		hi &= INT64_MIN;
		lo = 0;
		return *this;
	}
	do {
		uint32_t n = u > 63 ? 63 : u;
		u -= n;
		hi = ((int64_t)hi) << n;
		hi |= lo >> (64-n);
		lo <<= n;
	} while (u);
	return *this;
}


inline int128_t &int128_t::operator >>= (uint32_t u) 
{
	/*
	do {
		unsigned n = u > 32 ? 32 : u;
		u -= n;
		uint64_t i0 = hi & 0xffffffff00000000ull,
			 i1 = hi << 32,
			 i2 = lo & 0xffffffff00000000ull,
			 i3 = lo << 32;
		i0 >>= n;
		i1 >>= n;
		i2 >>= n;
		i3 >>= n;
		i0 |= i1 >> 32;
		i2 |= i1 << 32;
		i2 |= i3 >> 32;
		hi = i0;
		lo = i2;
	} while (u);
	return *this;
	*/
	if (u >= 127) {
		hi &= INT64_MIN;
		lo = 0;
		return *this;
	}
	do {
		uint32_t n = u > 63 ? 63 : u;
		u -= n;
		lo >>= n;
		lo |= hi << (64-n);
		hi = ((int64_t)hi) >> n;
	} while (u);
	return *this;
}


inline int128_t operator << (const int128_t &l, int u) 
{
	/*
	if (u >= 128)
		return int128_t();
	int128_t r(l);
	do {
		unsigned n = u;
		if (n > 32)
			n = 32;
		u -= n;
		uint64_t i0 = r.hi >> 32,
			 i1 = (uint32_t) r.hi,
			 i2 = r.lo >> 32,
			 i3 = (uint32_t) r.lo;
		i0 <<= n;
		i1 <<= n;
		i2 <<= n;
		i3 <<= n;
		i0 <<= 32;
		i0 |= i1;
		i0 |= i2 >> 32;
		i2 <<= 32;
		i2 |= i3;
		r.hi = i0;
		r.lo = i2;
	} while (u);
	return r;
	*/
	assert(u >= 0);
	if (u >= 127) {
		if (l.hi >> 63)
			return int128_t(INT64_MIN,0);
		return int128_t();
	}
	int128_t r(l);
	do {
		uint32_t n = u > 63 ? 63 : u;
		u -= n;
		r.hi = ((int64_t)r.hi) << n;
		r.hi |= r.lo >> (64-n);
		r.lo <<= n;
	} while (u);
	return r;
}


inline int128_t operator >> (const int128_t &l, int u) 
{
	/*
	if (u >= 128)
		return int128_t();
	int128_t r(l);
	do {
		unsigned n = u > 32 ? 32 : u;
		u -= n;
		uint64_t i0 = r.hi & 0xffffffff00000000ull,
			 i1 = r.hi << 32,
			 i2 = r.lo & 0xffffffff00000000ull,
			 i3 = r.lo << 32;
		i0 >>= n;
		i1 >>= n;
		i2 >>= n;
		i3 >>= n;
		i0 |= i1 >> 32;
		i2 |= i1 << 32;
		i2 |= i3 >> 32;
		r.hi = i0;
		r.lo = i2;
	} while (u);
	return r;
	*/
	assert(u >= 0);
	if (u >= 127) {
		if (l.hi >> 63)
			return int128_t(INT64_MIN,0);
		return int128_t();
	}
	int128_t r(l);
	do {
		uint32_t n = u > 63 ? 63 : u;
		u -= n;
		r.lo >>= n;
		r.lo |= r.hi << (64-n);
		r.hi = ((int64_t)r.hi) >> n;
	} while (u);
	return r;
}


#if defined __GNUC__ && __GNUC__ < 4
inline bool operator < (const int128_t &l, int r)
{
	return l < int128_t(r);
}


inline bool operator <= (const int128_t &l, int r)
{
	return l <= int128_t(r);
}


inline bool operator > (const int128_t &l, int r)
{
	return l > int128_t(r);
}


inline bool operator >= (const int128_t &l, int r)
{
	return l >= int128_t(r);
}
#endif


inline bool operator < (const int128_t &l, int32_t r)
{
	return l < int128_t(r);
}


inline bool operator <= (const int128_t &l, int32_t r)
{
	return l <= int128_t(r);
}


inline bool operator > (const int128_t &l, int32_t r)
{
	return l > int128_t(r);
}


inline bool operator >= (const int128_t &l, int32_t r)
{
	return l >= int128_t(r);
}


inline int128_t::operator double () const
{
	double r;
	if (hi>>63) {
		r = (double)(~lo + 1);
	} else {
		r = (double)lo;
	}
	r += (double)hi * (double)INT64_MAX;
	return r;
}


inline int128_t operator - (const int128_t &l, int r)
{
	return l + int128_t(-r);
}


inline bool operator <= (const int128_t &l, const int128_t &r)
{
	int64_t d = l.hi - r.hi;
	return d ? (d < 0) : (l.lo <= r.lo);
}


inline bool operator < (const int128_t &l, const int128_t &r)
{
	int64_t d = l.hi - r.hi;
	return d ? (d < 0) : (l.lo < r.lo);
}


inline bool operator >= (const int128_t &l, const int128_t &r)
{
	int64_t lh = l.hi, rh = r.hi;
	if (lh == rh) {
		return (l.lo >= r.lo);
	} else
		return (lh > rh);
}


inline bool operator > (const int128_t &l, const int128_t &r)
{
	int64_t lh = l.hi, rh = r.hi;
	if (lh == rh) {
		return (l.lo > r.lo);
	} else
		return (lh > rh);
}


#endif
