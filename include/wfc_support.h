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

#ifndef WFC_SUPPORT_H
#define WFC_SUPPORT_H

#include <iosfwd>
#include <string>
#include <inttypes.h>

struct C_ASCII
{
	C_ASCII(const std::string &txt, const std::string &pfx = "\t")
	: text(txt.c_str())
	, prefix(pfx)
	{ }

	C_ASCII(const char *txt, const std::string &pfx = "\t")
	: text(txt)
	, prefix(pfx)
	{ }

	const char *text;
	const std::string &prefix;
};

struct HexBytes
{
	HexBytes(const uint8_t *d, size_t l, const std::string &pfx = "\t")
	: data(d)
	, len(l)
	, prefix(pfx)
	{ }

	HexBytes(const char *d, size_t l, const std::string &pfx = "\t")
	: data((const uint8_t *)d)
	, len(l)
	, prefix(pfx)
	{ }

	const uint8_t *data;
	size_t len;
	const std::string &prefix;
};

std::ostream &operator << (std::ostream &, const C_ASCII &);
std::ostream &operator << (std::ostream &, const HexBytes &);

#endif
