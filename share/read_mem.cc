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

/* wfc-template:
 * function: read_u16
 * endian: little
 */
#define read_u16_le(wire) (*(uint16_t *)(wire))


/* wfc-template:
 * function: read_u16
 * Optimize:speed
 * description: WARNING: argument <wire> must not have side-effects.
 */
#define read_u16_fast(wire) (wire[0] | (wire[1]<<8))


/* wfc-template:
 * function: read_u16
 */
uint16_t read_u16_generic(const uint8_t *wire)
{
	uint16_t r;
	r = (uint16_t)wire[0]
	  | (uint16_t)wire[1] << 8;
	return r;
}


/* wfc-template:
 * function: read_u32
 * endian: little
 */
#define read_u32_le(wire) (*(uint32_t *)(wire))


/* wfc-template:
 * function: read_u32
 */
uint32_t read_u32_generic(const uint8_t *wire)
{
	uint32_t r;
	r = (uint32_t)*wire++;
	r |= (uint32_t)*wire++ << 8;
	r |= (uint32_t)*wire++ << 16;
	r |= (uint32_t)*wire << 24;
	return r;
}


/* wfc-template:
 * function: read_u64
 * endian: little
 */
#define read_u64_le(wire) (*(uint64_t *)(wire))


/* wfc-template:
 * function: read_u64
 */
uint64_t read_u64_generic(const uint8_t *wire)
{
	uint64_t r;
	r = (uint64_t)*wire++;
	r |= (uint64_t)*wire++ << 8;
	r |= (uint64_t)*wire++ << 16;
	r |= (uint64_t)*wire++ << 24;
	r |= (uint64_t)*wire++ << 32;
	r |= (uint64_t)*wire++ << 40;
	r |= (uint64_t)*wire++ << 48;
	r |= (uint64_t)*wire << 56;
	return r;
}

/* wfc-template:
 * function: read_float
 * endian: little
 */
#define read_float_le(wire) (*(float *)(wire))


/* wfc-template:
 * function: read_float
 */
float read_float_generic(const uint8_t *wire)
{
	union { uint32_t i; float f; } r;
	r.i = (uint32_t)*wire++;
	r.i |= (uint32_t)*wire++ << 8;
	r.i |= (uint32_t)*wire++ << 16;
	r.i |= (uint32_t)*wire << 24;
	return r.f;
}


/* wfc-template:
 * function: read_double
 * endian: little
 */
#define read_double_le(wire) (*(double *)(wire))


/* wfc-template:
 * function: read_double
 */
double read_double_generic(const uint8_t *wire)
{
	union { uint64_t i; double d; } r;
	r.i = (uint64_t)*wire++;
	r.i |= (uint64_t)*wire++ << 8;
	r.i |= (uint64_t)*wire++ << 16;
	r.i |= (uint64_t)*wire++ << 24;
	r.i |= (uint64_t)*wire++ << 32;
	r.i |= (uint64_t)*wire++ << 40;
	r.i |= (uint64_t)*wire++ << 48;
	r.i |= (uint64_t)*wire << 56;
	return r.d;
}


uint32_t read_u32_1(const uint8_t *wire)
{
	uint32_t r;
	r = (uint32_t)wire[0];
	r |= (uint32_t)wire[1] << 8;
	r |= (uint32_t)wire[2] << 16;
	r |= (uint32_t)wire[3] << 24;
	return r;
}


uint32_t read_u32_2(const uint8_t *wire)
{
	return	  (uint32_t)wire[0]
		| (uint32_t)wire[1] << 8
		| (uint32_t)wire[2] << 16
		| (uint32_t)wire[3] << 24;
}


union convert_mem
{
	convert_mem(const char *d)
	: cp(d)
	{ } 
	
	convert_mem(const uint8_t *d)
	: u8(d)
	{ } 
	
	const char *cp;
	const uint8_t *u8;
	const uint16_t *u16;
	const uint32_t *u32;
	const uint64_t *u64;
	
	operator uint16_t () const
	{ return *u16; }
	
	operator uint32_t () const;
	//{ return *u32; }
	
	operator uint64_t () const
	{ return *u64; }
};


convert_mem::operator uint32_t () const
{ return *u32; }

uint32_t read_u32_3(const uint8_t *wire)
{
	return convert_mem(wire);
}
