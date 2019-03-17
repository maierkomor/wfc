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

#ifndef SYSCONFIG_H
#define SYSCONFIG_H

#ifndef __MSP430
#include <assert.h>
#endif

#include <stdint.h>
#include <stdio.h>	// for ssize_t, except AVR
#include <limits.h>

#if !defined(__AVR) && !defined(__MSP430)
#include <string>
#endif


#ifdef __AVR
typedef int16_t ssize_t;
#ifndef VARINTBITS
#define VARINTBITS 16
#endif
#endif

#ifndef VARINTBITS
#define VARINTBITS 64
#endif

#if VARINTBITS == 64
typedef uint64_t varint_t;
typedef int64_t varsint_t;
#elif VARINTBITS == 32
typedef uint32_t varint_t;
typedef int32_t varsint_t;
#elif VARINTBITS == 16
typedef uint16_t varint_t;
typedef int16_t varsint_t;
#elif VARINTBITS == 8
typedef uint8_t varint_t;
typedef int8_t varsint_t;
#else
#error invalid varint size
#endif	// VARINTBITS


#endif
