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

#include <stdint.h>

/* wfc-template:
 * function: mangle_double
 */
union mangle_double_union
{
	double m_d;
	uint64_t m_u;

	mangle_double(double d)
	: m_d(d)
	{ }

	operator uint64_t () const
	{ return m_u; }
};


/* wfc-template:
 * function: mangle_float
 */
union mangle_float_union
{
	float m_d;
	uint32_t m_u;

	mangle_float(float d)
	: m_d(d)
	{ }

	operator uint32_t () const
	{ return m_u; }
};


uint64_t c_0(double d)
{
	return mangle_double(d);
}

uint64_t c_1(double d)
{
	return *reinterpret_cast<uint64_t*>(&d);
}


#ifdef TEST
#include <stdio.h>
#include <math.h>

int main()
{
	printf("c0 = %llu\n",c0(M_PI));
	printf("c1 = %llu\n",c1(M_PI));
}
#endif
