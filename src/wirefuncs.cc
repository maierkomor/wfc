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

#include "wirefuncs.h"

#if defined __x86 || defined __x86_64
#define UNALIGNED_LITTLE_ENDIAN
#endif

int read_varint(const uint8_t *wire, ssize_t wl, varint_t *r)
{
	uint8_t u8;
	int n = 0;
	*r = 0;
	do {
		if (--wl < 0)
			return 0;
		u8 = *wire++;
		*r |= (uint64_t)(u8&~0x80) << (n*7);
		++n;
	} while ((u8 & 0x80) && (n < 10));
	return n;
}


unsigned read_u64(const uint8_t *wire, signed wl, uint64_t *r)
{
	if (wl < 8)
		return 0;
#ifdef UNALIGNED_LITTLE_ENDIAN
	*r = *(uint64_t *)wire;
#else
	*r = (uint64_t)wire[0];
	*r |= (uint64_t)wire[1] << 8;
	*r |= (uint64_t)wire[2] << 16;
	*r |= (uint64_t)wire[3] << 24;
	*r |= (uint64_t)wire[4] << 32;
	*r |= (uint64_t)wire[5] << 40;
	*r |= (uint64_t)wire[6] << 48;
	*r |= (uint64_t)wire[7] << 56;
#endif
	return 8;
}


unsigned read_u32(const uint8_t *wire, signed wl, uint32_t *r)
{
	if (wl < 4)
		return 0;
#ifdef UNALIGNED_LITTLE_ENDIAN
	*r = *(uint32_t *)wire;
#else
	*r = (uint32_t)wire[0];
	*r |= (uint32_t)wire[1] << 8;
	*r |= (uint32_t)wire[2] << 16;
	*r |= (uint32_t)wire[3] << 24;
#endif
	return 4;
}


unsigned read_u16(const uint8_t *wire, signed wl, uint16_t *r)
{
	if (wl < 4)
		return 0;
#ifdef UNALIGNED_LITTLE_ENDIAN
	*r = *(uint16_t *)wire;
#else
	*r = (uint16_t)wire[0];
	*r |= (uint16_t)wire[1] << 8;
#endif
	return 2;
}


unsigned wiresize_u64(uint64_t u)
{
	if (u < (1ULL << 7))
		return 1;
	if (u < (1ULL << 14))
		return 2;
	if (u < (1ULL << 21))
		return 3;
	if (u < (1ULL << 28))
		return 4;
	if (u < (1ULL << 35))
		return 5;
	if (u < (1ULL << 42))
		return 6;
	if (u < (1ULL << 49))
		return 7;
	if (u < (1ULL << 56))
		return 8;
	if (u < (1ULL << 63))
		return 9;
	return 10;
}


