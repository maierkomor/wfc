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

#include "int128_t.h"
#include <signal.h>
#include <stdlib.h>

#if defined(_WIN32) || defined(__CYGWIN__)
#define abort() (*(char *)(0) = 0)
#ifdef assert
#undef assert
#endif
#define assert(x) ((x) || abort())
#endif

using namespace std;

const int128_t
	int128_t::inf(INT64_MAX,UINT64_MAX),
	int128_t::ninf(INT64_MIN,0);

int128_t::int128_t(const char *z)
: hi(0)
, lo(0)
{
	initfrom(z);
}


//int128_t::int128_t(const mpz_class &z)
//{
//	string str(z.get_str());
//	initfrom(str.c_str());
//}


const char *int128_t::initfrom(const char *Z)
{
	const char *z = Z;
	bool neg = false;
	hi = lo = 0;
	if (z[0] == '-') {
		neg = true;
		++z;
	}
	if (z[0] == '0') {
		if ((z[1] == 'x') || (z[1] == 'X')) {
			// hexadecimal
			z++;
			while (1) {
				char c = *(++z);
				if ((c >= '0') && (c <= '9')) {
					c -= '0';
				} else if ((c >= 'a') && (c <= 'f')) {
					c -= 'a' - 10;
				} else if ((c >= 'A') && (c <= 'F')) {
					c -= 'A' - 10;
				} else {
					break;;
				}
				if (hi >= 0x1000000000000000ull) {
					lo = UINT64_MAX;
					hi = INT64_MAX;
					break;
				}
				hi <<= 4;
				hi |= lo >> 60;
				lo <<= 4;
				lo |= c;
			}
		} else {
			// octal
			char c = *z;
			while ((c >= '0') && (c <= '7')) {
				if (hi > (INT64_MAX >> 3)) {
					lo = UINT64_MAX;
					hi = INT64_MAX;
					break;
				}
				hi <<= 3;
				hi |= lo >> 61;
				lo <<= 3;
				lo |= c - '0';
				c = *(++z);
			}
		}
	} else {
		// decimal
		char c = *z;
		while ((c >= '0') && (c <= '9')) {
			if (hi > (INT64_MAX / 10)) {
				lo = UINT64_MAX;
				hi = INT64_MAX;
				break;
			}
			hi *= 10;
			if (lo > (UINT64_MAX / 10)) {
				uint64_t h = lo >> 32, l = lo & 0xffffffff;
				h *= 10;
				l *= 10;
				hi += h >> 32;
				lo = (h << 32) + l;
			} else
				lo *= 10;
			lo += c - '0';
			c = *(++z);
		}
	}
	if (neg) {
		hi = ~hi;
		lo = ~lo;
		++lo;
		if (lo == 0)
			++hi;
	}
	return z;
}


static inline
void multiply(const int128_t &l, const int128_t &r, int128_t &ret)
{
	bool neg = ((l.hi^r.hi) >> 63) != 0;
	uint64_t l0,r0,l1,r1,l2,r2,l3,r3;
	if (l.hi >> 63) {
		const int128_t &t = -l;
		l0 = (uint32_t) t.lo;
		l2 = (uint32_t) t.hi;
		l1 = t.lo >> 32;
		l3 = t.hi >> 32;
	} else {
		l0 = (uint32_t) l.lo;
		l2 = (uint32_t) l.hi;
		l1 = l.lo >> 32;
		l3 = l.hi >> 32;
	}
	if (r.hi >> 63) {
		int128_t t(-r);
		r0 = (uint32_t) t.lo;
		r2 = (uint32_t) t.hi;
		r1 = t.lo >> 32;
		r3 = t.hi >> 32;
	} else {
		r0 = (uint32_t) r.lo;
		r2 = (uint32_t) r.hi;
		r1 = r.lo >> 32;
		r3 = r.hi >> 32;
	}
//	if (l3 && (r1 | r2 | r3))
//		goto overflow;
//	if (r3 && (l1 | l2 | l3))
//		goto overflow;
//	if (l2 && r2)
//		goto overflow;
	uint64_t i[4];
	i[0]  = l0 * r0;
	i[1]  = i[0] >> 32;
	i[0] = (uint32_t) i[0];
	i[1] += l1 * r0;
	i[2]  = i[1] >> 32;
	i[1] = (uint32_t) i[1];
	i[1] += l0 * r1;
	i[2] += i[1] >> 32;
	i[1] = (uint32_t) i[1];
	i[2] += l1 * r1;
	i[3]  = i[2] >> 32;
	i[2] = (uint32_t) i[2];
	i[2] += l0 * r2;
	i[3] += i[2] >> 32;
	i[2] = (uint32_t) i[2];
	i[2] += l2 * r0;
	i[3] += i[2] >> 32;
	i[2] = (uint32_t) i[2];
	i[3] += l0 * r3;
	if (i[3] >> 32)
		goto overflow;
	i[3] += l1 * r2;
	if (i[3] >> 32)
		goto overflow;
	i[3] += l2 * r1;
	if (i[3] >> 32)
		goto overflow;
	i[3] += l3 * r0;
	if (i[3] >> 32)
		goto overflow;
	ret.hi = (i[3] << 32) + i[2];
	ret.lo = (i[1] << 32) + i[0];
	if (neg)
		ret.negate();
	return;
overflow:
	//warning("overflow occured in multiplication - saturating");
	if (neg) {
		ret.hi = INT64_MIN;
		ret.lo = 0;
	} else {
		ret.hi = INT64_MAX;
		ret.lo = UINT64_MAX;
	}
	return;
}

int128_t operator * (const int128_t &l, const int128_t &r)
{
	int128_t ret;
	multiply(l,r,ret);
	return ret;
}

int128_t &int128_t::operator *= (const int128_t &r)
{
	multiply(*this,r,*this);
	return *this;
	/*
	bool neg = ((hi^r.hi) >> 63) != 0;
	uint64_t l0,r0,l1,r1,l2,r2,l3,r3;
	if (hi >> 63) {
		int128_t t(-(*this));
		l0 = (uint32_t) t.lo;
		l2 = (uint32_t) t.hi;
		l1 = t.lo >> 32;
		l3 = t.hi >> 32;
	} else {
		l0 = (uint32_t) lo;
		l2 = (uint32_t) hi;
		l1 = lo >> 32;
		l3 = hi >> 32;
	}
	if (r.hi >> 63) {
		int128_t t(-r);
		r0 = (uint32_t) t.lo;
		r2 = (uint32_t) t.hi;
		r1 = t.lo >> 32;
		r3 = t.hi >> 32;
	} else {
		r0 = (uint32_t) r.lo;
		r2 = (uint32_t) r.hi;
		r1 = r.lo >> 32;
		r3 = r.hi >> 32;
	}
	lo = hi = 0;
	if (l3 && (r1 | r2 | r3))
		goto overflow;
	if (r3 && (l1 | l2 | l3))
		goto overflow;
	if (l2 && r2)
		goto overflow;
	uint64_t i[4];
	i[0]  = l0 * r0;
	i[1]  = i[0] >> 32;
	i[0] = (uint32_t) i[0];
	i[1] += l1 * r0;
	i[2]  = i[1] >> 32;
	i[1] = (uint32_t) i[1];
	i[1] += l0 * r1;
	i[2] += i[1] >> 32;
	i[1] = (uint32_t) i[1];
	i[2] += l1 * r1;
	i[3]  = i[2] >> 32;
	i[2] = (uint32_t) i[2];
	i[2] += l0 * r2;
	i[3] += i[2] >> 32;
	i[2] = (uint32_t) i[2];
	i[2] += l2 * r0;
	i[3] += i[2] >> 32;
	i[2] = (uint32_t) i[2];
	i[3] += l0 * r3;
	if (i[3] >> 32)
		goto overflow;
	i[3] += l1 * r2;
	if (i[3] >> 32)
		goto overflow;
	i[3] += l2 * r1;
	if (i[3] >> 32)
		goto overflow;
	i[3] += l3 * r0;
	if (i[3] >> 31)
		goto overflow;
	hi = (i[3] << 32) + i[2];
	lo = (i[1] << 32) + i[0];
	if (neg)
		negate();
	return *this;
overflow:
	//warning("overflow occured in multiplication - saturating");
	if (neg) {
		hi = INT64_MIN;
		lo = 0;
	} else {
		hi = INT64_MAX;
		lo = UINT64_MAX;
	}
	return *this;
	*/
}


int128_t &int128_t::operator *= (int32_t r)
{
	bool neg = ((hi >> 63) ^ (r >> 31)) != 0;
	uint64_t l0,r0,l1,l2,l3;
	if (hi >> 63) {
		int128_t t(-(*this));
		l0 = (uint32_t) t.lo;
		l2 = (uint32_t) t.hi;
		l1 = t.lo >> 32;
		l3 = t.hi >> 32;
	} else {
		l0 = (uint32_t) lo;
		l2 = (uint32_t) hi;
		l1 = lo >> 32;
		l3 = hi >> 32;
	}
	if (r < 0) {
		r0 = -r;
	} else {
		r0 = r;
	}
	lo = hi = 0;
	uint64_t i[4];
	i[0]  = l0 * r0;
	i[1]  = i[0] >> 32;
	i[0] = (uint32_t) i[0];
	i[1] += l1 * r0;
	i[2]  = i[1] >> 32;
	i[1] = (uint32_t) i[1];
	i[2] += l2 * r0;
	i[3] = i[2] >> 32;
	i[2] = (uint32_t) i[2];
	i[3] += l3 * r0;
	if (i[3] >> 31)
		goto overflow;
	hi = (i[3] << 32) + i[2];
	lo = (i[1] << 32) + i[0];
	if (neg)
		negate();
	return *this;
overflow:
	//warning("overflow occured in multiplication - saturating");
	if (neg) {
		hi = INT64_MIN;
		lo = 0;
	} else {
		hi = INT64_MAX;
		lo = UINT64_MAX;
	}
	return *this;
}


string int128_t::to_dec() const
{
	if ((hi | lo) == 0)
		return string("0");
	//int128_t t(abs()),z(0x4b3b4ca85a86c47allu,0x098a224000000000llu);
//	if (is_inf())
//		return "inf";
//	if (is_ninf())
//		return "-inf";
	char dec[40], *d = dec;	// 38 digits are possible with 127 bits

	int128_t t(abs()),z(10);
	if (hi >> 63) {
		*d++ = '-';
	}
	if (t.hi >> 63) {
		return("170141183460469231731687303715884105728");
	}
	while (z <= t && (z.hi < 0x4b3b4ca85a86c47aull))
		//z *= int128_t(10);
		z *= 10;
	if ((z.hi < 0x4b3b4ca85a86c47aull) || (z>t))
		z /= int128_t(10);
	assert(z <= t);
	while ((z.lo > 1)|z.hi) {	// while (z > int128_t(1))
		int128_t dig;
		int128_t_divmod(t,z,dig,t);
//		assert(dig.lo < 10);
		if (dig.lo > 10) {
			printf("%s/%s=%s",t.to_hex().c_str(),z.to_hex().c_str(),dig.to_hex().c_str());
			abort();
		}
		*d = (char) ('0' + dig.lo);
		++d;
		z /= int128_t(10);
	}
	*d = (char) ('0' + t.lo);
	++d;
	*d = 0;
	return string(dec);
}


void int128_t::to_dec(char *d) const
{
	if ((hi | lo) == 0) {
		*d = '0';
		d[1] = 0;
		return;
	}
	//int128_t t(abs()),z(0x4b3b4ca85a86c47allu,0x098a224000000000llu);
	int128_t t(abs()),z(10);

	if (hi >> 63) {
		*d++ = '-';
	}
	while (z <= t && (z.hi < 0x4b3b4ca85a86c47aull))
		z *= 10;
	if (z.hi != 0x4b3b4ca85a86c47aull)
		z /= 10;
	while ((z.lo > 1)|z.hi) {	// while (z > int128_t(1))
		int128_t dig;
		int128_t_divmod(t,z,dig,t);
		assert(dig.lo < 10);
		*d = (char) ('0' + dig.lo);
		++d;
		z /= 10;
	}
	*d = (char) ('0' + t.lo);
	++d;
	*d = 0;
}


string int128_t::to_hex(bool ox) const
{
	unsigned long long h = hi, l = lo;
	unsigned hx = 16, lx = 16;
//	if (is_inf())
//		return "inf";
//	if (is_ninf())
//		return "-inf";
	if ((hi | lo) == 0) {
		if (ox)
			return string("0x0");
		return string("0");
	}
	string o;
	if (hi>>63) {
		o += '-';
		h = ~h;
		l = ~l + 1;
		if (l == 0)
			++h;
	}
	if (ox)
		o += "0x";
	while (hx && ((h >> 60) == 0)) {
		h <<= 4;
		--hx;
	}
	while (hx) {
		char c = (char)(h >> 60);
		if (c < 10)
			c += '0';
		else
			c += 'a'-10;
		o += c;
		h <<= 4;
		--hx;
	}
	if (((hi == 0) || ((hi == UINT64_MAX))) && (lo != 0)) {
		while (lx && ((l >> 60) == 0)) {
			l <<= 4;
			--lx;
		}
	}
	while (lx) {
		char c = (char)(l >> 60);
		if (c < 10)
			c += '0';
		else
			c += 'a'-10;
		o += c;
		l <<= 4;
		--lx;
	}
	return o;
}


int128_t operator / (const int128_t &l, const int128_t &r)
{
	if (!r) {
		raise(SIGFPE);
		return 0;
	}
	// OK 1
	int128_t ret, l0(abs(l)), r0(abs(r));
	bool neg = ((l.hi^r.hi) >> 63) != 0;
	int shifted = 0;
	assert((r0.hi >> 63) == 0);
	//printf("/ l0 = %llx%016llx\n",l0.hi,l0.lo);
	//printf("/ r0 = %llx%016llx\n",r0.hi,r0.lo);
	while (l0.hi > r0.hi || (!(l0.hi < r0.hi) && (l0.lo > r0.lo))) {
		r0.hi <<= 1;
		r0.hi |= r0.lo >> 63;
		r0.lo <<= 1;
		//printf("/ r0 = %llx%016llx\n",r0.hi,r0.lo);
		++shifted;
	}
	//printf("shifted %d\n",shifted);
	//printf("/ l0 = %llx%016llx\n",l0.hi,l0.lo);
	do { 
		if ((r0.hi < l0.hi) || (!(r0.hi > l0.hi) && (r0.lo <= l0.lo))) {
			if (r0.lo > l0.lo)
				--l0.hi;
			l0.hi -= r0.hi;
			l0.lo -= r0.lo;
			//printf("/ l0 = %llx%016llx\n",l0.hi,l0.lo);
			if (shifted < 64) 
				ret.lo |= 1ull << shifted;
			else
				ret.hi |= 1ull << (shifted-64);
			//printf("/ ret = %llx%016llx\n",ret.hi,ret.lo);
		}
		--shifted;
		r0.lo >>= 1;
		r0.lo |= (r0.hi << 63);
		r0.hi >>= 1;
	} while (shifted >= 0);
	return neg ? -ret : ret;
}


int128_t &int128_t::operator /= (const int128_t &r)
{
	// OK 1
	int128_t l0(abs()), r0(r.abs());
	bool neg = ((hi ^ r.hi) >> 63) != 0;
	hi = lo = 0;
	int shifted = 0;
	while (l0.hi > r0.hi || (!(l0.hi < r0.hi) && (l0.lo > r0.lo))) {
		r0.hi <<= 1;
		r0.hi |= r0.lo >> 63;
		r0.lo <<= 1;
		++shifted;
	}
	do { 
		if ((r0.hi < l0.hi) || (!(r0.hi > l0.hi) && (r0.lo <= l0.lo))) {
			if (r0.lo > l0.lo)
				--l0.hi;
			l0.hi -= r0.hi;
			l0.lo -= r0.lo;
			if (shifted < 64) 
				lo |= 1ull << shifted;
			else
				hi |= 1ull << (shifted-64);
		}
		--shifted;
		r0.lo >>= 1;
		r0.lo |= (r0.hi << 63);
		r0.hi >>= 1;
	} while (shifted >= 0);
	if (neg) {
		if (hi | lo) {
			hi = ~hi;
			lo = ~lo + 1;
			if (lo == 0)
				++hi;
		}
	}
	return *this;
}


int128_t &int128_t::operator /= (int32_t r)
{
	// OK 1
	int128_t l0(abs()), r0(r < 0 ? -r : r);
	bool neg = ((hi >> 63) ^ (r < 0)) != 0;
	hi = lo = 0;
	int shifted = 0;
	while (l0.hi > r0.hi || (!(l0.hi < r0.hi) && (l0.lo > r0.lo))) {
		r0.hi <<= 1;
		r0.hi |= r0.lo >> 63;
		r0.lo <<= 1;
		++shifted;
	}
	do { 
		if ((r0.hi < l0.hi) || (!(r0.hi > l0.hi) && (r0.lo <= l0.lo))) {
			if (r0.lo > l0.lo)
				--l0.hi;
			l0.hi -= r0.hi;
			l0.lo -= r0.lo;
			if (shifted < 64) 
				lo |= 1ull << shifted;
			else
				hi |= 1ull << (shifted-64);
		}
		--shifted;
		r0.lo >>= 1;
		r0.lo |= (r0.hi << 63);
		r0.hi >>= 1;
	} while (shifted >= 0);
	if (neg) {
		if (hi | lo) {
			hi = ~hi;
			lo = ~lo + 1;
			if (lo == 0)
				++hi;
		}
	}
	return *this;
}


int128_t operator % (const int128_t &l, const int128_t &r)
{
	int128_t l0(l.abs()), r0(r.abs());
	bool neg = ((l.hi ^ r.hi) >> 63) != 0;
	int shifted = 0;
	while (l0.hi > r0.hi || (!(l0.hi < r0.hi) && (l0.lo > r0.lo))) {
		r0.hi <<= 1;
		r0.hi |= r0.lo >> 63;
		r0.lo <<= 1;
		++shifted;
	}
	do { 
		if ((r0.hi < l0.hi) || (!(r0.hi > l0.hi) && (r0.lo <= l0.lo))) {
			if (r0.lo > l0.lo)
				--l0.hi;
			l0.hi -= r0.hi;
			l0.lo -= r0.lo;
		}
		--shifted;
		r0.lo >>= 1;
		r0.lo |= (r0.hi << 63);
		r0.hi >>= 1;
	} while (shifted >= 0);
	return neg ? -int128_t(l0.hi,l0.lo) : int128_t(l0.hi,l0.lo);
}


int128_t &int128_t::operator %= (const int128_t &r)
{
	int128_t l0(abs()), r0(r.abs());
	bool neg = ((hi ^ r.hi) >> 63) != 0;
	int shifted = 0;
	while (l0.hi > r0.hi || (!(l0.hi < r0.hi) && (l0.lo > r0.lo))) {
		r0.hi <<= 1;
		r0.hi |= r0.lo >> 63;
		r0.lo <<= 1;
		++shifted;
	}
	do { 
		if ((r0.hi < l0.hi) || (!(r0.hi > l0.hi) && (r0.lo <= l0.lo))) {
			if (r0.lo > l0.lo)
				--l0.hi;
			l0.hi -= r0.hi;
			l0.lo -= r0.lo;
		}
		--shifted;
		r0.lo >>= 1;
		r0.lo |= (r0.hi << 63);
		r0.hi >>= 1;
	} while (shifted >= 0);
	neg = false;
	hi = l0.hi;
	lo = l0.lo;
	if (neg) {
		if (hi | lo) {
			hi = ~hi;
			lo = ~lo + 1;
			if (lo == 0)
				++hi;
		}
	}
	return *this;
}


void int128_t_divmod(const int128_t &l, const int128_t &r, int128_t &d, int128_t &m)
{
	// OK 1
	if ((r.lo == 0) && (r.hi == 0)) {
		raise(SIGFPE);
		return;
	}
	int128_t r0(abs(r));
	int shifted = 0;
	bool neg = ((l.hi ^ r.hi) >> 63) != 0;
	int128_t m0 = l.abs();
	//printf("%lld%08lld/%lld%08lld = ",l.hi,l.lo,r.hi,r.lo);
	while (m0.hi > r0.hi || (!(m0.hi < r0.hi) && (m0.lo > r0.lo))) {
		r0.hi <<= 1;
		r0.hi |= r0.lo >> 63;
		r0.lo <<= 1;
		++shifted;
	}
	int128_t d0;
	do { 
		if ((r0.hi < m0.hi) || (!(r0.hi > m0.hi) && (r0.lo <= m0.lo))) {
			if (r0.lo > m0.lo)
				--m0.hi;
			m0.hi -= r0.hi;
			m0.lo -= r0.lo;
			if (shifted < 64) 
				d0.lo |= 1ull << shifted;
			else
				d0.hi |= 1ull << (shifted-64);
		}
		--shifted;
		r0.lo >>= 1;
		r0.lo |= (r0.hi << 63);
		r0.hi >>= 1;
	} while (shifted >= 0);
	if (neg) {
		d = -d0;
		if (m0)
			--d;
		m = -m0;
	} else {
		d = d0;
		m = m0;
	}
	//printf("%lld,%lld\n",d.lo,m.lo);
}



int128_t &int128_t::operator = (double d)
{
	hi = (uint64_t)(d / (double)UINT64_MAX);
	lo = (uint64_t)(d - (double)hi * (double)UINT64_MAX);
	if (d < 0) {
		hi = ~hi;
		lo = ~lo + 1;
		if (lo == 0)
			++hi;
	}
	return *this;
}


//int128_t::operator mpz_class () const
//{
//	string str(to_dec());
//	mpz_class ret(str.c_str());
//	return ret;
//}



#undef __sun
#ifdef TESTMODULE
#include <iostream>
#include <string>
#ifdef __sun
#include <mp.h>
#include <sys/time.h>
#else
#include <gmp.h>
//#include <gmpxx.h>
#endif

const char *testdata[] = {
	"0x0",
	"0x1",
	"0xff",
	"0x100",
	"0xffff",
	"0x10000",
	"0xffffffff",
	"0x100000000",
	"0xffffffffffffffff",
	"0x10000000000000000",
	"0xffffffffffffffffffffffff",
	"0x1000000000000000000000000",
	"0x7fffffffffffffffffffffffffffffff",
	"-0x1",
	"-0xff",
	"-0x100",
	"-0xffff",
	"-0x10000",
	"-0xffffffff",
	"-0x100000000",
	"-0xffffffffffffffff",
	"-0x10000000000000000",
	"-0xffffffffffffffffffffffff",
	"-0x1000000000000000000000000",
	/*
	"0xffffffffffffffffffffffffffffffff",
	"-0xffffffffffffffffffffffffffffffff"
	*/
};

const char *ovfl_s = "100000000000000000000000000000000";


void test_mul(const char *ls, const char *rs)
{
	int128_t l(ls), r(rs);
	int128_t x;
	x = l * r;
	l *= r;
	string result(x.to_dec());
	if (x != l) {
		printf("(%s * %s) != (l *= r): %s != %s\n",ls,rs,result.c_str(),l.to_dec().c_str());
		abort();
	}

#ifdef __sun
	MINT *zl, *zr, *zx, *n1;
	n1 = mp_itom(-1);
	if (ls[0] == '-') {
		zl = mp_xtom((char *)ls+3);
		mp_mult(zl,n1,zl);
	} else
		zl = mp_xtom((char *)ls+2);
	n1 = mp_itom(-1);
	if (rs[0] == '-') {
		zr = mp_xtom((char *)rs+3);
		mp_mult(zr,n1,zr);
	} else
		zr = mp_xtom((char *)rs+2);
	zx = mp_itom(0);
	mp_mult(zl,zr,zx);
	char *zls = mp_mtox(zl);
	char *zrs = mp_mtox(zr);
	char *zxs = mp_mtox(zx);
	if (strcmp(zxs,result.c_str()+2)) {
		printf("int128_t: %s * %s = %s\n",ls,rs,result.c_str());
		printf("sun_mp: %s * %s = %s\n\n",zls,zrs,zxs);
	}
	free(zls);
	free(zrs);
	free(zxs);
	mp_mfree(zl);
	mp_mfree(zr);
	mp_mfree(zx);
#else
	mpz_t zl, zr, zy;
	mpz_init(zy);
	mpz_init_set_str(zl,ls,0);
	mpz_init_set_str(zr,rs,0);
	mpz_mul(zy,zl,zr);
	char *zls = mpz_get_str(0,10,zl);
	char *zrs = mpz_get_str(0,10,zr);
	char *zys = mpz_get_str(0,10,zy);
	if (strcmp(zys,result.c_str())) {
		printf("int128_t: %s * %s = %s\n",ls,rs,result.c_str());
		printf("gmp mpz: %s * %s = %s\n\n",zls,zrs,zys);
	}
	free(zls);
	free(zrs);
	free(zys);
	mpz_clear(zl);
	mpz_clear(zr);
	mpz_clear(zy);
#endif
}


void test_div(const char *ls, const char *rs)
{
	int128_t l(ls), r(rs);
	int128_t x;
	if (r == 0)
		return;
	x = l / r;
	l /= r;
	assert(x == l);
	string result(x.to_hex(false));

#ifdef __sun
	MINT *zl, *zr, *zx, *n1;
	n1 = mp_itom(-1);
	if (ls[0] == '-') {
		zl = mp_xtom((char *)ls+3);
		mp_mult(zl,n1,zl);
	} else
		zl = mp_xtom((char *)ls+2);
	n1 = mp_itom(-1);
	if (rs[0] == '-') {
		zr = mp_xtom((char *)rs+3);
		mp_mult(zr,n1,zr);
	} else
		zr = mp_xtom((char *)rs+2);
	zx = mp_itom(0);
	mp_mdiv(zl,zr,zx);
	char *zls = mp_mtox(zl);
	char *zrs = mp_mtox(zr);
	char *zxs = mp_mtox(zx);
	if (strcmp(zxs,result.c_str()+2)) {
		printf("int128_t: %s / %s = %s\n",ls,rs,result.c_str());
		printf("sun_mp: %s / %s = %s\n\n",zls,zrs,zxs);
	}
	free(zls);
	free(zrs);
	free(zxs);
	mp_mfree(zl);
	mp_mfree(zr);
	mp_mfree(zx);
#else
	mpz_t zl, zr, zy;
	mpz_init(zy);
	mpz_init_set_str(zl,ls,0);
	mpz_init_set_str(zr,rs,0);
	mpz_div(zy,zl,zr);
	char *zls = mpz_get_str(0,16,zl);
	char *zrs = mpz_get_str(0,16,zr);
	char *zys = mpz_get_str(0,16,zy);
	if (strcmp(zys,result.c_str())) {
		printf("int128_t: %s / %s = %s\n",ls,rs,result.c_str());
		printf("gmp mpz: %s / %s = %s\n\n",zls,zrs,zys);
	}
	free(zls);
	free(zrs);
	free(zys);
	mpz_clear(zl);
	mpz_clear(zr);
	mpz_clear(zy);
#endif
}


void test_sub(const char *ls, const char *rs)
{
	int128_t l(ls), r(rs);
	int128_t x;
	x = l - r;
	l -= r;
	assert(l == x);
	string r_dec(x.to_dec());
	string r_hex(x.to_hex(false));

#ifdef __sun
	MINT *zl, *zr, *zx, *n1;
	n1 = mp_itom(-1);
	if (ls[0] == '-') {
		zl = mp_xtom((char *)ls+3);
		mp_mult(zl,n1,zl);
	} else
		zl = mp_xtom((char *)ls+2);
	n1 = mp_itom(-1);
	if (rs[0] == '-') {
		zr = mp_xtom((char *)rs+3);
		mp_mult(zr,n1,zr);
	} else
		zr = mp_xtom((char *)rs+2);
	zx = mp_itom(0);
	mp_msub(zl,zr,zx);
	char *zls = mp_mtox(zl);
	char *zrs = mp_mtox(zr);
	char *zxs = mp_mtox(zx);
	if (strcmp(zxs,result.c_str()+2)) {
		printf("int128_t: %s - %s = %s\n",ls,rs,result.c_str());
		printf("sun_mp: %s - %s = %s\n\n",zls,zrs,zxs);
	}
	free(zls);
	free(zrs);
	free(zxs);
	mp_mfree(zl);
	mp_mfree(zr);
	mp_mfree(zx);
#else
	mpz_t zl, zr, zy;
	mpz_init(zy);
	mpz_init_set_str(zl,ls,0);
	mpz_init_set_str(zr,rs,0);
	mpz_sub(zy,zl,zr);
	char *zls = mpz_get_str(0,10,zl);
	char *zrs = mpz_get_str(0,10,zr);
	char *zys = mpz_get_str(0,10,zy);
	if (strcmp(zys,r_dec.c_str()) && !x.is_inf() && !x.is_ninf()) {
		printf("int128_t: %s - %s = %s (%s)\n",ls,rs,r_dec.c_str(),r_hex.c_str());
		printf("gmp mpz: %s - %s = %s\n\n",zls,zrs,zys);
	}
	free(zls);
	free(zrs);
	free(zys);
	mpz_clear(zl);
	mpz_clear(zr);
	mpz_clear(zy);
#endif
}

void test_add(const char *ls, const char *rs)
{
	int128_t l(ls), r(rs);
	int128_t x;
	x = l + r;
	l += r;
	assert(l == x);
	string result(x.to_dec());

#ifdef __sun
	MINT *zl, *zr, *zx, *n1;
	n1 = mp_itom(-1);
	if (ls[0] == '-') {
		zl = mp_xtom((char *)ls+3);
		mp_mult(zl,n1,zl);
	} else
		zl = mp_xtom((char *)ls+2);
	n1 = mp_itom(-1);
	if (rs[0] == '-') {
		zr = mp_xtom((char *)rs+3);
		mp_mult(zr,n1,zr);
	} else
		zr = mp_xtom((char *)rs+2);
	zx = mp_itom(0);
	mp_madd(zl,zr,zx);
	char *zls = mp_mtox(zl);
	char *zrs = mp_mtox(zr);
	char *zxs = mp_mtox(zx);
	if (strcmp(zxs,result.c_str()+2)) {
		printf("int128_t: %s + %s = %s\n",ls,rs,result.c_str());
		printf("sun_mp: %s + %s = %s\n\n",zls,zrs,zxs);
	}
	free(zls);
	free(zrs);
	free(zxs);
	mp_mfree(zl);
	mp_mfree(zr);
	mp_mfree(zx);
#else
	mpz_t zl, zr, zy;
	mpz_init(zy);
	mpz_init_set_str(zl,ls,0);
	mpz_init_set_str(zr,rs,0);
	mpz_add(zy,zl,zr);
	char *zls = mpz_get_str(0,10,zl);
	char *zrs = mpz_get_str(0,10,zr);
	char *zys = mpz_get_str(0,10,zy);
	if (strcmp(zys,result.c_str())) {
		printf("int128_t: %s + %s = %s\n",ls,rs,result.c_str());
		printf("gmp mpz: %s + %s = %s\n\n",zls,zrs,zys);
	}
	free(zls);
	free(zrs);
	free(zys);
	mpz_clear(zl);
	mpz_clear(zr);
	mpz_clear(zy);
#endif
}

int main()
{
	for (int x = 0; x < sizeof(testdata)/sizeof(testdata[0]); ++x) {
		int128_t v(testdata[x]);
		v = -v;
		printf("-%s = %s\n",testdata[x],v.to_hex().c_str());
	}
	for (int x = 0; x < sizeof(testdata)/sizeof(testdata[0]); ++x) {
		int128_t v = abs(testdata[x]);
		printf("abs(%s) = %s\n",testdata[x],v.to_dec().c_str());
	}
	for (unsigned lx = 0; lx < sizeof(testdata)/sizeof(testdata[0]); ++lx) {
		for (unsigned rx = 0; rx < sizeof(testdata)/sizeof(testdata[0]); ++rx) {
			test_add(testdata[lx],testdata[rx]);
			test_sub(testdata[lx],testdata[rx]);
			test_div(testdata[lx],testdata[rx]);
			test_mul(testdata[lx],testdata[rx]);
		}
	}

}


/*
int main(void)
{
	char d[64];
	hrtime_t s,e;
	volatile int i;


	int128_t      x("0x1234567890abcde");
	int128_t      y("0x234");
	int128_t      z("0x23456783456789015678");
#ifdef __sun
	MINT *xz, *yz, *zz, *rz = mp_itom(0), *rz2 = mp_itom(0);
	xz = mp_xtom("1234567890abcde");
	yz = mp_xtom("234");
	zz = mp_xtom("23456783456789015678");
#else
	mpz_class xz("1234567890abcde");
	mpz_class yz("234");
	mpz_class zz("23456783456789015678");
#endif

	printf("x = %16llx%016llx\n",x.hi,x.lo);
	printf("y = %16llx%016llx\n",y.hi,y.lo);
	printf("z = %16llx%016llx\n",z.hi,z.lo);
	cout << x.to_dec() << endl;
	cout << y.to_dec() << endl;
	cout << z.to_dec() << endl;
#ifdef __sun
	cout << "xz = " << mp_mtox(xz) << endl;
	cout << "yz = " << mp_mtox(yz) << endl;
	cout << "zz = " << mp_mtox(zz) << endl;
#else
	cout << xz.get_str(16) << endl;
	cout << yz.get_str(16) << endl;
	cout << zz.get_str(16) << endl;
#endif

	int128_t r;
	s = gethrtime();
	for (i = 0; i < 100000; ++i)
		r = z%y;
	e = gethrtime();
	printf("%lldns\n",e-s);
	printf("%016llx:%016llx\n",r.hi,r.lo);
		r = z % y;
	r = x << 1;
	printf("%016llx:%016llx\n",r.hi,r.lo);
	printf("%s\n",r.to_hex().c_str());
#ifdef __sun
	s = gethrtime();
	for (i = 0; i < 100000; ++i)
		mp_mdiv(zz,yz,rz,rz2);
	e = gethrtime();
	printf("%lldns\n",e-s);
	cout << mp_mtox(rz) << " : " << mp_mtox(rz2) << endl;
#else
	mpz_class rz = xz * yz;
	cout << rz.get_str(16) << endl;
#endif
	//x.to_dec(d);
	//printf("%s\n",d);
	return 0;
}
*/

/*
int main(void)
{
#ifdef __sun
	hrtime_t s,e;
#endif
	int128_t x,y,z;
	x = 135;
	y = -1117;
	cout << "x = " << x.to_dec() << endl;
//	cout << "x = " << x.to_hex() << endl;
//	cout << "y = " << y.to_dec() << endl;
//	cout << "y = " << y.to_hex() << endl;
	int128_t x10 = "1234567890";
	int128_t x20 = "-12345678901234567890";
	cout << "x10 = " << x10.to_dec() << endl;
	cout << "x20 = " << x20.to_dec() << endl;
	x = x20 / x10;
	cout << "x = " << x.to_dec() << endl;
	y = x * x10;
	cout << "y = " << y.to_dec() << endl;
	y /= x10;
	cout << "y = " << y.to_dec() << endl;
	y *= x10;
	cout << "y = " << y.to_dec() << endl;
	y %= x10;
	cout << "y = " << y.to_dec() << endl;
	y = x * x10;
	cout << "y = " << (++y).to_dec() << endl;
	cout << "y = " << (--y).to_dec() << endl;
	cout << "y = " << (-y).to_dec() << endl;
	cout << "y = " << (y+int128_t(-1000)).to_dec() << endl;
	cout << "y = " << (y-1000).to_dec() << endl;
	cout << "y = " << (y+1000).to_dec() << endl;
	cout << "y = " << (y-=1000).to_dec() << endl;
	z = 1;
	for (int i = 0; i < 39; ++i) {
		z *= 10;
		cout << "z = " << z.to_dec() << endl;
		cout << "z = " << z.to_hex() << endl;
	}

	int128_t a(0,UINT64_MAX);
	cout << "a = " << a.to_dec() << endl;
	++a;
	cout << "a = " << a.to_dec() << endl;
	--a;
	cout << "a = " << a.to_dec() << endl;
	a+=1;
	cout << "a = " << a.to_dec() << endl;
	a-=1;
	cout << "a = " << a.to_dec() << endl;
	a = 0x101;
	cout << "a = " << a.to_dec() << endl;
	++a;
	cout << "a = " << a.to_dec() << endl;
	--a;
	cout << "a = " << a.to_dec() << endl;
	a+=1;
	cout << "a = " << a.to_dec() << endl;
	a-=1;
	cout << "a = " << a.to_dec() << endl;
	a <<= 64;
	cout << "a = " << a.to_dec() << endl;
	++a;
	cout << "a = " << a.to_dec() << endl;
	--a;
	cout << "a = " << a.to_dec() << endl;
	a+=1;
	cout << "a = " << a.to_dec() << endl;
	a-=1;
	cout << "a = " << a.to_dec() << endl;
	a = 0;
	cout << "a = " << a.to_dec() << endl;
	--a;
	cout << "a = " << a.to_dec() << endl;
	++a;
	cout << "a = " << a.to_dec() << endl;
	a-=1;
	cout << "a = " << a.to_dec() << endl;
	a+=1;
	cout << "a = " << a.to_dec() << endl;
	++a;
	cout << "a = " << a.to_dec() << endl;
	--a;
	cout << "a = " << a.to_dec() << endl;
	a+=1;
	cout << "a = " << a.to_dec() << endl;
	a-=1;
	cout << "a = " << a.to_dec() << endl;

	int128_t m("5000000000");
	cout << "m   = " << m.to_dec() << endl;
	cout << "m^2 = " << (m * m).to_dec() << endl;
	cout << "m-1 = " << (m-1).to_dec() << endl;

	cout << "-13 > -15 == " << (int128_t(-13) > int128_t(-15)) << endl;
	cout << "13 > -15 == " << (int128_t(13) > int128_t(-15)) << endl;
	cout << "-13 > 15 == " << (int128_t(-13) > int128_t(15)) << endl;
	cout << "13 > 15 == " << (int128_t(13) > int128_t(15)) << endl;
	cout << "-15 > -13 == " << (int128_t(-15) > int128_t(-13)) << endl;
	cout << "15 > -13 == " << (int128_t(15) > int128_t(-13)) << endl;
	cout << "-15 > 13 == " << (int128_t(-15) > int128_t(13)) << endl;
	cout << "15 > 13 == " << (int128_t(15) > int128_t(13)) << endl;
	cout << "-13 < -15 == " << (int128_t(-13) < int128_t(-15)) << endl;
	cout << "13 < -15 == " << (int128_t(13) < int128_t(-15)) << endl;
	cout << "-13 < 15 == " << (int128_t(-13) < int128_t(15)) << endl;
	cout << "13 < 15 == " << (int128_t(13) < int128_t(15)) << endl;
	cout << "-15 < -13 == " << (int128_t(-15) < int128_t(-13)) << endl;
	cout << "15 < -13 == " << (int128_t(15) < int128_t(-13)) << endl;
	cout << "-15 < 13 == " << (int128_t(-15) < int128_t(13)) << endl;
	cout << "15 < 13 == " << (int128_t(15) < int128_t(13)) << endl;
	cout << "1 == 1 == " << (int128_t(1) == int128_t(1)) << endl;
	cout << "1 == 0 == " << (int128_t(1) == int128_t(0)) << endl;
	cout << "1 != 1 == " << (int128_t(1) != int128_t(1)) << endl;
	cout << "1 != 0 == " << (int128_t(1) != int128_t(0)) << endl;

	y = x20 * x20;
	cout << "y = " << (y).to_hex() << endl;
	cout << "y>>16 = " << (y>>16).to_hex() << endl;
	cout << "y>>24 = " << (y>>24).to_hex() << endl;
	cout << "y>>72 = " << (y>>72).to_hex() << endl;
	cout << "y>>13 = " << (y>>13).to_hex() << endl;
	cout << "y<<16 = " << (y<<16).to_hex() << endl;
	cout << "y<<24 = " << (y<<24).to_hex() << endl;
	cout << "y<<72 = " << (y<<72).to_hex() << endl;
	y <<= 64;
	cout << "y = " << y.to_dec() << endl;
	y >>= 64;
	cout << "y = " << y.to_dec() << endl;
	y = y << 64;
	cout << "y = " << y.to_dec() << endl;
	y = y >> 64;
	cout << "y = " << y.to_dec() << endl;
	z = x * y;
	cout << "z = " << z.to_dec() << endl;
	z = -y;
	cout << "z = " << z.to_dec() << endl;
	z = y * int128_t(-1);
	cout << "z = " << z.to_dec() << endl;
	z *= x;
	cout << "z = " << z.to_dec() << endl;
	z = x10;
	cout << "x10 = " << x10.to_dec() << endl;
	z *= int128_t(10);
	cout << "z = " << z.to_dec() << endl;
	z = x10;
	z *= 10;
	cout << "z = " << z.to_dec() << endl;

	return 0;
}
*/

#endif
