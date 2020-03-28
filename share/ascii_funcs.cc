
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

#include <inttypes.h>
#include <stddef.h>
#include <string.h>


/* wfc-template:
 * function: ascii_indent
 */
void ascii_indent($streamtype &out, ssize_t n)
{
	out << '\n';
	while (n > 0) {
		out << '\t';
		--n;
	}
}


/* wfc-template:
 * function: ascii_string
 * sysinclude: string.h
 */
void ascii_string($streamtype &out, const char *str, size_t len, size_t indent)
{
	unsigned cil = 0;
	out << '"';
	while (len) {
		char c = *str++;
		--len;
		if (cil == 64) {
			out << "\"\n";
			$ascii_indent(out,indent);
			out << '"';
			cil = 0;
		}
		++cil;
		if ((c >= 0x23) && (c <= 0x7e)) {
			if (c == 0x5c) {
				// '\' itself must be escaped
				out << '\\';
			}
			// 0-9, a-z A-Z
			// ']'..'~', some operators, etc
			out << c;
			continue;
		}
		switch (c) {
		case '\0': out << "\\0"; continue;
		case '\t': out << "\\t"; continue;
		case '\r': out << "\\r"; continue;
		case '\b': out << "\\b"; continue;
		case '\a': out << "\\a"; continue;
		case '\e': out << "\\e"; continue;
		case '\f': out << "\\f"; continue;
		case '\v': out << "\\v"; continue;
		case '\\': out << "\\\\"; continue;
		case '"' : out << "\\\""; continue;
		case ' ' : out << ' '; continue;
		case '!' : out << '!'; continue;
		case '\n':
			out << "\\n\"\n";
			$ascii_indent(out,indent);
			out << '"';
			continue;
		default:
			out << "\\0" << (unsigned)((c>>6)&0x3) << (unsigned)((c>>3)&0x7) << (unsigned)(c&0x7);
		}
	}
	out << '"';
}


/* wfc-template:
 * function: ascii_bytes
 */
void ascii_bytes($streamtype &out, const uint8_t *str, size_t len, size_t indent)
{
	static char hex_table[] = "0123456789abcdef";
	unsigned cil = 0;		// charakters in one line
	bool mline = (len > 16);	// more than one line?
	out << '{';
	if (mline) {
		out << '\n';
		$ascii_indent(out,indent);
	} else {
		out << ' ';
	}
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
				out << '\n';
				$ascii_indent(out,indent);
				cil = 0;
			}
		}
	}
	if (mline) {
		out << '\n';
		$ascii_indent(out,indent-1);
	} else {
		out << ' ';
	}
	out << '}';
}

