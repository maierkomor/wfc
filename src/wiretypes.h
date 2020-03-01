/*
 *  Copyright (C) 2017-2020, Thomas Maier-Komor
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

#ifndef WIRETYPES_H
#define WIRETYPES_H

typedef enum wiretype_e {
	wt_varint	= 0x0,
	wt_64bit	= 0x1,
	wt_lenpfx	= 0x2,
	wt_8bit		= 0x3,		// extension
	wt_16bit	= 0x4,		// extension
	wt_32bit	= 0x5,
	// reserved 0x6
	// reserved 0x7
	wt_dynamic	= 0x8,

	wt_msg		= 0x12,
	wt_bytes	= 0x22,
	wt_packed	= 0x32,
	wt_repeated	= 0x42,
	wt_any		= 0xf6,
} wiretype_t;

#endif
