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

unsigned wiresize(uint64_t);

/* wfc-template:
 * function: wiresize_s
 * varintbits: 64
 */
unsigned wiresize_s_64(int64_t s)
{
	uint64_t u = (s << 1) ^ (s >> 63);
	return wiresize(u);
}


/* wfc-template:
 * function: wiresize_s
 * varintbits: 32
 */
unsigned wiresize_s_32(int32_t s)
{
	uint32_t u = (s << 1) ^ (s >> 31);
	return wiresize(u);
}


/* wfc-template:
 * function: wiresize_s
 * varintbits: 16
 */
unsigned wiresize_s_160(int16_t s)
{
	uint16_t u = (s << 1) ^ (s >> 15);
	if (u < (1ULL << 7))
		return 1;
	if (u < (1ULL << 14))
		return 2;
	return 3;
}

/* wfc-template:
 * function: wiresize_s
 * varintbits: 16
 */
unsigned wiresize_s_161(int16_t s)
{
	uint16_t u = (s << 1) ^ (s >> 15);
	u >>= 7;
	if (0 == u)
		return 1;
	u >>= 7;
	if (u == 0)
		return 2;
	return 3;
}


