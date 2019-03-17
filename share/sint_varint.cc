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
 * function: sint_varint
 */
varint_t sint_varint_0(varsint_t i)
{
	return (i << 1) ^ (i >> ((sizeof(varint_t)*8)-1));
}

/* wfc-template:
 * function: sint_varint
 */
varint_t sint_varint_1(varsint_t i)
{
	if (i < 0)
		return (i << 1) ^ ((varsint_t)-1);
	return (i << 1);
}

/* wfc-template:
 * function: sint_varint
 */
varint_t sint_varint_2(varsint_t i)
{
	varint_t r = (varint_t)i << 1;
	if (i < 0)
		r ^= ((varsint_t)-1);
	return r;
}


#ifdef RUNTEST
int main()
{

}
#endif
