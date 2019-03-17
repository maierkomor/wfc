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

#ifndef WIREFUNCS_H
#define WIREFUNCS_H

#include <stdint.h>
#include <sys/types.h>

typedef uint64_t varint_t;

int read_varint(const uint8_t *wire, ssize_t wl, varint_t *r);
unsigned read_u64(const uint8_t *wire, signed wl, uint64_t *r);
unsigned read_u32(const uint8_t *wire, signed wl, uint32_t *r);
unsigned read_u16(const uint8_t *wire, signed wl, uint16_t *r);
unsigned wiresize_u64(uint64_t u);

#endif
