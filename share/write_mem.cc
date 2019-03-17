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

#include "sysconfig.h"

void write_u32(uint8_t *wire, uint32_t v);
void write_u64(uint8_t *wire, uint64_t v);

union mangle_double
{
	explicit mangle_double(double d)
	: f64(d)
	{ }

	operator uint64_t () const
	{ return u64; }

	double f64;
	uint64_t u64;
};


/* wfc-template:
 * function: write_u64
 */
void write_u64_generic(uint8_t *wire, uint64_t v)
{
#if defined __BYTE_ORDER__ && (__BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__)
	*(uint64_t*)wire = v;
#else
	*wire++ = v & 0xff;
	v >>= 8;
	*wire++ = v & 0xff;
	v >>= 8;
	*wire++ = v & 0xff;
	v >>= 8;
	*wire++ = v & 0xff;
	v >>= 8;
	*wire++ = v & 0xff;
	v >>= 8;
	*wire++ = v & 0xff;
	v >>= 8;
	*wire++ = v & 0xff;
	v >>= 8;
	*wire++ = v;
#endif
}


/* wfc-template:
 * function: write_u64
 */
void write_u64_0(uint8_t *wire, uint64_t v)
{
	*wire++ = v & 0xff;
	v >>= 8;
	*wire++ = v & 0xff;
	v >>= 8;
	*wire++ = v & 0xff;
	v >>= 8;
	*wire++ = v & 0xff;
	v >>= 8;
	*wire++ = v & 0xff;
	v >>= 8;
	*wire++ = v & 0xff;
	v >>= 8;
	*wire++ = v & 0xff;
	v >>= 8;
	*wire++ = v;
}


/* wfc-template:
 * function: write_u64
 * include: endian.h
 * description: system must support unaligned access
 */
void write_u64_endian_h(uint8_t *wire, uint64_t v)
{
	*(uint64_t*)wire = htole64(v);
}


{
/* wfc-template:
 * function: write_u64
 */
void write_u64_Os(uint8_t *wire, uint64_t v)
{
	for (int i = 0; i < 8; ++i) {
		*wire++ = v & 0xff;
		v >>= 8;
	}
}


/* wfc-template:
 * function: write_u64
 */
void write_u64_Os2(uint8_t *wire, uint64_t v)
{
	for (int i = 0; i < 7; ++i) {
		*wire++ = v & 0xff;
		v >>= 8;
	}
	*wire = v;
}


/* wfc-template:
 * function: write_u16
 * endian: little
 * Optimize: speed
 */
#define write_u16_le(w,v) (*(uint16_t*)w) = v


/* wfc-template:
 * function: write_u32
 * endian: little
 * Optimize: speed
 */
#define write_u32_le(w,v) (*(uint32_t*)w) = v


/* wfc-template:
 * function: write_u64
 * endian: little
 * Optimize: speed
 */
#define write_u64_le(w,v) (*(uint64_t*)w) = v


/* wfc-template:
 * function: write_u64
 * endian: little
 */
void write_u64_le_fun(uint8_t *wire, uint64_t v)
{
	*(uint64_t*)wire = v;
}


/* wfc-template:
 * function: write_u32
 */
void write_u32_0(uint8_t *wire, uint32_t v)
{
	*wire++ = v & 0xff;
	v >>= 8;
	*wire++ = v & 0xff;
	v >>= 8;
	*wire++ = v & 0xff;
	v >>= 8;
	*wire++ = v;
}


/* wfc-template:
 * function: write_u32
 */
void write_u32_1(uint8_t *wire, uint32_t v)
{
	wire[0] = v & 0xff;
	v >>= 8;
	wire[1] = v & 0xff;
	v >>= 8;
	wire[2] = v & 0xff;
	v >>= 8;
	wire[3] = v;
}


/* wfc-template:
 * function: write_u32
 * endian: little
 */
void write_u32_lef(uint8_t *wire, uint32_t v)
{
	*(uint32_t*)wire = v;
}


/* wfc-template:
 * function: write_u16
 */
void write_u16_0(uint8_t *wire, uint16_t v)
{
	*wire++ = v & 0xff;
	v >>= 8;
	*wire++ = v;
}


/* wfc-template:
 * function: write_u16
 */
void write_u16_1(uint8_t *wire, uint16_t v)
{
	wire[0] = v & 0xff;
	v >>= 8;
	wire[1] = v;
}


/* wfc-template:
 * function: write_u16
 * endian: little
 */
void write_u16_lef(uint8_t *wire, uint16_t v)
{
	*(uint16_t*)wire = v;
}


/* no-wfc-template:
 * function: write_float
 */
void write_float_0(uint8_t *wire, float f)
{
	union {
		float f;
		uint32_t i;
	} u;
	u.f = f;
	write_u32(wire,u.i);
}


/* no-wfc-template:
 * function: write_float
 * endian: little
 */
void write_float_le(uint8_t *wire, float f)
{
	write_u32(wire,*(uint32_t*)&f);
}


/* no-wfc-template:
 * function: write_double
 */
void write_double_0(uint8_t *wire, double d)
{
	union {
		double d;
		uint64_t i;
	} u;
	u.d = d;
	write_u64(wire,u.i);
}


/* no-wfc-template:
 * function: write_double
 */
void write_double_u(uint8_t *wire, double d)
{
	write_u64(wire,mangle_double(d));
}


/* no-wfc-template:
 * function: write_double
 */
void write_double_m(uint8_t *wire, double d)
{
	write_u64(wire,mangle_double(d));
}


/* no-wfc-template:
 * function: write_double
 * endian: little
 */
void write_double_le(uint8_t *wire, double d)
{
	write_u64(wire,*(uint64_t*)&d);
}


/* no-wfc-template:
 * function: write_double
 * endian: little
 */
#define write_double_def(wire,d) (*(double*)(wire) = (d))



/*
unsigned u16_mem(uint8_t *wire, signed wl, uint16_t v)
{
	if (wl < 2)
		return 0;
	*wire++ = v & 0xff;
	v >>= 8;
	*wire++ = v & 0xff;
	return 2;
}

unsigned u32_mem(uint8_t *wire, signed wl, uint32_t v)
{
	if (wl < 4)
		return 0;
	*wire++ = v & 0xff;
	v >>= 8;
	*wire++ = v & 0xff;
	v >>= 8;
	*wire++ = v & 0xff;
	v >>= 8;
	*wire++ = v & 0xff;
	return 4;
}

unsigned u64_mem_0(uint8_t *wire, signed wl, uint64_t v)
{
	if (wl < 8)
		return 0;
	*wire++ = v & 0xff;
	v >>= 8;
	*wire++ = v & 0xff;
	v >>= 8;
	*wire++ = v & 0xff;
	v >>= 8;
	*wire++ = v & 0xff;
	v >>= 8;
	*wire++ = v & 0xff;
	v >>= 8;
	*wire++ = v & 0xff;
	v >>= 8;
	*wire++ = v & 0xff;
	v >>= 8;
	*wire++ = v & 0xff;
	return 8;
}
*/

