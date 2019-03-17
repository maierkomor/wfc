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
 * function: wiresize
 */
unsigned wiresize_generic(varint_t u)
{
	unsigned x = 0;
	do ++x;
	while (u>>=7U);
	return x;
}


/* wfc-template:
 * function: wiresize
 */
unsigned wiresize_b(varint_t u)
{
	unsigned x = 1;
	while (u>>=7U)
		++x;
	return x;
}


/* wfc-template:
 * function: wiresize
 * Optimize: speed
 * varintbits: 64
 */
unsigned wiresize_0(varint_t u)
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


/* wfc-template:
 * function: wiresize
 * Optimize: speed
 * varintbits: 64
 */
unsigned wiresize_1(varint_t u)
{
	if (0 == (u>>=7U))
		return 1;
	if (0 == (u>>=7U))
		return 2;
	if (0 == (u>>=7U))
		return 3;
	if (0 == (u>>=7U))
		return 4;
	if (0 == (u>>=7U))
		return 5;
	if (0 == (u>>=7U))
		return 6;
	if (0 == (u>>=7U))
		return 7;
	if (0 == (u>>=7U))
		return 8;
	if (0 == (u>>=7U))
		return 9;
	return 10;
}


/* wfc-template:
 * function: wiresize
 * Optimize: speed
 * varintbits: 64
 */
unsigned wiresize_2(varint_t u)
{
	if ((u >> 7) == 0)
		return 1;
	if ((u >> 14) == 0)
		return 2;
	if ((u >> 21) == 0)
		return 3;
	if ((u >> 28) == 0)
		return 4;
	if ((u >> 35) == 0)
		return 5;
	if ((u >> 42) == 0)
		return 6;
	if ((u >> 49) == 0)
		return 7;
	if ((u >> 56) == 0)
		return 8;
	if ((u >> 63) == 0)
		return 9;
	return 10;
}


/* wfc-template:
 * function: wiresize
 * Optimize: speed
 * varintbits: 32
 */
unsigned wiresize_3(uint32_t u)
{
	if (u < (1ULL << 7))
		return 1;
	if (u < (1ULL << 14))
		return 2;
	if (u < (1ULL << 21))
		return 3;
	if (u < (1ULL << 28))
		return 4;
	return 5;
}


/* wfc-template:
 * function: wiresize
 * Optimize: speed
 * varintbits: 16
 */
unsigned wiresize_4(uint16_t u)
{
	if (u < (1ULL << 7))
		return 1;
	if (u < (1ULL << 14))
		return 2;
	return 3;
}


