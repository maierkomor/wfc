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
 * function:send_xvarint
 * intsize:16
 */
void send_xvarint_16($putparam, varint_t v)
{
	uint8_t fill64 = (varsint_t)v < 0 ? 0xfc : 0;
	do {
		uint8_t u8 = v & 0x7f;
		v >>= 7;
		if (v)
			u8 |= 0x80;
		else
			u8 |= fill64;
		$wireput(u8);
	} while (v);
	if (fill64) {
		for (int i = 0; i < 6; ++i)
			$wireput(0xff);
	}
}


/* wfc-template:
 * function:send_xvarint
 * intsize:32
 */
void send_xvarint_32($putparam, varint_t v)
{
	uint8_t fill64 = (varsint_t)v < 0 ? 0xf0 : 0;
	do {
		uint8_t u8 = v & 0x7f;
		v >>= 7;
		if (v)
			u8 |= 0x80;
		else
			u8 |= fill64;
		$wireput(u8);
	} while (v);
	if (fill64) {
		for (int i = 0; i < 5; ++i)
			$wireput(0xff);
	}
}


/* wfc-template:
 * function:write_xvarint
 * intsize:16
 */
int write_xvarint_16(uint8_t *wire, ssize_t wl, varint_t v)
{
	uint8_t *w = wire;
	int fill64 = (varsint_t)v < 0;
	do {
		uint8_t u8 = v & 0x7f;
		v >>= 7;
		if (v)
			u8 |= 0x80;
		if (--wl <= 0)
			$handle_error;
		*w++ = u8;
	} while (v);
	if (fill64) {
		w[-1] |= 0xfc;
		for (int i = 0; i < 7; ++i)
			*w++ = 0xff;
	}
	return w-wire;
}


/* wfc-template:
 * function:write_xvarint
 * intsize:32
 */
int write_xvarint_32(uint8_t *wire, ssize_t wl, varint_t v)
{
	uint8_t *w = wire;
	int fill64 = (varsint_t)v < 0;
	do {
		uint8_t u8 = v & 0x7f;
		v >>= 7;
		if (v)
			u8 |= 0x80;
		if (--wl <= 0)
			$handle_error;
		*w++ = u8;
	} while (v);
	if (fill64) {
		w[-1] |= 0xf0;
		for (int i = 0; i < 5; ++i)
			*w++ = 0xff;
	}
	return w-wire;
}


/* wfc-template:
 * function:write_xvarint
 */
int write_xvarint_generic(uint8_t *wire, ssize_t wl, varint_t v)
{
	uint8_t *w = wire;
	uint64_t u64 = (uint64_t)(int64_t)(varsint_t)v;
	do {
		uint8_t u8 = u64 & 0x7f;
		u64 >>= 7;
		if (u64)
			u8 |= 0x80;
		if (--wl <= 0)
			$handle_error;
		*w++ = u8;
	} while (u64);
	return w-wire;
}

