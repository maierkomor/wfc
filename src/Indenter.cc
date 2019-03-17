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

#include "Indenter.h"

#include <assert.h>
#include <string.h>

using namespace std;


void Indenter::process(const char *str)
{
	const char *s = str;
	char c = *s;
	while (c) {
		++s;
		switch (c) {
		case '\n': {
				int ind = indent;
				if (indNext) {
					++ind;
					indNext = false;
				}
				while (*s == '\t')
					++s;
				if (!strncmp(s,"case ",5))
					--ind;
				else if (!strncmp(s,"case\t",5))
					--ind;
				else if (!strncmp(s,"default ",8))
					--ind;
				else if (!strncmp(s,"default\t",8))
					--ind;
				else if (!strncmp(s,"default:",8))
					--ind;
				else if (!strncmp(s,"if ",3))
					indNext = true;
				else if (!strncmp(s,"if\t",3))
					indNext = true;
				else if (!strncmp(s,"else ",5))
					indNext = true;
				else if (!strncmp(s,"else\t",5))
					indNext = true;
				else if (!strncmp(s,"else\n",5))
					indNext = true;
				else if (!strncmp(s,"if(",3))
					indNext = true;
				else if (!strncmp(s,"do ",3))
					indNext = true;
				else if (!strncmp(s,"do\t",3))
					indNext = true;
				else if (!strncmp(s,"for ",4))
					indNext = true;
				else if (!strncmp(s,"for\t",4))
					indNext = true;
				else if (!strncmp(s,"for(",4))
					indNext = true;
				else if ((0 == strncmp(s,"while ",6)) || (0 == strncmp(s,"while(",6))) {
					if (ind == (indent+1)) {
						// do before while
						ind = indent;
					} else {
						indNext = true;
					}
				} else if (*s == '}') 
					--ind;
				out += '\n';
				assert(ind >= 0);
				while (ind) {
					out += '\t';
					--ind;
				}
			}
			break;
		case '"':
			out += c;
			do {
				c = *s;
				++s;
				assert(c);
				out += c;
				if (c == '\\')
					out += *s++;
			} while (c != '"');
			break;
		case '\'':
			out += c;
			do {
				c = *s;
				++s;
				assert(c);
				out += c;
				if (c == '\\')
					out += *s++;
			} while (c != '\'');
			break;
		case '\\':
			out += '\\';
			out += *s;
			++s;
			break;
		case '}':
			--indent;
			out += c;
			break;
		case '{':
			++indent;
			indNext = false;
			out += c;
			break;
		default:
			out += c;
		}
		c = *s;
	}
}

/*
void Indenter::process(const char *s)
{
	char c = *s;
	while (c) {
		++s;
		switch (c) {
		case ' ':
		case '\t':
			hadWS = true;
			out += c;
			break;
		case '\n':
			afterNL = true;
			out += c;
			break;
		case 'f':

		case '+':
		case '-':
		case '*':
		case '/':
		case '%':
		case '&':
		case ',':
		case '.':
		case '|':
		case '=':
		case '>':
		case '<':
		case '!':
		case '?':
		case ':':
		case '~':
			out += c;
			break;
		case ';':

			break;
		case '\'':
			{
				const char *eos = strchr(s,'\'');
				assert(eos);
				++eos;
				out.append(string(s-1,eos));
				s = eos;
			}
			break;
		case '"':
			{
				const char *eos = strchr(s,'"');
				assert(eos);
				++eos;
				out.append(string(s-1,eos));
				s = eos;
			}
			break;
		case '(':
			++pL;
			out += '(';
			break;
		case ')':
			++pL;
			out += ')';
			break;
		case '{':
			++bL;
			out += '{';
			break;
		case '}':
			++bL;
			out += '}';
			break;
		default:
			out += c;
		}
		c = *s;
	}
}

*/


#ifdef TEST
#include <iostream>

const char *text = 
	"if (x) {\n"
	"xx \n"
	"} else {\n"
	"xy\n"
	"}\n"
	"if (bla)\n"
	"blub();\n"
	"do\n"
	"++x;\n"
	"while(y());\n"
	"if (x) {\n"
	" int x;\n"
	"if (y){\n" 
	" xy\n"
	"}\n"
	"}\n";

int main()
{
	string str;
	Indenter f(str);
	f.process(text);
	cout << "in:\n" << text << "\n\n";
	cout << "out:\n" << str << "\n\n";
}
#endif
