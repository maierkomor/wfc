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
 * function: varint_sint
 */
int64_t varint_sint_default(varint_t v)
{
	varint_t x = -(v & 1);
	v >>= 1;
	v ^= x;
	return v;
}


/* wfc-template:
 * function: varint_sint
 * Optimize: review
 */
int64_t varint_sint_review(varint_t v)
{
	if (v & 1)
		return ~(v >> 1);
	else
		return v >> 1;
}


/* wfc-template:
 * function: varint_sint
 */
int64_t varint_sint_alt1(varint_t v)
{
	uint8_t x = v & 1;
	v >>= 1;
	if (x)
		v = ~v;
	return v;
}

