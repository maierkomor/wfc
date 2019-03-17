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

#include "wfc_support.h"

#include <iostream>
#include <string.h>


using namespace std;


void ascii2c_src(ostream &out, const char *str, const string &prefix)
{
	unsigned cil = 0;
	size_t len = str ? strlen(str) : 0;
	out << '"';
	while (len) {
		char c = *str++;
		--len;
		if (cil == 64) {
			out << "\"\n" << prefix << '"';
			cil = 0;
		}
		++cil;
		if ((c >= 0x5d) && (c <= 0x7e)) {
			// ']'..'~'
			// a-z plus some more
			out << c;
			continue;
		}
		if ((c == ' ') || (c == '!')) {
			out << c;
			continue;
		}
		if ((c >= 0x23) && (c <= 0x5b)) {
			// 0-9, A-Z, some operators, etc
			out << c;
			continue;
		}
		switch (c) {
		case '\0': out << "\\0"; continue;
		case '\t': out << "\\t"; continue;
		case '\n': out << "\\n\"\n" << prefix << '"'; continue;
		case '\r': out << "\\r"; continue;
		case '\b': out << "\\b"; continue;
		case '\a': out << "\\a"; continue;
		case '\e': out << "\\e"; continue;
		case '\f': out << "\\f"; continue;
		case '\v': out << "\\v"; continue;
		case '\\': out << "\\\\"; continue;
		case '"': out << "\\\""; continue;
		default:
			out << "\\0" << (unsigned)((c>>6)&0x3) << (unsigned)((c>>3)&0x7) << (unsigned)(c&0x7);
		}
	}
	out << '"';
}


void ascii2hex(std::ostream &out, const uint8_t *str, size_t len, const string &prefix)
{
	static char hex_table[] = "0123456789abcdef";
	unsigned cil = 0;	// charakters in one line
	bool mline = (len > 16);
	if (mline)
		out << "{\n" << prefix;
	else
		out << "{ ";
	while (len) {
		uint8_t c = *str++;
		out << hex_table[(c >> 4)&0xf] << hex_table[c&0xf];
		--len;
		++cil;
		if (len) {
			out << ',';
			if ((cil & 0x3) == 0)
				out << ' ';
			if (mline && (cil == 8)) {
				out << ' ';
			} else if (cil == 16) {
				out << '\n' << prefix;
				cil = 0;
			}
		}
	}
	if (mline) {
		string indent = prefix;
		indent.resize(prefix.size()-1);
		out << '\n' << indent << '}';
	} else
		out << " }";
}


ostream &operator << (ostream &out, const C_ASCII &in)
{
	ascii2c_src(out,in.text,in.prefix);
	return out;
}


ostream &operator << (ostream &out, const HexBytes &in)
{
	ascii2hex(out,in.data,in.len,in.prefix);
	return out;
}
