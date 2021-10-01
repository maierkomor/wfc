/*
 *  Copyright (C) 2017-2021, Thomas Maier-Komor
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
 * function:place_varint
 * optimize:speed
 * VarIntBits:64
 * description: place varint_t in memory with full length
 */
void place_varint_vi64(uint8_t *w, varint_t v)
{
	*w++ = (v & 0x7f) | 0x80;
	v >>= 7;
	*w++ = (v & 0x7f) | 0x80;
	v >>= 7;
	*w++ = (v & 0x7f) | 0x80;
	v >>= 7;
	*w++ = (v & 0x7f) | 0x80;
	v >>= 7;
	*w++ = (v & 0x7f) | 0x80;
	v >>= 7;
	*w++ = (v & 0x7f) | 0x80;
	v >>= 7;
	*w++ = (v & 0x7f) | 0x80;
	v >>= 7;
	*w++ = (v & 0x7f) | 0x80;
	v >>= 7;
	*w++ = (v & 0x7f) | 0x80;
	v >>= 7;
	*w = v;
}

/* wfc-template:
 * function:place_varint
 * optimize:speed
 * VarIntBits:32
 * description: place varint_t in memory with full length
 */
void place_varint_vi32(uint8_t *w, varint_t v)
{
	*w++ = (v & 0x7f) | 0x80;
	v >>= 7;
	*w++ = (v & 0x7f) | 0x80;
	v >>= 7;
	*w++ = (v & 0x7f) | 0x80;
	v >>= 7;
	*w++ = (v & 0x7f) | 0x80;
	v >>= 7;
	*w++ = (v & 0x7f) | 0x80;
}

/* wfc-template:
 * function:place_varint
 * optimize:speed
 * VarIntBits:16
 * description: place varint_t in memory with full length
 */
void place_varint_vi16(uint8_t *w, varint_t v)
{
	*w++ = (v & 0x7f) | 0x80;
	v >>= 7;
	*w++ = (v & 0x7f) | 0x80;
	v >>= 7;
	*w++ = (v & 0x7f) | 0x80;
}


/* wfc-template:
 * function:place_varint
 * description: place varint_t in memory with full length
 */
void place_varint_generic(uint8_t *w, varint_t v)
{
	for (size_t x = 0; x < sizeof(varint_t)*8/7; ++x) {
		*w++ = (v & 0x7f) | 0x80;
		v >>= 7;
	}
	*w = v;
}

