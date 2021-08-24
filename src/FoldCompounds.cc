/*
 *  Copyright (C) 2017-2021, Thomas Maier-Komor
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

#include "FoldCompounds.h"

using namespace std;

void FoldCompounds::process(const char *s)
{
	char c = *s;
	while (c) {
		++s;
		bool deleted = false;
		if (skip) {
			switch (c) {
			case '@':
				// don't skip the next '{' - required for map initializer
				skip = false;
				break;
			case '{':
				++nesting;
				toskip.push(nesting);
				deleted = true;
				break;
			case '}':
				if (!toskip.empty() && (toskip.top() == nesting)) {
					toskip.pop();
					deleted = true;
				} else {
					out += '}';
				}
				--nesting;
				break;
			case '/':
				if ((*s == '/') && (!comments)) {
					do ++s;
					while ((*s != '\n') && (*s != 0));
				} else {
					out += c;
				}
				break;
			case ' ':
			case '\t':
			case '\n':
			case '\r':
				out += c;
				break;
			default:
				out += c;
				skip = false;
			}
		} else {
			switch (c) {
			case '{':
				++nesting;
				skip = true;
				out += '{';
				break;
			case '}':
				if (!toskip.empty() && (toskip.top() == nesting)) {
					toskip.pop();
					deleted = true;
				} else {
					out += '}';
				}
				--nesting;
				break;
			case '/':
				if ((*s == '/') && (!comments)) {
					do ++s;
					while ((*s != '\n') && (*s != 0));
				} else {
					out += c;
				}
				break;
			default:
				out += c;
			}
		}
		if (deleted && (*s == '\n')) {
			//while (!out.empty() && (out.back() == '\t'))
				//out.pop_back();
			size_t n = out.size();
			while ((n != 0) && ((out[--n] == '\t') || out[n] == ' '))
				out.resize(n);
			if ((n != 0) && (out[n] == '\n'))
				++s;
		}
		c = *s;
	}
}


#ifdef TEST
#include <iostream>

const char *text = 
	"if (x) {  {\n"
	"	xx \n"
	"	}\n"
	"} else {\n"
	"  {\n"
	"    xy\n"
	" }\n"
	" x {\n"
	" int x; {\n" 
	" xy\n"
	"}\n"
	"}\n";

int main()
{
	string str;
	FoldCompounds f(str);
	f.process(text);
	cout << "in:\n" << text << "\n\n";
	cout << "out:\n" << str << "\n\n";
}
#endif
